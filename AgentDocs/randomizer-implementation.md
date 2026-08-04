# Randomizer Implementation Handoff

## Current branch

- Active release branch: `romhack/nuzlocke-1.1.0` at `4ebbfa896c`.
- The previous integrated baseline is tagged `1.0.0-nuzlocke` at
  `d932605110` and remains the current `romhack/main` tip.
- The 1.1.0 branch contains the reviewed TM, world-item, direct-gift, level-1
  learnset, EV, Day Care, reward-audit, and player-documentation changelist.
- Do not merge this branch into `romhack/main` without explicit user approval.
- Base: `pokeemerald-expansion` stable `1.16.2`.
- Push only to the NiftyGrimoire fork through `origin`.
- Never push to `upstream`.

Verify the current Git state and remote tracking state before committing or
pushing; this handoff does not authorize a push.

## Primary plan

Read `AgentDocs/hack-plans/gameplay-randomizer-overview.md` before implementing
another phase. Its Architecture Decisions and Phase 1 Contract supersede the older
broad summary where they differ.

Use `AgentDocs/manual-validation-checklist.md` for the consolidated manual
gameplay and release-validation procedure.

## Completed foundation

- Master randomizer config gate in `include/config/randomizer.h`.
- Version 2 algorithm identifier. Version 2 deliberately replaces the unreleased
  version-1 learnset mapping with bucketed selection; development saves created
  before this change are not supported.
- Per-save nonzero `u32 randomizerSeed` and algorithm version in `SaveBlock3`.
- Seed initialization during `NewGameInitData`.
- Stateless `RandomizerHash` with separate encounter, ability, and learnset
  categories.
- General species eligibility helper excluding invalid, disabled, egg, Mega,
  Primal, Ultra Burst, Gigantamax, Tera, and Totem species.
- Focused tests in `test/randomizer.c`.
- Intentional `SaveBlock3` size guard updated from 4 to 8 bytes.

Wild and scripted encounter randomization, level caps and EV removal, starters,
enemy trainers, evolution-family abilities, universal TM compatibility,
randomized level-up learnsets, streamlined evolutions, and the implemented QoL
features are present in the 1.0.0 baseline. The 1.1.0 branch adds randomized TM
mappings, visible world items, eligible direct gifts, the revised two-move
level-1 opening, the 64-slot Items pocket, the Route 117 Day Care closure, Shoal
Cave and finite-reward redesigns, and the completed transaction audit described
below.

## Learnset phase complete

- Every enabled real Pokemon species can learn every configured TM through the
  shared `CanLearnTeachableMove` compatibility seam.
- HM and move-tutor compatibility remains authored. Egg and invalid-species
  safeguards remain unchanged.
- Level-up learnsets are resolved at runtime through
  `GetSpeciesLevelUpLearnset`, covering initial moves, ordinary level-up and
  evolution learning, reminders, AI legality checks, and Pokedex displays.
- Each enabled species receives two starting moves at level 1. New moves are
  front-loaded across the cap bands with counts of 3, 3, 2, 2, 1, 1, 1, 1, 2,
  and 2 for 1-15, 15-19, 19-24, 24-29, 29-31, 31-33, 33-42, 42-46, 46-58,
  and 58-100 respectively. This keeps only five total learned moves available by
  level 15, adds one extra learn after the eighth-gym cap, adds one extra learn
  after the pre-Elite Four team-leader cap, and still produces exactly 20 moves
  without increasing the engine's normal level-up table capacity. Moves are
  deterministic per save, species, and slot, with no duplicates.
- The two level-1 slots use a dedicated aggressive opening bucket. Damaging
  moves at 40 power or below receive the dominant weight, moves from 41 through
  60 retain a small fallback weight, and stronger moves receive zero weight.
  Only basic-tier status moves are eligible at level 1. Later slots return to the
  normal level-scaled damaging and status-tier curves.
