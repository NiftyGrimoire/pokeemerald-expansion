# Gameplay Randomizer Hack Plan

## Summary
Build a per-save deterministic gameplay randomizer on top of `pokeemerald-expansion` stable `1.16.2`, keeping Emerald's map/story for now. Each save gets one randomizer seed; the same species always has the same randomized ability and learnset within that save, including after evolution.

## Architecture Decisions
- Randomized data is resolved at runtime from the save seed. A build-time generated
  table cannot vary between save files.
- Randomization uses a stateless hash, not the game's mutable RNG stream. A result is
  keyed by the save seed, a versioned category, and explicit context values. Looking
  up one result must not affect any other result.
- Category identifiers and hash behavior are save-format API. Once released, changing
  either requires incrementing the randomizer algorithm version.
- Version 1 saves store a nonzero `u32 randomizerSeed` and a randomizer algorithm
  version in `SaveBlock3`. New-game initialization creates both values. Save
  compatibility with unmodified Expansion saves is not required for the initial
  romhack release.
- A single eligibility helper will define the general species pool. It must require an
  enabled, real Pokemon species and exclude Mega Evolutions, Primal Reversions, Ultra
  Burst forms, Gigantamax forms, Tera forms, Totem forms, and other forms found to be
  unusable outside their special context. Feature-specific filters may narrow this
  pool further.
- Evolution-family identity and the exact randomized-ability representation remain
  open design questions. No ability hook should be implemented until both are
  specified together.
- Randomized learnsets will be resolved at runtime and may use a cache if profiling
  shows it is needed. The move pool, weighting, duplicate policy, and evolution move
  behavior must be specified before hooking learnset access.

## Development Phases
1. Foundation:
   - Add a master compile-time config gate, enabled for this hack.
   - Store and initialize the per-save seed and algorithm version.
   - Add a pure, deterministic, category-separated hash API.
   - Add the shared species eligibility helper.
   - Add focused unit tests for initialization, determinism, category/key separation,
     and representative eligible/ineligible species.
2. Encounters:
   - Specify which normal, fishing, rock-smash, outbreak, Feebas, scripted, and
     DexNav paths are randomized.
   - Implement a deterministic eligible-species pool and hook the agreed paths after
     vanilla slot and level selection.
   - Scale the eligible encounter pool by route difficulty so low-level areas draw
     from lower-BST species and high-level areas draw from higher-BST species.
   - Derive the difficulty band from stable encounter-table data rather than the
     mutable level roll, preserving deterministic slot mappings.
3. Legendary encounters:
   - Randomize scripted/static encounters only when the original species is a
     Legendary or Paradox Pokemon.
   - Select replacements from a dedicated pool containing only enabled, usable
     restricted Legendary, sub-Legendary, and Paradox species.
   - Exclude Mythical Pokemon and Ultra Beasts unless the policy is deliberately
     expanded later.
   - Preserve the scripted encounter's level, battle setup, flags, and progression
     behavior while replacing only its species.
4. Abilities:
   - Specify family identity, whether the result is an ability ID or an ability slot,
     legal ability pool rules, and form/gimmick overrides.
   - Route all Pokemon origins through the same resolver.
5. Learnsets:
   - Specify candidate moves and weighting, then add the runtime resolver and any
     measured cache.
6. Progression rules:
   - Enable hard caps and no EV gain.
   - Replace friendship evolutions from an explicit species-by-species conversion
     table, preserving applicable secondary conditions.
7. Quality of life:
   - Implement and grant Level Capper and Portable Healer after their exact item-use
     and level-up/evolution behavior is specified.

### Phase 1 Contract
- `RandomizerHash` is a pure function: it does not read or advance either global RNG.
- The saved seed is nonzero. New-game initialization converts a generated zero to a
  fixed nonzero fallback.
- Hash callers supply all contextual identity explicitly. The foundation API does not
  infer map, encounter, species, or slot state from globals.
- Feature-specific config gates are added with their feature, so an enabled config
  option never advertises behavior that has not been implemented.
- Save structure size tests are intentionally updated for the new romhack format.

## Key Changes
- Add randomizer config gates, default enabled for this hack:
  - Randomized encounters from all enabled Pokemon through Gen 9.
  - Randomized species abilities, stable across each evolution line.
  - Randomized learnsets, weighted toward the Pokemon's type.
  - Hard Emerald gym-based level caps.
  - No EV gain.
  - Friendship evolutions replaced by level evolutions.
