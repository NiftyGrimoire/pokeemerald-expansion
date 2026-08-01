# Randomizer Implementation Handoff

## Current branch

- Integration branch: `romhack/main`.
- Base: `pokeemerald-expansion` stable `1.16.2`.
- Push only to the NiftyGrimoire fork through `origin`.
- Never push to `upstream`.

The ability randomizer was merged locally into `romhack/main` by `ea8674abb9`.
Verify the current Git state and remote tracking state before committing or
pushing; this handoff does not authorize a push.

## Primary plan

Read `AgentDocs/hack-plans/gameplay-randomizer-overview.md` before implementing
another phase. Its Architecture Decisions and Phase 1 Contract supersede the older
broad summary where they differ.

## Completed foundation

- Master randomizer config gate in `include/config/randomizer.h`.
- Version 1 algorithm identifier.
- Per-save nonzero `u32 randomizerSeed` and algorithm version in `SaveBlock3`.
- Seed initialization during `NewGameInitData`.
- Stateless `RandomizerHash` with separate encounter, ability, and learnset
  categories.
- General species eligibility helper excluding invalid, disabled, egg, Mega,
  Primal, Ultra Burst, Gigantamax, Tera, and Totem species.
- Focused tests in `test/randomizer.c`.
- Intentional `SaveBlock3` size guard updated from 4 to 8 bytes.

Wild encounter randomization, BST-scaled ordinary encounter pools, scripted
encounter randomization, level caps and EV removal, starter randomization, enemy
trainer randomization, and evolution-family ability randomization are merged into
`romhack/main`. Learnset randomization has begun with universal TM compatibility;
randomized level-up learnsets, evolution changes, and quality-of-life hooks have
not been implemented.

## Learnset phase in progress

- Every enabled real Pokemon species can learn every configured TM through the
  shared `CanLearnTeachableMove` compatibility seam.
- HM and move-tutor compatibility remains authored. Egg and invalid-species
  safeguards remain unchanged.
- Level-up learnsets are resolved at runtime through
  `GetSpeciesLevelUpLearnset`, covering initial moves, ordinary level-up and
  evolution learning, reminders, AI legality checks, and Pokedex displays.
- Each enabled species receives four starting moves at level 1. New moves are
  front-loaded across the cap bands with counts of 3, 3, 2, 2, 1, 1, 1, 1, 1,
  and 1 for 1-15, 15-19, 19-24, 24-29, 29-31, 31-33, 33-42, 42-46, 46-58,
  and 58-100 respectively. This produces exactly 20 moves without increasing
  the engine's normal level-up table capacity. Moves are deterministic per save,
  species, and slot, with no duplicates.
- Damaging-move weights follow a level-based target power from roughly 40 in the
  opening game to 115 at level 80. STAB and coverage multipliers remain, but all
  eligible power bands retain nonzero weight, so an unusually strong early move
  or weak late move remains possible.
- Status moves use basic, strong, and elite potency tiers. Basic effects are
  favored before level 24, strong setup/recovery/status/hazard effects from 24,
  and elite setup or exceptional utility from 42. All status moves whose effects
  set weather, including weather-plus-switch moves, belong to the strong tier.
  Per-move basic/strong/elite weights are 10/6/3 early, 2/24/8 from level 24,
  and 1/10/36 from level 42. These elevated strong and elite weights compensate
  for the larger basic-status candidate pool while every tier remains possible.
- Placeholder, Transform, Sketch, Dark Void, Hyperspace Fury, Aura Wheel,
  species-power-override, and Struggle moves are excluded. The authored move is
  the fallback if no candidate exists.
- The resolver regenerates into one small EWRAM buffer per accessor call. Add
  a broader cache only if profiling shows this scan is too expensive.
- Egg-move randomization and any deliberate evolution-family inheritance policy
  remain unimplemented. Gift, wild, and trainer Pokemon already use the shared
  initial-moves path unless their data supplies explicit moves.

## Validation already performed

- Three seed/hash tests passed.
- One species eligibility test passed.
- The `SaveBlock3` layout test passed.
- Ten OpenRouter worker unit tests passed after its execution limits changed.

Rerun relevant checks after any new integration.