- Damaging-move weights follow a level-based target power from roughly 40 in the
  opening game to 115 at level 80. STAB moves use a 4x multiplier and coverage
  moves use 2x, strengthening the preference for same-type attacks. Before level
  24, each seven points away from the target removes one base-weight point;
  afterward the band widens to ten points. This makes the early curve reject
  overpowered attacks more aggressively without imposing a hard power clamp. All
  eligible power bands retain nonzero weight, so an unusually strong early move
  or weak late move remains possible. Power-distance penalties saturate at a base
  weight of one; this explicit saturation avoids unsigned underflow making
  extreme-power moves such as Self-Destruct, Explosion, and V-create dominant.
  Review any future changes to this curve against detrimental damaging moves, not
  just raw base power. Moves flagged by `IsExplosionMove`, recoil-heavy attacks,
  fixed-damage edge cases, and other high-power moves with severe drawbacks can
  become overrepresented if they sit near the level target power; Misty Explosion
  is the canonical late-game check case because it has only 100 listed power but
  still faints the user.
- Status moves use basic, strong, and elite potency tiers. Basic effects are
  favored before level 24, strong setup/recovery/status/hazard effects from 24,
  and elite setup or exceptional utility from 42. All status moves whose effects
  set weather, including weather-plus-switch moves, belong to the strong tier.
  Protect-like effects, including Protect, Detect, guard moves, and damaging or
  stat-lowering shield variants, also belong to the strong tier.
  Per-move basic/strong/elite weights are 13/8/4 early, 3/32/11 from level 24,
  and 1/13/48 from level 42. These are approximately 4/3 of the prior values so
  status moves retain their relationship to the increased STAB multiplier.
  Elevated strong and elite weights compensate for the larger basic-status
  candidate pool while every tier remains possible.
- Placeholder, Transform, Sketch, Dark Void, Hyperspace Fury, Aura Wheel,
  Tera Blast, species-power-override, and Struggle moves are excluded. The
  authored move is the fallback if no candidate exists.
- The full learnset resolver regenerates into one small EWRAM buffer per accessor
  call. Initial move assignment uses a behavior-preserving rolling window: it
  generates randomized slots sequentially only through the Pokemon's current
  level, retains the last four eligible moves, and skips every future-level slot.
  This keeps wild, gift, and trainer initial moves identical to the full table
  while reducing encounter-time move-pool scans, especially at low levels. Add a
  broader cache only if profiling shows the remaining scans are too expensive.
- Expansion already exposes the authoritative configured move metadata through
  `gMovesInfo`, but does not provide a prebuilt randomizer bucket index. Version 2
  builds that compact index once from `gMovesInfo` during New Game initialization,
  with lazy initialization covering a loaded save after boot. Damaging moves share
  buckets by exact type and normalized power; status moves share the existing
  basic, strong, and elite buckets. Encounter-time selection weighs buckets rather
  than rescanning and reclassifying every move. Previously selected moves are
  subtracted from their buckets, retaining the no-duplicates policy. The compact
  index costs about 7.5 KiB of EWRAM; the production build uses 242,192 of 262,144
  EWRAM bytes after this change.
- Egg moves intentionally remain authored while breeding is planned for removal.
  Gift, wild, and trainer Pokemon already use the shared initial-moves path unless
  their data supplies explicit moves.

Learnset branch validation:

- Nine focused tests cover universal TM compatibility, preserved HM/tutor/egg
  compatibility, deterministic schedules, cap-band distribution, weighted move
  strength and extreme-power saturation, exclusions, seed/species separation,
  and weather/status tiers.
- A normal `make -j4` ROM build succeeds.
- Manual gameplay checks remain in the follow-up section below.

## Foundation validation already performed

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

## Learnset manual follow-up

After integration into `romhack/main`:

1. Manually verify initial moves, ordinary level-up learning, evolution learning,
   Move Reminder output, Pokedex output, and trainer/wild/gift initial moves.
2. Check representative early-, middle-, and late-game species across multiple
   seeds for useful move variety and the intended power/status progression.
3. Profile learnset access only if gameplay shows visible delay; retain the single
   shared EWRAM buffer unless measurement justifies a broader cache.
4. Leave egg moves authored while breeding is slated for removal. Define a new
   policy only if egg moves receive a non-breeding acquisition path.

## Evolution phase complete

The authoritative species-by-species behavior is documented in
`AgentDocs/hack-plans/evolution-rules.md`.

- All enabled friendship evolutions use explicit level 20, 30, or 40 tiers.
- Audited clock-only restrictions are removed. Move and move-type requirements
  are replaced by explicit levels, and species-specific item requirements are
  consolidated into ordinary stones.
- Paired clock and regional outcomes are selected deterministically from the
  saved seed through the dedicated evolution hash category.
- The target filter is used by normal, trade, item, battle-special, overworld,
  and script-trigger evolution scans so a non-selected form cannot leak through
  another access mode.
