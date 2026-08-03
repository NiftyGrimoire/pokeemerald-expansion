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
- Version 2 saves store a nonzero `u32 randomizerSeed` and a randomizer algorithm
  version in `SaveBlock3`. New-game initialization creates both values. Version 2
  intentionally replaces the unreleased version-1 learnset mapping with bucketed
  move selection; pre-version-2 development saves are not supported. Save
  compatibility with unmodified Expansion saves is not required for the initial
  romhack release.
- A single eligibility helper will define the general species pool. It must require an
  enabled, real Pokemon species and exclude Mega Evolutions, Primal Reversions, Ultra
  Burst forms, Gigantamax forms, Tera forms, Totem forms, and other forms found to be
  unusable outside their special context. Feature-specific filters may narrow this
  pool further.
- Evolution-family identity and randomized ability resolution are implemented
  through the shared deterministic family policy documented in the handoff.
- Randomized learnsets resolve at runtime on `romhack/main`. The version-1 move
  pool, weighting, duplicate policy, schedule, and access seam are documented in
  the implementation handoff. Add a broader cache only if profiling shows it is
  needed.

## Roadmap and Task List

Status in this list is authoritative for phase-level planning. Detailed completed
behavior and validation belong in `AgentDocs/randomizer-implementation.md`.
"Implemented on feature branch" does not mean merged into `romhack/main`.

Current execution order:

1. Complete the remaining manual gameplay checks for Phases 8 and 9.
2. Implement Phase 10's shared level-to-cap flow and Portable Healer.
3. Specify and implement the Phase 11 world-item pool and safeguards.
4. Audit and tune whole-game progression in Phase 12 after the preceding systems
   can be evaluated together.

1. Foundation — complete on `romhack/main`:
   - Add a master compile-time config gate, enabled for this hack.
   - Store and initialize the per-save seed and algorithm version.
   - Add a pure, deterministic, category-separated hash API.
   - Add the shared species eligibility helper.
   - Add focused unit tests for initialization, determinism, category/key separation,
     and representative eligible/ineligible species.
2. Encounters — complete on `romhack/main`; manual gameplay/performance checks remain:
   - Specify which normal, fishing, rock-smash, outbreak, Feebas, scripted, and
     DexNav paths are randomized.
   - Implement a deterministic eligible-species pool and hook the agreed paths after
     vanilla slot and level selection.
   - Scale the eligible encounter pool by route difficulty so low-level areas draw
     from lower-BST species and high-level areas draw from higher-BST species.
   - Derive the difficulty band from stable encounter-table data rather than the
     mutable level roll, preserving deterministic slot mappings.
3. Legendary encounters — complete on `romhack/main`; manual gameplay checks remain:
   - Route ordinary `setwildbattle` encounters through the regular BST-scaled
     encounter resolver using their fixed scripted level.
   - Randomize scripted/static encounters only when the original species is a
     Legendary or Paradox Pokemon through the dedicated special pool.
   - Select replacements from a dedicated pool containing only enabled, usable
     restricted Legendary, sub-Legendary, and Paradox species.
   - Exclude Mythical Pokemon and Ultra Beasts unless the policy is deliberately
     expanded later.
   - Preserve the scripted encounter's level, battle setup, flags, and progression
     behavior while replacing only its species.
   - Manually verify the Emerald Legendary encounters, including capture/defeat
     flags and repeat-entry behavior.
4. Level caps and EV removal — complete on `romhack/main`; manual gameplay checks
   remain:
   - Enable the existing hard Emerald flag-based level-cap system.
   - Prevent Rare Candies and EXP Candies from exceeding the active cap.
   - Disable battle EV gain and prevent EV-boosting items from bypassing the
     zero-EV cap.
   - Manually verify battle experience, cap progression, candy behavior, and all
     EV-changing item paths before building the shared level-to-cap action on these
     rules.
5. Starters — complete on `romhack/main`; manual gameplay checks remain:
   - Deterministically select three distinct starter choices for each save.
   - Limit choices to enabled, generally usable, non-special base-stage Pokemon
     with BST 300-350 and a complete three-stage evolution line.
   - Resolve all player-facing starter uses through `GetStarterPokemon` so the
     selection UI, granted Pokemon, party checks, and credits agree.
   - The rival is not required to choose or retain one of these species; rival
     parties follow the enemy-trainer policy below.
   - Manually verify selection labels, sprites, cries, confirmation, the granted
     Pokemon, party checks, and save-to-save variation in gameplay.