## Completed encounter phase

Phase 2 encounter work from `romhack/randomizer-encounters` was merged through
PR #2. BST-scaled ordinary encounter pools from
`romhack/randomizer-encounter-bst` were subsequently merged locally into
`romhack/main`. Their scope policy is:

- Randomize standard grass/cave, Surf, fishing-rod, Rock Smash, mass-outbreak,
  and Feebas encounters.
- Preserve the vanilla method, selected slot, level, encounter-rate checks,
  Repel checks, and encounter-influencing ability checks.
- Leave roamers and Battle Pike/Pyramid encounters unchanged. Scripted encounters
  are handled separately by the scripted encounter phase.
- Leave DexNav and overworld-visible encounters unchanged. Both systems are
  disabled in the current build and require separate identity and UI policies
  before being enabled.

The intended core behavior is to preserve the vanilla encounter method, selected
slot rarity, and level, then deterministically replace only the species with an
eligible species derived from the save seed and explicit encounter context.

The BST branch derives difficulty from the weighted average of the fixed level
ranges in each land, Surf, Rock Smash, or rod-specific encounter table. Ordinary
encounters select uniformly from the corresponding inclusive BST band:

- Level 1-10: BST 180-360.
- Level 11-20: BST 240-420.
- Level 21-30: BST 300-480.
- Level 31-40: BST 360-540.
- Level 41-50: BST 420-600.
- Level 51+: BST 480-720.

Restricted Legendary, sub-Legendary, Mythical, Ultra Beast, and Paradox species
are excluded from ordinary pools. Empty bands expand outward by 60 BST per pass.

Validation on the encounter branches:

- The encounter resolver determinism, eligibility, and context-separation test
  passes.
- The expanded species eligibility test passes.
- All 15 existing random-mon-generation tests pass after sharing its form-safety
  helper with the gameplay randomizer.
- A normal `make -j4` ROM build succeeds.
- All seven focused randomizer tests pass with BST-boundary and special-species
  exclusion coverage on `romhack/randomizer-encounter-bst`.

Manual validation still required:

- Confirm early land encounters remain in low-BST pools.
- Confirm Surf and improved rods can access stronger pools on the same map.
- Confirm late-game tables select from the high-BST pools.
- Exercise Feebas, outbreaks, Sweet Scent, and double wild battles.
- Check that the two-pass species scan causes no perceptible encounter delay.

The fixed table-weight calculation and empty-band fallback are verified by source
review but do not have direct unit-test hooks. The configured species data contains
candidates in every preferred band, so the fallback cannot be triggered naturally
by the current test configuration.

## Completed scripted encounter phase

The scripted encounter implementation was merged through PR #3. It includes:

- A dedicated enabled config gate and hash category.
- A qualifying-species helper limited to enabled, generally usable restricted
  Legendary, sub-Legendary, and Paradox species.
- A deterministic resolver keyed by map ID, original species, and battle slot.
- A `ScrCmd_setwildbattle` hook that replaces only qualifying species before the
  existing creation flow.
- Ordinary `setwildbattle` encounters route through the regular BST-scaled
  resolver, using the scripted level as difficulty and a distinct scripted
  encounter type.
- Focused classification, determinism, context-separation, and passthrough tests.

Validation on the feature branch:

- All 13 tests in `test/randomizer.c` pass.
- A normal `make -j4` ROM build succeeds.
- Source review confirms the direct `src/berry.c` caller is not hooked.

Manual gameplay validation is still required for Groudon, Kyogre, Rayquaza,
Regirock, Regice, and Registeel, including capture/defeat flags and repeat-entry
behavior. FRLG static maps remain outside required Emerald gameplay coverage
until reachability is established.

The initial policy is:

- Randomize only scripted/static encounters whose original species is flagged
  restricted Legendary, sub-Legendary, or Paradox.
- Select uniformly from enabled, generally usable species with one of those same
  three classifications.
- Exclude Mythical Pokemon and Ultra Beasts.
- Preserve level, held item, personality-generation flow, battle type, scripts,
  event flags, and story progression.
- Use the saved randomizer seed and a stable explicit encounter identity. Do not
  read or advance either mutable RNG stream.

