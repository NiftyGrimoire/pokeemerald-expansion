# Randomizer Implementation Handoff

## Current branch

- Integration branch: `romhack/main`.
- Base: `pokeemerald-expansion` stable `1.16.2`.
- Push only to the NiftyGrimoire fork through `origin`.
- Never push to `upstream`.

At the time of this handoff, the local branch contains these unpushed commits:

- `c796e1202e` — Refine randomizer implementation phases.
- `c9aac2dee8` — Add randomizer foundation.
- `4b693b2628` — Bound autonomous worker execution.

Verify the current Git state rather than assuming these commits remain unpushed.

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

No encounter, ability, learnset, progression, evolution, or quality-of-life hook
has been implemented yet.

## Validation already performed

- Three seed/hash tests passed.
- One species eligibility test passed.
- The `SaveBlock3` layout test passed.
- Ten OpenRouter worker unit tests passed after its execution limits changed.

Rerun relevant checks after any new integration.

## Recommended next phase

Phase 2 encounter work is implemented on `romhack/randomizer-encounters`,
pending review and approval to merge. Its scope policy is:

- Randomize standard grass/cave, Surf, fishing-rod, Rock Smash, mass-outbreak,
  and Feebas encounters.
- Preserve the vanilla method, selected slot, level, encounter-rate checks,
  Repel checks, and encounter-influencing ability checks.
- Leave roamers, scripted/static encounters, and Battle Pike/Pyramid encounters
  unchanged.
- Leave DexNav and overworld-visible encounters unchanged. Both systems are
  disabled in the current build and require separate identity and UI policies
  before being enabled.

The intended core behavior is to preserve the vanilla encounter method, selected
slot rarity, and level, then deterministically replace only the species with an
eligible species derived from the save seed and explicit encounter context.

Validation on the encounter branch:

- The encounter resolver determinism, eligibility, and context-separation test
  passes.
- The expanded species eligibility test passes.
- All 15 existing random-mon-generation tests pass after sharing its form-safety
  helper with the gameplay randomizer.
- A normal `make -j4` ROM build succeeds.

Codex should first locate and document the smallest common hook or the necessary
separate hooks. Once signatures and exact files are known, delegate only the
mechanical resolver or one hook at a time.

## Unresolved architecture

- Legendary encounters: locate every scripted/static creation path and define
  stable context keys that distinguish separate encounters without depending on
  mutable RNG. The initial candidate pool is enabled, usable restricted
  Legendary, sub-Legendary, and Paradox species; Mythicals and Ultra Beasts are
  excluded.
- Ability randomization: decide whether the stored/resolved result is an ability
  ID or an ability slot. The same slot across an evolution family does not imply
  the same actual ability.
- Evolution-family identity: define the exact base-family rule, including branches
  and regional forms.
- Learnsets: define the move candidate pool, weighting, duplicate rules, special
  move exclusions, and evolution behavior.
- Friendship evolution replacements: create an explicit species-level conversion
  table.
- Level Capper: specify exact level-up, move-learning, and evolution behavior.