- Milcery evolves with one of seven ordinary stones. The stone chooses its
  decoration and the save seed selects one of nine cream flavors, with no Sweet,
  spin, or clock requirement.
- The six New Mauville, Petalburg Woods, and Shoal Cave evolution routes are
  location-free: their duplicate level-up entries are removed and their existing
  Thunder, Leaf, or Ice Stone entries are the sole triggers.
- All trade methods are removed. Plain trades, Pumpkaboo forms, Karrablast, and
  Shelmet use the Linking Cord; former held-item trades are distributed across
  ordinary evolution stones to balance their usefulness.
- Happiny's Oval Stone, Gligar's Razor Fang, and both Sneasel forms' Razor Claw
  routes are likewise consolidated into Shiny, Moon, Dusk, and Dawn Stones.
- Dartrix evolves into either selected Decidueye form at level 34; the Hisuian
  route no longer waits until level 36.
- All move and move-type evolution checks are replaced by explicit levels so
  randomized learnsets cannot block evolution. Sylveon joins Espeon and Umbreon
  in one seeded level-30 Eevee choice; Eevee's stone routes remain available.
- Audited party, weather, battle-counter, walking, unavailable-currency, and
  unsupported-script routes use explicit levels. Gender still selects the two
  Basculegion forms, and Nincada retains its intentional Poké Ball requirement.
- Applin uses Fire, Sun, and Leaf Stones for Flapple, Appletun, and Dipplin
  respectively; Dipplin then evolves into Hydrapple at level 40.
- Every remaining species-specific evolution item is replaced by an ordinary
  stone. Branches remain player-controlled, source forms preserve antique or
  masterpiece outcomes, and `I_USE_EVO_HELD_ITEMS_FROM_BAG` is disabled.

Focused validation currently covers the explicit friendship tiers, an actual
zero-friendship level evolution, allowlisted condition bypasses, deterministic
and seed-separated branch selection, a clock-independent Rockruff evolution,
and a region-independent Pikachu stone evolution. Milcery coverage verifies one
stable flavor across all decorations and an actual stone evolution. The
location-route test verifies all six targets retain their
item evolution and no longer have a level-up route. An exhaustive enabled-species
scan verifies that no trade, move-dependent, inaccessible, or grind-heavy
evolution condition remains outside the intentional Shedinja Poké Ball check,
every item evolution uses a standard stone or Linking Cord, and all declared
replacement routes retain their intended target.

## Other unresolved architecture

- Move-bucket effective-power valuation remains a future task. Beneficial
  attacking stat changes should add 10 effective power per stage (including 20
  for a sharp two-stage change and summed multi-stat stages), while two-turn
  attacks should subtract 10. The authoritative task, required effect audit, and
  test cases are recorded under Phase 8 in
  `AgentDocs/hack-plans/gameplay-randomizer-overview.md`.
- Low-priority move-description cleanup: Blazing Torque is mechanically implemented correctly
  (80 power, 100% accuracy, 30% burn chance), but its description is still the
  upstream `"---"` placeholder. Audit and replace the player-facing placeholders
  for all five Torque moves together, since randomized learnsets can make these
  formerly Starmobile-only moves normally obtainable.
- Starter randomization: the player-facing three-choice policy is implemented.
  Rival parties are governed independently by the enemy-trainer policy above.
- Hidden item spots are disabled on `romhack/qol-remove-hidden-items`: direct
  interaction cannot collect them and the Itemfinder does not detect them.