6. Enemy trainer parties — complete on `romhack/main`; manual gameplay checks
   remain:
   - Randomize ordinary enemy party species per configured trainer encounter.
   - Key each slot by the save seed, algorithm version, trainer ID, and party slot
     so the same encounter is stable within a save but differs across saves.
   - Treat separate rival trainer IDs as separate encounters. Rival teams may be
     unrelated between story battles; no persistent rival evolution line is
     required.
   - Preserve party size and levels, and select replacements near each original
     species' BST to retain approximate difficulty.
   - Use an inclusive original-BST ±50 preferred pool, expanding outward by 50
     only when that pool is empty.
   - Regenerate legal moves and validate abilities, held items, and gimmicks
     against the replacement species.
   - Initially exclude Battle Frontier, Trainer Hill, e-Reader, Secret Base, and
     player-controlled trainer parties.
   - Manually verify ordinary, rival, boss, override, and pooled trainers;
     save-to-save variation; repeated-encounter stability; and held-item/gimmick
     behavior.
7. Abilities — complete on `romhack/main`; manual gameplay checks remain:
   - Resolve one deterministic ability ID per enabled evolution-graph family.
   - Route normal party, box, battle, UI, and overworld reads through
     `GetAbilityBySpecies` while preserving the saved authored ability slot.
   - Preserve entire families containing form-driving abilities and exclude
     form/signature-only, Wonder Guard, placeholder, and unimplemented abilities
     from the version-1 candidate pool.
   - Cache family identities and resolved family abilities in EWRAM.
8. Learnsets — complete on `romhack/main`; manual gameplay checks remain:
   - Allow every enabled real Pokemon species to learn every configured TM while
     preserving authored HM and move-tutor compatibility.
   - Give each species four starting moves at level 1, then front-load new moves
     across the configured cap bands while retaining the normal 20-move table
     capacity. Deterministically replace each move per save, species, and slot
     without duplicates.
   - Weight damaging moves toward a level-scaled target power and shift status
     weighting from basic to strong and elite effects as levels rise. Keep every
     eligible tier possible at nonzero weight, use a 4x STAB multiplier versus 2x
     coverage, and scale status-tier weights proportionally to retain their
     approximate relationship to STAB. Exclude unsuitable species-locked moves
     and retain the authored move as an empty-pool fallback. Saturate
     power-distance penalties at the minimum weight without unsigned subtraction
     so extreme-power moves remain rare instead of overflowing the weighted pool.
   - Resolve level-up, evolution, reminder, AI, and Pokedex paths through the
     shared full-table accessor. Initial move assignment generates the same
     deterministic sequence only through the Pokemon's current level and keeps a
     rolling window of the last four moves, avoiding work on future-level slots.
     Build a compact move-bucket index once from Expansion's authoritative
     `gMovesInfo` metadata, grouping damaging moves by exact type and normalized
     power and status moves by potency tier. Runtime selection weighs these
     buckets and subtracts prior selections instead of repeatedly scanning and
     reclassifying the complete move table. Only add a broader cache after
     measuring the remaining access cost.
   - Do not expand egg-move support while breeding is planned for removal. Revisit
     egg moves only if a non-breeding acquisition path is retained or added.
9. Evolution rules — complete on `romhack/main`; manual gameplay checks remain:
   - The authoritative species-level conversion ledger is
     `AgentDocs/hack-plans/evolution-rules.md`.
   - Replace friendship evolutions from an explicit species-by-species conversion
     table, preserving applicable secondary conditions.
   - Make time-dependent and alternate-form evolution lines practical in a short
     Nuzlocke: replace day/night dependencies and deterministically randomize the
     available branch or form, including regional forms, under an explicit
     species-by-species policy.
   - Replace move requirements with explicit levels, remove location and trade
     requirements, and consolidate item evolutions into standard stones or the
     Linking Cord. Preserve player choice for stone-based branches.
10. Quality of life — planned:
   - Add a button to the party menu and Pokemon Storage that raises a selected
     Pokemon to the current level cap without requiring a boxed Pokemon to be moved
     into the party first. Process the intervening levels in order so every
     move-learning opportunity and evolution check occurs normally rather than
     jumping directly to the final level.
   - Add an option on the Pokemon summary stats page to display the Pokemon's
     individual IV values, with a clear way to return to the normal stat view.
   - Add a Bag Repel QoL option. Specify whether it is a reusable Key Item or a
     direct Bag toggle, along with its encounter-level rule and how the player
     disables it, before implementation.
   - Allow each HM field action once its corresponding badge has been earned,
     without requiring any Pokemon in the party to know the HM move. Preserve
     the badge-based story progression checks.
   - Add an Oldale Town NPC who gives the player 200 Ultra Balls and ₽9,999 as
     an early-game supply grant.
   - Remove hidden item spots so all obtainable overworld pickups are visibly
     represented to the player.
   - Implement and grant the Portable Healer after its exact item-use behavior is
     specified.