The primary scripted path is:

1. `setwildbattle` is defined in `asm/macros/event.inc`.
2. `ScrCmd_setwildbattle` in `src/scrcmd.c` reads one or two species, levels,
   and held items.
3. It calls `CreateScriptedWildMon` or `CreateScriptedDoubleWildMon` in
   `src/script_pokemon_util.c`.
4. `ScrCmd_dowildbattle` starts the already-created opponent through
   `BattleSetup_StartScriptedWildBattle` or its double-battle counterpart in
   `src/battle_setup.c`.

The enabled Emerald map scripts using `setwildbattle` for the initial special
pool include Groudon, Kyogre, Rayquaza, Regirock, Regice, and Registeel. Expansion
also contains FRLG map scripts for Mewtwo and the Kanto birds; confirm whether
those maps are reachable in this hack before treating them as required gameplay
coverage. Non-special scripted encounters such as Kecleon, Voltorb, Electrode,
and Sudowoodo use the regular BST-scaled encounter pool. Their fixed scripted
level selects the difficulty band. `CreateScriptedWildMon` also has a direct
non-script-command caller in `src/berry.c`; it remains unchanged because
randomization is applied in `ScrCmd_setwildbattle`, not in the creation helper.

Resolve the stable identity before coding. Map ID plus original species is enough
for the currently identified Emerald Legendary maps, but it is not a general
guarantee when a map contains multiple matching statics. Prefer passing an
explicit context from `ScrCmd_setwildbattle` (for example a stable script/map
identity) rather than inferring identity later from party state. Double scripted
battles also need a slot component so their two opponents cannot collide.

Suggested bounded OpenRouter chunks, after Codex makes the identity decision:

1. Modify only `include/randomizer.h` and `src/randomizer.c` to add the special
   classification helper/resolver and focused pure tests if the test seam belongs
   there.
2. After review, modify only `src/scrcmd.c` and the minimum required declarations
   to pass stable context and replace qualifying species before creation.
3. After review, modify only `test/randomizer.c` to cover classification,
   exclusions, determinism, context separation, and unchanged ordinary species.

Codex retains architecture, worktree integration, source review, build/test
validation, documentation, and commits. Workers remain isolated, uncommitted,
and may not push or merge.

## Completed level caps and EV removal phase

The implementation was merged locally into `romhack/main`. It:

- Enables hard experience caps using the existing Emerald badge-flag cap table.
- Prevents Rare Candies and EXP Candies from exceeding the active cap.
- Disables battle EV gain.
- Prevents EV-boosting items from bypassing the zero-EV cap.
- Removes a malformed duplicate `B_EV_CAP_VARIABLE` definition from the existing
  config block.
- Adds focused tests for cap progression, hard-cap EXP behavior, and EV
  suppression from battles and vitamins.

Validation on the feature branch:

- All three tests in `test/caps.c` pass.
- A normal `make -j4` ROM build succeeds.

Manual gameplay validation still required:

- Confirm battle EXP stops exactly at each active cap.
- Confirm Rare Candies and each enabled EXP Candy cannot exceed the cap.
- Confirm badge acquisition advances through 15, 19, 24, 29, 31, 33, 42, 46,
  and 58 before the post-Champion fallback to level 100.
- Confirm battles, vitamins, feathers, and EV-affecting berries cannot produce
  positive EVs from a fresh zero-EV Pokemon.

## Completed starter phase

The starter implementation from `romhack/randomizer-starters` is merged into
`romhack/main`. Manual gameplay checks listed below remain outstanding.

Call-path review:

- `GetStarterPokemon(slot)` in `src/starter_choose.c` is the shared player-facing
  seam. The selection labels, sprites, cries, confirmation screen, granted
  Pokemon, `IsStarterInParty`, and credits all resolve through it.
- `CB2_GiveStarter` stores only the selected slot in `VAR_STARTER_MON`, then
  grants the species returned by `GetStarterPokemon`. Keeping the slot as the
  stable identity avoids new save fields.
- Rival scripts on Route 103, Route 110, Route 119, Rustboro, and Lilycove switch
  on `VAR_STARTER_MON` and select fixed trainer IDs. Those parties contain the
  vanilla rival starter or its evolution as their final party member.