- Emerald item and direct-gift randomization is implemented on the current
  feature branch. `GetRandomizedWorldItem` and `GetRandomizedFreeItem` use the
  save seed, map group/number, stable object or gift context, and authored item.
  The resolvers are stateless: duplicates are allowed, no pickup counters or new
  save data are used, and the single useful pool is pure random with no
  progression guarantees or bands.
  The generated pool excludes Poke Ball variants, HMs, TMs as replacement items,
  and pure money rewards. Pure money rewards include Nuggets, Mushrooms, Pearls,
  Pearl String, Stardust, Star Piece, Comet Shard, Rare Bone, and equivalent
  vendor-only treasures. Authored Poke Ball and money pickups become useful
  randomized rewards; authored key items, HMs, and TMs remain their authored
  item slots. Hidden items remain disabled and invalid special templates remain
  unchanged.
  The useful pool contains 67 equally weighted results: 11 evolution items and
  56 held items. It includes standard evolution stones, Linking Cord, selected
  evolution-held items with independent utility, all 18 type boosters, the
  Choice trio, Focus/Band items, accuracy and critical-hit lenses, and selected
  general and lower-tier utility held items. The latter includes Quick Claw,
  Loaded Dice, Bright Powder, White Herb, Power Herb, Covert Cloak, Grip Claw,
  the four weather rocks, Light Clay, Terrain Extender, and all four terrain
  seeds. All four terrain-setting moves and Surge abilities remain eligible for
  their respective randomizers. Medicine, X-items, berries, mixed-use
  sellables, retired species-specific evolution items, respawn behavior, and
  broader distribution remain audit work. Emerald item balls and eligible direct Emerald gifts
  now share deterministic resolvers; key items, HMs, TM slots, and berries remain
  authored. Shops, exchanges, prize tables, Battle Pyramid items, FRLG content,
  and internal transfers remain outside this randomizer. No new pickup spots are
  added.
- TM move randomization is implemented on `romhack/nuzlocke-1.1.0`. The existing
  50 TM slots map to 50 unique moves per save using the saved randomizer
  seed and algorithm version, without adding a 50-entry save table. Candidates
  come from the complete implemented practical move table, excluding
  invalid sentinels and placeholder-only or unusable special entries. Weighted
  selection favors high-power damaging buckets and high-tier status moves;
  lower-power and severe-drawback moves remain possible but receive drawback
  penalties. The randomized mapping flows through descriptions, Bag use,
  compatibility, Move Reminder, Pokedex, reverse lookups, and debug helpers.
  HM move mappings and compatibility remain authored.
  The randomizer algorithm version is now 5. Version 3 introduced randomized TM
  mappings; version 4 added world/free-item hash categories and revised the
  randomized level-up schedule and opening-move weighting; version 5 expanded
  the world/free-item result pool from 23 to 67 entries. Deterministic trainer,
  learnset, TM, and item mappings from earlier development versions are stale.

### TM move randomization versus level-up learnset randomization

The two systems deliberately share move-safety classification, but they solve
different gameplay problems and therefore use different identities, weighting,
and duplicate rules.

Shared candidate policy:

- Both systems begin with `IsMoveRandomizerEligible`. Invalid sentinels,
  placeholders, Transform, Sketch, Dark Void, species-power-override moves,
  Hyperspace Fury, Aura Wheel, Struggle, and Tera Blast are excluded.
- TM selection additionally calls `IsAuthoredHMMove`. An authored HM move may
  appear naturally in a species' randomized level-up learnset, but it may not
  occupy a randomized TM slot. This preserves one unambiguous HM reverse lookup
  and prevents a TM from duplicating Cut through Dive.
- Neither system changes move type, power, accuracy, effect, priority, PP, or
  other move data. They select existing implemented moves.

TM mapping rules:

- The unit of identity is the TM slot, not a species. `TM01` through `TM50` form
  one global mapping for the save, keyed by the save seed,
  `RANDOMIZER_CATEGORY_TM`, the one-based TM index, and the shrinking candidate
  count. The mapping is cached and rebuilt when the active save seed changes.
- Selection is weighted without replacement. After a move is assigned, it is
  removed from the candidate array before the next slot is selected. All 50 TMs
  are therefore unique within a save, although a TM move may also occur in one
  or more species' level-up learnsets.
- TM weighting is intentionally species-neutral and uses the late-game level-80
  curves. Damaging moves target approximately 115 power: moves near that target
  receive up to weight 12, while increasingly distant power bands fall toward a
  minimum weight of 1. Fixed- and level-based-damage moves with listed power 0
  are normalized to power 50 for weighting.
- Status moves use the late basic/strong/elite weights 1/13/48. Elite moves such
  as Spore, Shell Smash, Quiver Dance, and comparable reviewed effects are much
  more likely than basic status moves; strong setup, recovery, status, hazard,
  screen, weather, and Protect-like effects occupy the middle tier.
- Explosion/self-KO moves, ordinary recoil-effect moves, and recoil-on-miss
  moves retain access but have their computed weight divided by four, with a
  floor of 1. This is a TM-specific permanent-resource safeguard; high listed
  power alone should not make a severe-drawback move dominant.