11. World items — planned:
   - Deterministically randomize visible item pickups found in the overworld;
     hidden item spots are removed rather than randomized.
   - After reviewing item availability and progression balance, consider adding
     new visible overworld pickup spots in areas that need more useful rewards.
     Prefer placing these rewards behind optional trainer battles so obtaining
     them involves a visible, deliberate risk-and-reward choice.
   - Limit replacements to items useful in a Nuzlocke, initially held items and
     evolution items; define exclusions and progression safeguards before
     implementation.
   - Build the pool from actual current utility, not an item's legacy category.
     Exclude obsolete species-specific evolution items whose routes now use
     stones, unless an item retains a separately useful battle effect.
   - Audit low-value consumables before admitting them. In particular, exclude
     X-items if in-battle stat items are outside the ruleset, and exclude basic
     Potions or similar healing items if the Portable Healer makes them redundant.
12. Streamlined game progression — planned:
   - Minimize mandatory grinding so a viable party can stay near each active level
     cap through normal trainer battles and exploration.
   - Remove breeding as a required or supported progression system; audit the Day
     Care, eggs, inherited moves, and breeding-only rewards or encounters so nothing
     important depends on breeding.
   - Remove berry planting, watering, growth, and harvesting as a supported
     progression mechanic. Audit berry plots, related NPCs and tutorials, and
     berry-dependent rewards or encounters; provide direct, finite acquisition
     paths for any berries that remain important to Nuzlocke gameplay.
   - Complete the no-EV experience by removing or repurposing EV-focused items,
     rewards, dialogue, and UI that no longer provide useful choices.
   - Review Emerald's plot progression for optional streamlining, prioritizing fewer
     forced detours, repeated conversations, and backtracking while preserving
     progression flags, essential tutorials, major encounters, and story coherence.
   - Playtest the full badge-to-Champion route against the level caps before choosing
     EXP, trainer-level, encounter-level, or plot-flow adjustments.

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
  - Move-, clock-, location-, trade-, and species-specific-item evolution barriers
    replaced by explicit levels, deterministic seeded branches, standard stones,
    or the Linking Cord according to the authoritative evolution ledger.
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
  - Route non-special `setwildbattle` encounters through the same ordinary pool,
    using the fixed scripted level for difficulty and a distinct scripted
    encounter type for hash separation.
- Legendary encounters:
  - Detect scripted/static encounters whose original species is flagged as a
    restricted Legendary, sub-Legendary, or Paradox Pokemon.
  - Replace those species deterministically from a separate pool containing only
    enabled and generally usable Pokemon with one of those same classifications.
  - Do not include Mythical Pokemon or Ultra Beasts in the initial pool.
  - Preserve the original scripted level and progression behavior.
- Starters:
  - Replace the three displayed and granted starter species deterministically from
    the save seed while preserving each choice slot's identity.
  - Ensure the selection UI, granted Pokemon, and starter-dependent scripts agree
    on the randomized choices.
  - Decide whether choices must be distinct and whether the pool excludes special
    species, unusable forms, evolved species, or Pokemon outside an approved BST
    range before implementation.
- Enemy trainer parties:
  - Resolve replacements independently for each configured trainer ID and party
    slot. This includes each May/Brendan battle as its own encounter.
  - Keep the result deterministic within a save; do not consume mutable battle RNG
    or reroll the team when an encounter is reloaded.
  - Preserve configured party size and levels while using an original-BST-relative
    candidate pool to maintain approximate trainer difficulty.
  - Give replacement species legal moves and valid abilities, and safely handle
    incompatible species-specific items and battle gimmicks.
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
  - Add a reusable action to both the party menu and Pokemon Storage that raises the
    selected Pokemon to the current cap. Boxed Pokemon must not need a temporary open
    party slot. Both entry points must use the same sequential level-up flow so no
    learn-move prompt or evolution opportunity is skipped; stop safely if the flow is
    cancelled or interrupted, persist all changes to the correct party or box slot,
    and never exceed the active cap.
  - Add a Portable Healer Key Item that heals the party outside battle using existing
    party-heal behavior, and grant it early in the game, preferably during
    starter/new-game setup.
  - Add a summary-screen stats-page option that toggles between the normal stat
    presentation and explicit IV values without changing the Pokemon.
  - Add a Bag Repel option for enabling and disabling repel behavior without
    consuming ordinary Repel items; finalize its exact Bag UX and encounter rule
    before implementation.
  - Once the corresponding badge is earned, allow an HM field action without a
    party Pokemon knowing the move. Keep the existing badge gates so field
    traversal cannot bypass normal story progression.
  - Add an Oldale Town NPC who gives the player 200 Ultra Balls and ₽9,999 as
    an early-game supply grant. Choose the NPC and exact script placement during
    implementation.