- The common creation seam for ordinary trainer parties is
  `CreateNPCTrainerPartyFromTrainer` in `src/battle_main.c`.
- `CreateNPCTrainerParty` has the trainer ID needed to give each configured
  encounter a stable identity. Keep that ID available when resolving randomized
  party species rather than deriving identity from a trainer pointer.
- The public `CreateNPCTrainerPartyFromTrainer` helper is also used to construct
  a player-controlled trainer party. Do not apply enemy randomization globally
  inside that helper without distinguishing opponent parties.
- `wild_encounter_ow.c` references Treecko, Torchic, and Mudkip only for doll
  object graphics and is unrelated to starter selection.

The player-facing starter resolver is implemented:

- Generate three distinct choices from the save seed and starter slot.
- Require generally eligible, enabled, non-special base-stage Pokemon with a
  base-stat total from 300 through 350 inclusive.
- Require a complete three-stage evolution line: the candidate has no
  pre-evolution, has a usable evolution, and at least one usable middle-stage
  evolution can evolve again.
- Exclude restricted Legendary, sub-Legendary, Mythical, Ultra Beast, Paradox,
  and battle-only species/forms.
- Resolve through `GetStarterPokemon`, keeping the selection UI, label, sprite,
  cry, granted Pokemon, party check, and credits consistent.
- Build the pre-evolution lookup once and cache all three choices by save seed so
  repeated UI lookups do not rescan the evolution graph.

Focused validation:

- All 16 tests in `test/randomizer.c` pass, including starter eligibility,
  uniqueness, determinism, save separation, and invalid-slot handling.
- A normal `make -j4` ROM build succeeds.

## Completed enemy trainer party phase

The chosen policy is per-encounter trainer randomization:

- Randomize every ordinary enemy trainer party, including May and Brendan.
- Treat each configured trainer ID as a separate encounter. Rival fights at
  different story points may therefore have unrelated teams; the rival does not
  need to retain or evolve one of the randomized starter choices.
- Make a party stable within a save by resolving each slot from the saved
  randomizer seed, randomizer algorithm version, trainer ID, and party slot.
  Reloading or repeating the same trainer encounter must not reroll its team.
- Different saves should normally produce different teams for the same trainer.
- Preserve the configured party size and levels. Preserve other trainer tuning
  only where it remains valid for the replacement species.
- Choose replacement species near the original species' BST so gym leaders,
  bosses, and ordinary trainers retain approximately their authored strength.
  Define and test the exact BST band and fallback before implementation.
- Generate a legal level-up moveset for a replacement instead of copying custom
  moves that may be illegal or unusable for it.
- Resolve abilities against the replacement species. The current creation code
  validates configured abilities against the original species, so simply
  replacing the `CreateMon` species argument is unsafe.
- Define handling for species-specific held items and battle gimmicks before
  enabling them on randomized replacements.
- Initially exclude Battle Frontier, Trainer Hill, e-Reader, Secret Base, and
  player-controlled trainer parties. The existing ordinary trainer creation path
  already distinguishes most of these battle types.

Add a dedicated trainer config gate and hash category, then add focused tests for
determinism, save separation, trainer/slot separation, BST bounds and fallback,
species eligibility, and rival encounters being independent trainer identities.

The pure resolver foundation is implemented on `romhack/main`:

- `RANDOMIZER_TRAINERS` is enabled and has a dedicated version-1 hash category.
- `GetRandomizedSpeciesForTrainer(originalSpecies, trainerId, partySlot)` keys
  each result by the save seed, algorithm version, trainer ID, original species,
  and configured party slot.
- The preferred inclusive pool is within 50 BST of the original species. An empty
  pool expands outward by 50 BST per pass.
- Candidates use the ordinary enabled, usable, non-special species policy,
  excluding restricted Legendary, sub-Legendary, Mythical, Ultra Beast, Paradox,
  and battle-only species/forms.
- Three focused tests cover determinism, eligibility, preferred BST bounds,
  seed/trainer/slot context separation, and invalid-species passthrough. All 19
  tests in `test/randomizer.c` pass.