- The randomized move is resolved everywhere the TM is presented or consumed:
  item description, Bag use, compatibility checks, Move Reminder and Pokedex
  displays, reverse move-to-item lookup, and debug helpers. The item remains its
  authored TM slot, so finding TM24 still gives TM24, but the taught move and
  description are save-dependent.
- Every enabled real Pokemon is compatible with every TM. This universal TM
  compatibility is separate from the move-selection algorithm.

Level-up learnset rules:

- The unit of identity is `(species, learnset slot)`, keyed with
  `RANDOMIZER_CATEGORY_LEARNSET`. Each species receives its own deterministic
  20-move schedule. The same species has the same moves in wild, gift, trainer,
  level-up, evolution-learning, Move Reminder, and Pokedex paths.
- Moves are selected without duplication inside one species' 20-move learnset,
  using the previously selected moves as an exclusion list. Different species
  may freely share moves, and no global uniqueness is intended.
- Weighting depends on both species type and the scheduled learn level.
  Damaging STAB buckets receive a 4x type multiplier, coverage receives 2x, and
  status receives 1x. The target damaging power rises with level from roughly 40
  early to 115 at level 80. Status weighting shifts from basic moves early to
  strong moves from level 24 and elite moves from level 42.
- The two level-1 slots use a special opening policy: attacks at 40 power or
  below receive the dominant weight, 41-60 power attacks receive a small
  fallback weight, stronger attacks receive zero, and only basic-tier status
  moves are eligible. Later slots use the normal level-scaled curves.
- Learnset weighting does not apply the TM system's blanket recoil/self-KO
  quarter-weight penalty. Its power curve, level placement, typing multipliers,
  and no-duplicate rule govern selection instead; detrimental damaging moves
  remain an explicit balance-review concern.
- If a valid move cannot be selected, the resolver returns the authored move as
  a safe fallback. TM initialization instead expects the reviewed candidate pool
  to contain substantially more than the 50 required unique moves.

In short: TM randomization builds one species-neutral, late-game-weighted set of
50 globally unique reusable teaching resources. Learnset randomization builds a
different type-aware, level-progressive set of 20 moves for every species.

## Committed 1.1.0 changelist review ledger

The release-sized changelist was reviewed, corrected, and committed as
`4ebbfa896c`. Preserve this ledger as implementation history and as a pointer to
the remaining manual-validation work. The detailed case-by-case reward decisions
are recorded below and in
`AgentDocs/pending-changelist-review-2026-08-04.md`.

The general Items pocket is expanded from 30 to 64 distinct-item slots to make
the broader randomized useful-item pool practical. Per-item stacks remain capped
at 999, and the other four Bag pocket capacities are unchanged. This adds 136
bytes to `SaveBlock1` (15,568 to 15,704 bytes) and intentionally changes its
layout; existing development saves created with the 30-slot layout are not
supported. Multi-reward scripts must still use correct all-or-nothing or retry
semantics rather than assuming that a 64-slot pocket can never fill.