- Store one `u32 randomizerSeed` in save data and initialize it during new-game setup.
- Add deterministic helper functions:
  - `GetRandomizerSeed()`
  - `RandomizerHash(category, species/map/slot/etc.)`
  - `GetRandomizedSpeciesForEncounter(originalSpecies, map, encounterType, slot)`
  - `GetRandomizedAbilityNumForSpecies(species)`
  - `GetRandomizedLevelUpMove(species, learnLevel, slot)`
- Randomization must be stable for a save, but different across new saves.

## Gameplay Implementation
- Encounters:
  - Intercept wild encounter species selection after the vanilla encounter slot is chosen.
  - Preserve encounter levels, encounter method, and slot rarity.
  - Replace species with a deterministic random species from enabled Pokemon in a
    BST band appropriate to the encounter table's level range.
  - Exclude invalid entries: `SPECIES_NONE`, eggs, disabled species, battle-only forms, unusable forms.
  - Use stable encounter-table difficulty data as hash/filter context. Do not use
    the mutable rolled encounter level as identity.
  - If no species is eligible in the preferred BST band, expand to the nearest
    adjacent band deterministically rather than failing the encounter.
- Legendary encounters:
  - Detect scripted/static encounters whose original species is flagged as a
    restricted Legendary, sub-Legendary, or Paradox Pokemon.
  - Replace those species deterministically from a separate pool containing only
    enabled and generally usable Pokemon with one of those same classifications.
  - Do not include Mythical Pokemon or Ultra Beasts in the initial pool.
  - Preserve the original scripted level and progression behavior.
- Abilities:
  - Choose one legal randomized ability per evolution family/base species.
  - Apply the same ability slot result to all members of that evolution line.
  - Player, wild, gift, and trainer Pokemon all resolve through the same deterministic ability logic unless a battle gimmick/form explicitly overrides ability.
- Moves:
  - Keep move data itself intact: move power/type/effect are not randomized.
  - Randomize level-up learnsets at runtime using deterministic seed logic.
  - Weight candidate moves toward Pokemon typing: strong preference for STAB, secondary preference for coverage/status, reject unusable/special-case moves.
  - Same species gets the same learnset across encounters in the same save.
- Level caps / EVs:
  - Set `B_EXP_CAP_TYPE = EXP_CAP_HARD`.
  - Set `B_LEVEL_CAP_TYPE = LEVEL_CAP_FLAG_LIST`.
  - Set `B_RARE_CANDY_CAP = TRUE`.
  - Keep existing Emerald cap table initially: 15, 19, 24, 29, 31, 33, 42, 46, champion 58.
  - Set `B_EV_CAP_TYPE = EV_CAP_NO_GAIN`; hide/avoid EV-focused UI where practical.
- QoL:
  - Add two reusable Key Items:
    - Level Capper: choose a party Pokemon and raise it to the current cap, respecting evolution/learn-move flow.
    - Portable Healer: heal party outside battle using existing party-heal behavior.
  - Grant both Key Items early in the game, preferably during starter/new-game setup.
- Friendship evolutions:
  - Replace all `IF_MIN_FRIENDSHIP` evolution conditions with level-based equivalents:
    - Baby/early evolutions: level 20.
    - Midgame evolutions: level 30.
    - Special/late evolutions: level 40.
  - Preserve extra conditions where meaningful, such as day/night or known move type, unless they depended only on friendship.

## Test Plan
- Build with `make -j$(nproc)`.
- Start two new saves and confirm encounter/ability/learnset randomization differs between saves.
- Within one save, confirm repeated encounters of the same species have consistent ability and learnset.
- Confirm evolution preserves the randomized ability behavior for that evolution line.
- Confirm Pokemon at cap gain no EXP and Rare Candy/EXP Candy cannot exceed cap.
- Confirm EVs do not increase from battle or EV items.
- Confirm friendship evolution species evolve by the new level thresholds.
- Confirm Level Capper and Portable Healer work outside battle and are blocked or harmless in battle.

## Assumptions
- Development is based on `expansion/1.16.2` and integrated into `romhack/main`.
- Pokemon pool is all enabled species/forms through Gen 9, filtered for validity.
- Randomization is per-save deterministic, not build-time static and not rerolled every encounter.
- Move randomization means randomized learnsets, not randomized move effects/power/type.
- Initial level caps use Emerald's existing cap values.