Trainer party creation is now integrated:

- The private ordinary enemy path passes the configured trainer ID explicitly,
  including override trainers. The selected source `monIndex` is the stable party
  slot identity when trainer pools reorder entries.
- The existing public `CreateNPCTrainerPartyFromTrainer` remains unrandomized for
  player-controlled parties, debug/direct callers, and existing tests. The
  explicit `CreateRandomizedNPCTrainerPartyFromTrainer` entry point is used only
  by the ordinary enemy path and focused integration coverage.
- Randomized enemies keep configured party size, levels, held items, IVs, EVs,
  friendship, balls, shiny state, Dynamax level/permission, and Tera settings.
- Replacement species receive their legal initial level-up moves instead of the
  original custom moves and retain the legal ability selected by `CreateMon`.
- Configured nicknames and Gigantamax factor are suppressed when the species
  changes. Held items remain equipped; incompatible species-specific effects and
  transformation items remain inactive through existing battle validation.
- Secret Base, Battle Frontier, Trainer Hill, e-Reader, and player-controlled
  parties remain excluded. Disabling the trainer-randomizer config gate preserves
  the complete original creation behavior.

Focused validation:

- All 19 tests in `test/randomizer.c` pass.
- All 21 tests in `test/battle/trainer_control.c` pass, including randomized
  species creation, preserved trainer tuning, legal initial moves, and a valid
  replacement-species ability.
- A normal `make -j4` ROM build succeeds.

Manual gameplay validation still required:

- Repeat an ordinary trainer and confirm its party is stable in one save.
- Compare the same trainer across new saves and confirm the party changes.
- Exercise May/Brendan, a Gym Leader, an override trainer, and a trainer-pool
  encounter.
- Confirm player-controlled trainer parties and excluded facilities remain
  unchanged.
- Exercise held Mega/Z items, Dynamax, Gigantamax-configured originals, and Tera
  settings on replacement species.

## Completed ability randomization phase

Chosen representation:

- Resolve an ability ID at runtime. Do not rewrite the saved `abilityNum`.
- Key the result by the saved seed, algorithm version, and a stable evolution
  family identity. The individual species and saved ability slot are not hash
  inputs.
- Every ordinary member of a family therefore resolves to the same actual
  ability, rather than merely using the same slot number with species-specific
  results.
- Preserve the saved `abilityNum` for compatibility with existing Pokemon data,
  scripts, debug tools, and any form/gimmick species that must retain its
  authored ability.

Evolution-family identity:

- Treat the enabled evolution graph as a family, ignoring `EVO_SPLIT_FROM_EVO`
  display/back-reference entries.
- Walk real incoming evolution edges to the root. Branched evolutions share the
  same root and randomized ability.
- When malformed or unusual data gives a species more than one reachable root,
  use the lowest species ID as the deterministic family identity.
- Regional lines share an ability only when the configured evolution graph
  actually connects them to the same root. A distinct regional base form remains
  a distinct family. This avoids inventing relationships from National Dex
  numbers or naming conventions.
- Cache the computed family identity by species; repeated ability reads occur in
  hot battle, UI, and overworld paths and must not rescan the full graph.

Primary runtime seam:

- `GetAbilityBySpecies` in `src/pokemon.c` is the shared conversion from a
  species plus saved slot to an ability ID. It feeds `GetMonAbility`, battle
  initialization, party and summary UI, box/party conversion, AI, and most
  overworld ability checks.
- Apply randomization after the existing species/slot fallback has produced a
  valid authored ability. If the feature is disabled, the species is invalid, or
  the species/form is excepted, return that authored result unchanged.
- Pokedex code calls `GetAbilityBySpecies` for all three authored slots. Once the
  hook is enabled those calls will intentionally show the same randomized family
  ability; verify the layout does not print redundant entries.
- Direct `GetSpeciesAbility` callers describe authored species data or choose a
  saved slot. Do not globally replace that lower-level accessor.

Form and gimmick safety:

- `GetAbilityBySpecies` is consulted by the form-change engine as well as normal
  battle setup. Species whose authored ability enables, identifies, or preserves
  a battle form must pass through unchanged.