| # | Original finding | Current status | Remaining work |
|---|---|---|---|
| 1 | Shoal Cave randomized its required Salt/Shell ingredients and Shell Bell exchange, making the authored loop impossible. | Resolved by intentional redesign. Shoal Cave is permanently low tide, the ingredient/exchange loop is retired, the four reachable Salt spots are one-time randomized pickups, Shell spots are inaccessible, and neither ingredient can be generated by the replacement pool. | Manual gameplay validation remains. Reconsider the entire cave only during the later streamlining pass. |
| 2 | The Oldale package randomized 200 Ultra Balls into 200 copies of one replacement item. | Resolved. The grant is authored as 200 Ultra Balls plus ¥200,000, once. | Manually validate successful receipt and the full-Bag retry path. |
| 3 | Randomized TMs displayed the authored TM slot's move description. | Resolved. TM01-TM50 descriptions now resolve from the randomized move while their item names remain stable TM slot names. | Manually inspect representative Bag descriptions and retain focused automated coverage. |
| 4 | Authored HM moves could also occupy randomized TM slots, creating duplicate and ambiguous reverse mappings. | Resolved. Every authored HM move is excluded from randomized TM candidates and the invariant is tested across multiple seeds. | Manual Bag/use validation remains. |
| 5 | Exchanges, vendors, repeatable systems, and prize systems were broadly converted with `giveitem_randomized` even though they were outside the original direct-gift phase. | Resolved case by case. Authored transactions were restored where appropriate; selected vendors and challenge rewards retain documented deterministic randomization; daily and peripheral systems were made one-time, disabled, or deliberately redesigned; fossils and the E-Reader delivery are authored. See the completed decision queue below. | Perform the separate future streamlining and whole-game vendor audits already documented; do not silently extend randomization to unaudited transactions. |
| 6 | The randomized TM candidate filter admitted moves known to be unsuitable, with Tera Blast explicitly excused by a test. | Resolved. TM selection reuses the reviewed practical-move eligibility gate, excludes Tera Blast and other unusable/context-dependent cases, and separately excludes every authored HM. The detailed TM-versus-learnset contract above records the intentional identity, weighting, uniqueness, drawback, and integration differences. | Revisit only if the TM-specific balance policy is deliberately broadened. |
| 7 | Authored quantities were copied blindly to randomized replacements, producing disproportionate stacks such as five or six identical held/evolution items. | Resolved for every retained multi-item gift in this changelist. Birch's tutorial is restored to five authored Poke Balls; Oldale is authored; Seashore's six-copy challenge reward is an explicit temporary exception. | Reconsider Seashore only when its fights, reward, and vendor are disabled together during streamlining. |
| 8 | Tests cover pure randomizer resolution better than the actual script integration risks. | Partially addressed. TM descriptions and HM/TM collision coverage were added, and normal ROM builds now succeed after the hidden-item flag-zero build blocker was fixed. | Add or perform coverage for ordinary item-ball templates, `RandomizeFreeItemGift` variable handling, protected story-item macros, exchanges/repeatable rewards, multi-item quantities, and Bag-full retry stability. Keep the manual checklist as the gameplay gate where script-level unit coverage is impractical. |

### Completed special-reward decision queue

Already decided and implemented cases—Cozmo's Meteorite exchange, Harbor Mail,
the Scanner evolution-item selector, the Mauville Bike Shop, the Powder Jar and
Berry Powder vendor restoration, both Seashore House rewards, the Mt. Chimney
randomized vendor, the Trick House prize table, the Battle Frontier symbol
berries, the one-time Route 120 Berry, Berry Master's wife's one-time reward,
the disabled Seedot/Lotad size rewards, the restored Contest scarves, the
always-visible Mirage Tower and authored Mirage/Underpass fossils, and the Shoal
Cave redesign, plus the restored E-Reader Enigma Berry delivery—remain listed in
the status table above and should not be reprocessed as outstanding CSV work.

Sootopolis Kiri now gives her original two-Berry reward only once per save: one
random Pomeg-through-Nomel Berry plus either Figy or Iapapa. A preflight simulates
both additions against the Berry pocket before either grant occurs, preventing a
partial first grant when the second Berry would not fit. A permanent received
flag is set only after both authored items are successfully received.

Berry Master's wife no longer opens the Easy Chat phrase minigame. She gives
exactly one authored random Berry per save from the five former special-phrase
rewards: Spelon, Pamtre, Watmel, Durin, or Belue. A permanent flag replaces the
daily flag and is set only after the Berry enters the Bag, preserving full-Bag
retry behavior.

The Seedot/Lotad size-judging interaction and record displays remain as flavor
content, but neither brother grants an item for a new qualifying record. Their
successful-record dialogue no longer promises a reward. This removes the
repeatable randomized-item source without removing the harmless judging system.

The Slateport Fan Club chairman's five condition awards are restored to their
authored Red, Blue, Pink, Green, and Yellow Scarves. Each received flag is set
only after the matching scarf successfully enters the Bag. These Contest-only
rewards are temporary compatibility behavior; the later streamlining pass will
disable the Contest system and retire this assessment interaction with it.

Mirage Tower is always visible while its fossil event is unfinished; the former
per-map-load random visibility roll is removed. Choosing its Root Fossil or Claw
Fossil now grants that authored fossil, preserves the existing choice flags and
collapse sequence, and remains safely retryable when the Bag is full. A broader
choice from fossils across generations is deferred because it also requires an
explicit policy for presentation, revival, and the later Desert Underpass item.

Desert Underpass now grants the authored fossil not chosen in Mirage Tower:
Claw after choosing Root, or Root after choosing Claw. Its existing full-Bag
retry and object-removal behavior remain intact. The tunnel still opens only
after game clear, so this recovery does not contribute to the primary Nuzlocke
route.