- Friendship evolutions:
  - Replace all `IF_MIN_FRIENDSHIP` evolution conditions with level-based equivalents:
    - Baby/early evolutions: level 20.
    - Midgame evolutions: level 30.
    - Special/late evolutions: level 40.
  - Remove clock and move requirements because short-run access and randomized
    learnsets cannot reliably satisfy them.
- Time-dependent, branched, and form evolutions:
  - Remove day/night availability barriers for a quick Nuzlocke.
  - For species with alternate evolution outcomes or forms, deterministically choose
    an available result per save, including eligible regional forms.
  - Specify the eligible form pool, branch identity, evolution trigger, and handling
    of species whose regional form normally requires a different base form in the
    authoritative evolution ledger.
- World items:
  - Remove hidden item spots. Randomize only eligible visible overworld item
    pickups deterministically from the save seed and a stable pickup identity.
  - Review the distribution and usefulness of available pickups across the game.
    Add new visible pickup spots where the balance review identifies a resource
    or progression gap; do not add new hidden spots. Prefer rewards guarded by
    optional trainers, with placement that makes the battle and reward clearly
    avoidable rather than blocking required progression.
  - Restrict the initial replacement pool to held items and evolution items that are
    usable during a Nuzlocke.
  - Do not infer usefulness from `ITEM_TYPE_EVOLUTION_ITEM`: all species-specific
    evolution routes have been consolidated into stones or the Linking Cord.
    Remove retired evolution items from the pool when they have no other useful
    battle or field effect. Items such as King's Rock, Metal Coat, Razor Claw,
    Razor Fang, Deep Sea Tooth, and Deep Sea Scale require an explicit held-effect
    decision instead of automatic exclusion.
  - Review consumables against the intended Nuzlocke rules. X-items should be
    excluded if battle stat consumables are not part of the format. Potions and
    other basic healing should be excluded if free Portable Healer access makes
    them dead or disappointing rewards; otherwise define which healing tiers are
    worthwhile rather than admitting the entire medicine pocket.
  - Specify key-item/TM handling, duplicate policy, pickup respawn behavior, and any
    progression-critical exclusions before hooking item scripts.

## Test Plan
- Build with `make -j$(nproc)`.
- Start two new saves and confirm encounter/ability/learnset randomization differs between saves.
- Confirm the three starter choices are stable within a save, differ across suitable
  seeds, and match the Pokemon granted to the player.
- Confirm an enemy trainer's randomized party is stable when repeated in one save,
  differs across suitable saves, respects the configured levels and BST policy,
  and contains legal moves and abilities.
- Confirm separate rival encounters randomize independently without requiring a
  persistent starter or evolution line.
- Within one save, confirm repeated encounters of the same species have consistent ability and learnset.
- Confirm evolution preserves the randomized ability behavior for that evolution line.
- Confirm Pokemon at cap gain no EXP and Rare Candy/EXP Candy cannot exceed cap.
- Confirm EVs do not increase from battle or EV items.
- Confirm friendship evolution species evolve by the new level thresholds.
- Confirm day/night evolution lines and alternate/regional-form branches are
  obtainable without waiting for a real-time window and remain stable within a save.
- Confirm hidden item spots cannot be collected and visible world item
  replacements are deterministic within a save, differ between saves, and never
  produce an item outside the approved Nuzlocke-useful pool of
  standard stones, Linking Cord, and independently useful held items.
- Confirm the party-menu and Pokemon Storage level-cap actions never exceed the active
  cap, process all intervening move-learning and evolution opportunities in order,
  resume safely after cancellation or interruption, and correctly persist changes to
  party and boxed Pokemon without requiring a free party slot.
- Confirm the Portable Healer works outside battle and is blocked or harmless in battle.

## Assumptions
- Development is based on `expansion/1.16.2` and integrated into `romhack/main`.
- Pokemon pool is all enabled species/forms through Gen 9, filtered for validity.
- Randomization is per-save deterministic, not build-time static and not rerolled every encounter.
- Move randomization means randomized learnsets, not randomized move effects/power/type.
- Initial level caps use Emerald's existing cap values.