- The version-1 denylist covers Wonder Guard; all ability-driven form changes;
  Battle Bond, Zero to Hero, Commander, Embody Aspect, and Terapagos signature
  abilities; placeholder abilities; and explicitly unimplemented abilities.
- If any enabled evolution-family member has a denylisted authored ability, the
  entire family preserves its authored abilities. This keeps pre-evolutions
  consistent with form-dependent evolved species.
- Invalid, disabled, egg, battle-only, and otherwise unusable forms pass through
  unchanged.

Implementation:

- `RANDOMIZER_ABILITIES` enables the feature.
- `GetRandomizerEvolutionFamily` builds enabled evolution-graph components with
  minimum-species-ID union roots. Real evolution edges connect families;
  `EVO_SPLIT_FROM_EVO` entries do not.
- `GetRandomizedAbilityForSpecies` hashes the save seed, ability category,
  family identity, and algorithm version, then selects uniformly from the
  reviewed candidate pool.
- `GetAbilityBySpecies` applies the resolver after its existing authored-slot
  fallback. The saved `abilityNum` remains unchanged.
- Family identities, protected-family flags, and resolved abilities are cached
  in EWRAM. The ability cache resets if the save seed changes.

Focused validation:

- All 24 tests in `test/randomizer.c` pass.
- All 26 active tests in `test/pokemon.c` pass; its one pre-existing learnset
  size test remains known-failing.
- All three Stance Change tests pass.
- A normal `make -j4` ROM build succeeds.
- The Forecast suite's eleven self-contained Castform cases pass. Six mixed
  cases retain assumptions that unrelated weather-setting Pokemon expose their
  authored abilities and therefore need randomizer-aware test setup.

Manual gameplay validation still required:

- Compare the same ordinary family across new saves and confirm its ability
  changes while remaining stable within a save and through evolution.
- Check summary, party, box, Pokedex, and battle displays for agreement.
- Exercise representative overworld abilities and ordinary trainer Pokemon.
- Exercise Castform, Cherrim, Aegislash, Wishiwashi, Minior, Mimikyu, Cramorant,
  Eiscue, Morpeko, Zygarde, Silvally, Arceus, and Terapagos form behavior.

## Next phase: learnset randomization

Do not add the learnset config gate or hook until the version-1 policy resolves
all of the following:

- Candidate move pool, including explicit exclusions for unusable, scripted,
  field-only, form-specific, and otherwise special-case moves.
- Type weighting: exact STAB preference, coverage/status weighting, and behavior
  for typeless or form-changing species.
- Learnset shape: number of moves per level, duplicate policy, level-1 moves,
  evolution moves, and handling when the filtered pool is too small.
- Stable identity: exact hash keys for species, learn level, and move slot, plus
  whether evolution-family identity affects results.
- Runtime seam inventory covering level-up lookup, move learning after evolution,
  Move Reminder/relearner behavior, eggs, gifts, wild Pokemon, and trainer-party
  initial moves.
- Cache design only after the access pattern and memory cost are measured; EWRAM
  is currently 89.44% used in the normal build.

First implementation milestone after the policy is documented:

1. Add a pure candidate classifier and deterministic resolver API.
2. Add focused tests for determinism, seed/species/level/slot separation,
   weighting boundaries, exclusions, duplicates, and fallback behavior.
3. Integrate one shared level-up access seam, then validate evolution, reminder,
   wild, gift, and trainer creation paths independently.

## Other unresolved architecture

- Friendship evolution replacements: create an explicit species-level conversion
  table.
- Starter randomization: the player-facing three-choice policy is implemented.
  Rival parties are governed independently by the enemy-trainer policy above.
- Time-dependent and alternate-form evolutions: define a species-level policy that
  removes day/night availability barriers and deterministically selects eligible
  alternate or regional-form outcomes for a save.
- World item randomization: identify all visible and hidden pickup paths, define a
  stable pickup identity, and build an approved Nuzlocke-useful replacement pool
  limited initially to held items and evolution items. Explicitly decide key-item,
  TM, duplicate, respawn, and progression-critical-item handling.
- Level Capper: specify exact level-up, move-learning, and evolution behavior.