The E-Reader delivery path once again grants `ITEM_ENIGMA_BERRY_E_READER`
directly. Its validation, Bag/PC ownership checks, downloaded custom Berry data,
availability variable, and full-Bag retry behavior remain unchanged.

Future streamlining must audit all E-Reader, Mystery Gift, and other external
event items and unlocks that cannot be obtained through the intended standalone
play loop. This includes the Aurora Ticket and Birth Island/Deoxys, plus the
Mystic Ticket, Old Sea Map, Eon Ticket, and any comparable external delivery.
For each, deliberately provide an in-game acquisition path, enable the content
by progression, or remove it; do not leave encounters or rewards silently
inaccessible merely because their original distribution service is unavailable.

After the current special-reward audit, perform a separate whole-game vendor
audit. Decide each shop or repeatable seller independently and document its
inventory policy, price, repeatability, progression role, and exploit risk.
Mt. Chimney and Seashore House are approved isolated exceptions to the present
authored-vendor default; neither is a precedent that silently randomizes other
vendors.
- Summary IV view is implemented on `romhack/qol-summary-ivs`. On the Pokemon
  Skills page, A toggles between the normal stat display and all six numeric
  stored IVs, then back to stats. EV cycling is disabled, the page label changes
  between STATS and IVs, and Hyper Training does not replace displayed stored IVs
  with 31. The feature is display-only and does not mutate Pokemon data.
- Portable Healer and Repel Toggle are integrated into `romhack/main`. Both are
  reusable Key Items granted when the starter is created. Portable Healer
  restores HP, PP, and status for every party
  Pokemon outside battle. Repel Toggle stores an indefinite active state in the
  existing repel variable and blocks all standard random encounters regardless of
  the lead Pokemon's level. It remains active while walking until explicitly
  disabled. Scripted/static and deliberately triggered encounters remain
  available. Because spray and Lure state continues to use the existing shared
  variable, a Lure replaces the toggle state; disable the toggle before using an
  ordinary Repel spray.
- Badge-authorized HM actions are implemented on `romhack/qol-badge-hms`. A
  shared field-move user selector preserves every existing badge gate and, once
  unlocked, selects the first non-Egg party Pokemon without requiring the HM in
  its moveset. Direct overworld interactions use the same selector. The party
  menu exposes only unlocked HMs that are usable in the current field context,
  prioritizing them within its fixed action capacity; learned non-HM field moves
  retain their original requirements.
- Oldale supply grant is implemented on `romhack/qol-oldale-supplies`. The
  stationary girl in Oldale gives 200 Ultra Balls followed by ¥200,000 exactly
  once. A dedicated flag is set only after the item grant succeeds, so a full Bag
  does not consume the reward and the player can retry after making room.
- Battle style is globally locked to Set on `romhack/qol-no-free-switch`. New
  saves default to Set, existing saves resolve as Set when a battle starts, and
  the obsolete Battle Style row is removed from the Options menu. This suppresses
  the free switch prompt after an opposing trainer Pokemon faints while leaving
  ordinary voluntary switching unchanged. The focused global-style test passes,
  and a normal ROM build succeeds.
- Trainer AI tiers are implemented on `romhack/trainer-ai-tiers`. Ordinary
  trainers receive Basic Trainer scoring, Smart Mon Choices, and randomized
  equivalent switch-ins without voluntary switching. Rivals, villain admins and
  leaders, Gym Leaders, Elite Four, and Champion also receive Smart Switching,
  plausible STAB/status assumptions, and PP-stall prevention. Omniscient and
  Tera knowledge are excluded, authored Risky behavior is retained, and
  facilities plus player-controlled partners keep their existing AI policy.
  Opposing abilities remain unknown until revealed in battle; afterward,
  recorded abilities such as Lightning Rod inform subsequent move selection.
  The investigation and follow-up validation targets are documented in
  `AgentDocs/trainer-ai-investigation.md`.
- Level-to-cap action: implementation is integrated into `romhack/main`.
  `TryAdvanceMonOneLevelToCap` provides the shared
  one-level stat/experience step and stops at the active cap. The field party
  menu exposes `LEVEL TO CAP` and runs the existing stat, move-learning, and
  evolution presentation after every individual level before continuing.
  Evolution scenes now retain the supplied Pokemon pointer instead of looking
  it up again by party index, including their move-replacement summary screen;
  this permits the Storage `LEVEL TO CAP` action to operate on a persistent
  temporary Pokemon without occupying a party slot. Storage advances one level
  at a time, displays a level-up message plus stat-gain and resulting-stat pages
  for each level, prompts before opening move replacement, returns to the visible
  Storage UI between ordinary levels, and runs each evolution scene. It writes
  the completed record back to its
  original party or box position. If the
  player stops an evolution, both entry points stop the larger level-to-cap
  operation at that point and retain the levels already processed.
  Storage checkpoints the source party or box record after each completed level,
  move decision, and evolution rather than deferring persistence until the end.
  The focused cap suite passes 4/4 and the ROM builds successfully. A broad test
  run also passed the level-to-cap and party-navigation coverage but remains
  globally red because of unrelated existing overworld-ability and battle-test
  failures; use the focused suite as this feature's automated gate.
- Battle and item EV gains remain disabled through the existing
  `B_EV_CAP_TYPE = EV_CAP_NO_GAIN` toggle. The randomizer useful-item pool also
  excludes all EV-effect items, including vitamins and feathers; berries remain
  outside the general pool as a separate policy. Authored/debug/script EV edits
  remain distinct from normal player acquisition.
- Streamlined progression: the Emerald Route 117 Day Care entrance is blocked
  by a stationary closure NPC at the approach to the entrance warp. The NPC
  explains that the DAY CARE is closed and does not alter the underlying daycare
  save data or shared breeding scripts. Shoal Cave is permanently locked to its
  low-tide entrance and inner layouts so the full low-tide route and its item
  pickups, including TM07, are always accessible. As an intentional design
  change, the renewable Shoal ingredient and Shell Bell loop is removed: the old
  man only remarks that the cave is always at low tide, performs no ingredient
  checks or exchange, and no longer runs the daily collection reset. The four
  reachable former Shoal Salt points retain their individual collection flags
  and act as one-time randomized useful-item pickups. The four former Shoal Shell
  points belong to the unavailable high-tide route and cannot be collected.
  Neither Shoal Salt nor Shoal Shell is present in the randomizer replacement
  pool, so randomized pickups and gifts cannot introduce obsolete ingredients.
  A later streamlining pass may disable or bypass Shoal Cave after relocating any
  rewards that remain important. Full removal or replacement of breeding,
  egg production, inherited moves, berry planting/growth/harvesting, and their
  dependent rewards remains open. If berry harvesting remains, keep it in a separate
  berry-only randomizer and out of the general useful-item pool. Audit the Day
  Care, eggs, inherited moves,
  breeding-only content, berry plots, berry tutorials and NPCs, and any dependent
  rewards or encounters so important content receives a direct replacement path.
  The berry pass must disable planting, watering, growth, and harvesting; retire
  the 20 berries with no held effect (Razz, Bluk, Nanab, Wepear, Pinap, Pomeg,
  Kelpsy, Qualot, Hondew, Grepa, Tamato, Cornn, Magost, Rabuta, Nomel, Spelon,
  Pamtre, Watmel, Durin, and Belue); keep the e-Reader Enigma special slot out of
  normal distribution; and provide finite direct access to retained battle-useful
  berries after auditing Pokeblock and NPC dependencies.
  Also audit obsolete EV content, mandatory grinding against each cap, and
  optional plot detours before choosing concrete EXP, trainer, encounter, or
  script changes. The Powder Jar and all Berry Powder vendor purchases remain
  authored for now; the later streamlining pass will retire Berry Powder and
  remove its production, UI, vendor, dialogue, and dependent checks together.
  The Harbor Mail-for-Coin Case exchange also remains authored for now; the
  later streamlining pass will retire the Coin Case and Game Corner coin loop,
  audit and relocate any retained prizes, and choose a finite replacement for
  the Harbor Mail trade or remove that dependency. The authoritative cleanup
  scopes and prerequisites for both systems are recorded in the Phase 12 plan.
  The Scanner exchange is deliberately redesigned as
  a one-time player-selected evolution resource. Captain Stern offers a
  scrollable menu containing Fire, Water, Thunder, Leaf, Ice, Sun, Moon, Shiny,
  Dusk, and Dawn Stones plus the Linking Cord. The selected reward is authored
  rather than randomized. Declining the confirmation, backing out of the menu,
  or failing the Bag-space check preserves the Scanner; Stern removes it and
  sets the exchange flag only after the selected item is successfully received.
