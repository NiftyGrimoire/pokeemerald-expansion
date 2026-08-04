# Pending Changelist Review — 2026-08-04

## Purpose and final status

This document began as the working review queue for the former uncommitted
changelist on `romhack/disable-battle-ev-gain`. The findings were resolved and
the full release-sized change was committed on `romhack/nuzlocke-1.1.0` as
`4ebbfa896c`. The document is now a historical design/review ledger, not an open
implementation queue. Manual gameplay validation remains tracked separately in
`AgentDocs/manual-validation-checklist.md`.

## Changelist design inventory

The committed changelist combines five headline gameplay changes with a
supporting item/reward audit and several progression decisions. Treat the list
below as the release-note boundary for this CL; broader Phase 12 streamlining is
still future work.

### Headline systems

1. **Disable EV gain.** Battle EV gain and EV-item gain remain disabled through
   `B_EV_CAP_TYPE = EV_CAP_NO_GAIN`. Vitamins, feathers, and other EV-effect
   items are excluded from the randomized useful-item pool. Authored script and
   debug mutations are not redefined by this policy.
2. **Disable practical access to breeding.** A stationary closure NPC blocks the
   Route 117 Day Care entrance and explains that the service is closed. This is
   an access-level disable only: the underlying Day Care save data, shared
   breeding engine, egg moves, and other egg-related content are not removed in
   this CL and still require a later cleanup audit.
3. **Rebalance randomized level-1 learnsets.** Each species now begins with two
   randomized level-1 moves instead of four. The opening bucket strongly favors
   damaging moves at 40 power or below, permits a small 41–60-power fallback,
   rejects stronger attacks, and admits only basic-tier status moves. The two
   displaced starting slots are redistributed to levels 50 and 60, preserving
   the 20-move table while giving the post-eighth-gym and post-team-leader bands
   two moves each. This mapping change is part of algorithm version 4.
4. **Randomize TM moves.** The existing 50 TM items map to 50 unique,
   deterministic moves per save. Selection favors high-power attacks and
   high-tier status moves, penalizes severe drawbacks, uses the reviewed
   practical-move eligibility policy, and excludes Tera Blast plus every
   authored HM move. Names remain TM slot names; descriptions, Bag use,
   compatibility, Move Reminder, Pokedex, reverse lookup, and debug paths all
   resolve the randomized move. HMs remain authored, including HM08 Dive.
5. **Randomize world items and eligible free gifts.** Visible Emerald item balls
   and eligible direct NPC/story gifts resolve deterministically from the save
   seed and stable map/object or gift context. The useful pool contains standard
   evolution stones, Linking Cord, selected independently useful
   evolution-held items, and broadly useful held items. Duplicates are allowed;
   there are no progression bands or guaranteed resources. Key items, HMs, TM
   item slots, Berries, Medicine, X-items, Poke Ball replacements, pure-money
   replacements, hidden items, shops, exchanges, prize tables, Battle Pyramid
   generation, FRLG content, and internal transfers are excluded unless an
   explicit exception below says otherwise.

### Supporting inventory and acquisition changes

- The general Items pocket expands from 30 to 64 distinct stacks. Other pocket
  sizes and the 999-per-stack limit are unchanged. This intentionally changes
  `SaveBlock1`; development saves using the old layout are unsupported.
- Hidden item spots remain unavailable and undetectable by the Itemfinder.
  Zero-flag cross-version placeholders use a true no-op event rather than the
  test-signpost fallback.
- Authored Poke Ball and pure-money overworld pickups may become useful random
  items, but the Birch Lab catching tutorial is restored to five authored Poke
  Balls. Oldale separately grants 200 Ultra Balls and ¥200,000 once, with a
  full-Bag retry path.
- Mt. Chimney and Seashore House are deliberate vendor exceptions. Each sells
  one fixed randomized useful item repeatedly at the authored price (¥200 and
  ¥300 respectively), with space checks against the resolved item and atomic
  payment. The Seashore challenge still grants six copies of one fixed
  randomized reward. No other vendor inherits this policy automatically.
- Trick House keeps mixed behavior: eligible non-TM puzzle prizes randomize,
  while its authored TM slot remains a TM and teaches the save-randomized move.
- The Mystery Gift Battle Card prize is eligible for direct-gift randomization
  and is retryable when the Bag is full. External-event key items and the custom
  E-Reader Enigma Berry retain authored delivery behavior.

### Streamlined progression and finite-reward changes

- Shoal Cave is permanently low tide, keeping the low-tide route and TM07
  continuously accessible. The renewable Salt/Shell collection and Shell Bell
  exchange are retired; four reachable Salt spots become one-time randomized
  pickups, Shell spots are inaccessible, and neither ingredient can appear in
  randomized pools.
- Captain Stern's Scanner exchange becomes a one-time player choice among the
  ten standard evolution stones and Linking Cord. The selected reward is
  authored, and the Scanner is consumed only after successful Bag insertion.
- Mirage Tower remains visible until its fossil event is completed. Its
  Root/Claw choice stays authored, and Desert Underpass later grants the other
  authored fossil.
- Route 120's Berry NPC becomes one-time instead of daily. Kiri gives her
  authored two-Berry bundle once with atomic capacity preflight. Berry Master's
  wife drops the Easy Chat phrase minigame and gives one of her five former
  special Berries once. All three remain retryable on Bag failure.
- Sootopolis Seedot/Lotad size judging remains as flavor and record tracking but
  no longer grants repeatable items. Slateport condition judging retains the
  five authored Contest Scarves with safe one-time receipt.
- Authored progression transactions restored by the reward audit include
  Cozmo's Meteorite/TM27 exchange, Harbor Mail/Coin Case, both Bike Shop grants,
  Powder Jar and Berry Powder purchases, Battle Frontier symbol Berries,
  Contest Scarves, fossils, and E-Reader Enigma Berry delivery.

### Not part of this CL's design scope

- Full removal of the breeding engine, egg content, Berry farming, Contests,
  Berry Powder, Coin Case/Game Corner, and external event dependencies remains
  future streamlining work.
- Trainer randomization and global ability randomization are existing systems.
  This CL only updates deterministic trainer-test expectations because algorithm
  version 4 changes mappings; it does not introduce a new trainer policy.

## Priority queue

### 1. HM08 Dive is omitted by TM/HM bounds

**Severity:** High
**Type:** Functional progression defect
**Status:** Resolved. All valid TM/HM bounds are inclusive, and focused tests now
cover every authored HM plus direct HM08 Dive resolution and reverse lookup.

The last valid TM/HM table index is `NUM_ALL_MACHINES`, but the new code treats it as an exclusive upper bound in three places:

- `IsAuthoredHMMove` in `src/randomizer.c` uses `index < NUM_ALL_MACHINES`.
- `GetRandomizerTMHMMoveId` returns an authored move only when `index < NUM_ALL_MACHINES`.
- `GetRandomizerTMHMItemIdFromMoveId` searches HMs only while `index < NUM_ALL_MACHINES`.

Because HM08 Dive occupies the last index, the current behavior is:

- HM08 Dive resolves to `MOVE_NONE`.
- Reverse lookup cannot find the Dive HM item.
- Dive is not recognized as an authored HM and can enter the randomized TM candidate pool.
- Story progression that depends on teaching Dive can break.

The randomizer tests repeat the same exclusive bound, so the defect is not detected by the currently passing tests.

**Recommended resolution:** Use `<= NUM_ALL_MACHINES` for valid table traversal and lookup. Add explicit tests covering every authored HM, with a direct assertion for HM08 Dive and its reverse lookup.

### 2. Randomized vendors can charge money without delivering an item

**Severity:** High
**Type:** Functional transaction defect
**Status:** Resolved. Both vendors resolve the deterministic replacement before
checking space, charge only after that exact item fits, and refund the price if
the subsequent grant unexpectedly fails.

The Mt. Chimney and Seashore House vendors intentionally sell one deterministic randomized item repeatedly. Their scripts check space for the authored product before granting the replacement:

- `data/maps/MtChimney/scripts.inc` checks `ITEM_LAVA_COOKIE`, conditionally removes ¥200, and then calls `giveitem_randomized`.
- `data/maps/Route109_SeashoreHouse/scripts.inc` checks `ITEM_SODA_POP`, removes ¥300, and then calls `giveitem_randomized` without handling a failed grant.

Lava Cookie and Soda Pop use the Medicine pocket, while the randomized pool currently produces Items-pocket rewards. If the Medicine pocket has room but the Items pocket is full, the player can be charged without receiving anything.

**Recommended resolution:** Resolve the deterministic replacement before the purchase, check space for that exact replacement, and only charge once the grant is known to fit. Ensure both scripts handle an unexpected grant failure without consuming money.

**Design note:** These two vendors are deliberate exceptions to the general policy of restoring authored purchases. A future streamlining pass should review every vendor and explicitly decide which vendors, if any, remain randomized.

### 3. Kiri's two-Berry preflight does not match Berry-pocket stacking

**Severity:** Medium
**Type:** Functional reward-transaction edge case
**Status:** Resolved. The preflight now rejects a full existing Berry stack
instead of treating an empty slot as a legal duplicate stack. A focused Bag test
covers the full-stack-with-empty-slots case.

`SelectKiriBerryPair` in `src/script_pokemon_util.c` simulates whether both rewards fit. When a matching Berry stack is already at the maximum quantity, the simulation continues searching for an empty slot and treats a second stack as valid.

The real Berry-pocket insertion behavior does not allow that duplicate stack. This permits the following sequence:

1. The simulated preflight reports that both rewards fit.
2. The first Berry is granted.
3. The second Berry fails because its existing stack is full.
4. The permanent completion flag is not set.
5. The player can retry and potentially farm the first reward.

**Recommended resolution:** Make the simulation reproduce the real Berry-pocket rules exactly, including the full-existing-stack case. Alternatively, introduce a reusable atomic multi-item reward helper that preflights and grants through one shared implementation.

### 4. Mystery Gift Battle Card can consume a one-time reward on failure

**Severity:** Medium
**Type:** Functional integration defect
**Status:** Resolved. The completion flag is set only after a successful grant;
a failed grant displays a full-Bag retry message and leaves the reward available.

`data/scripts/gift_battle_card.inc` calls `giveitem_randomized ITEM_POTION`, ignores `VAR_RESULT`, and then sets `FLAG_MYSTERY_GIFT_DONE` unconditionally.

If the replacement cannot fit in the Items pocket, the player receives nothing and the reward is permanently marked complete. Randomizing the original Potion makes a new distinct stack more likely than the original behavior.

**Recommended resolution:** Set the completion flag only after a successful grant. Add a full-Bag message and allow the player to retry later.

### 5. Trainer-control tests fail

**Severity:** Medium until behavior is classified
**Type:** Technical validation failure
**Status:** Resolved. The authored Wobbuffet test now expects the shared global
ability resolver, matching the documented all-source ability policy. Algorithm
version 4 intentionally changes the fixed randomized trainer expectation to
Pyukumuku with Splishy Splash, Shadow Punch, Doom Desire, Triple Dive, and
Intimidate.

The focused `test/battle/trainer_control.c` run currently reports 20 passing and 2 failing tests:

- `CreateNPCTrainerPartyForTrainer generates customized Pokémon` expects the authored Wobbuffet to have Slow Start but `GetMonAbility` resolves Corrosion.
- `Randomized enemy trainer creation preserves tuning and generates species-safe data` expects Porygon for the fixed seed but now receives Pyukumuku.

The randomized-species failure is probably a stale deterministic expectation after algorithm changes. The authored-trainer ability failure needs investigation because it may expose an unintended interaction between authored trainer ability data and the global ability randomizer.

**Recommended resolution:** Determine the intended authored-trainer ability behavior before changing the assertion. If the gameplay is correct, update deterministic expected species, moves, and abilities together and document that the algorithm-version change intentionally invalidates old expected mappings.

### 6. TM candidate suitability remains incomplete

**Severity:** Medium
**Type:** Design and balance
**Status:** Resolved for the current policy. TM candidates now reuse the reviewed
general move-eligibility gate, which excludes Tera Blast and the other
unusable/context-dependent special cases, then additionally excludes authored
HM moves. TM weighting remains independently tuned.

The TM-specific eligibility filter does not exclude every move rejected by the level-up learnset randomizer. Tera Blast is the known example: the tests explicitly accept it as an exception even though `IsMoveRandomizerEligible` rejects it.

The larger design question is which technically implemented moves make good permanent, universally learnable TMs. Species-dependent, form-dependent, context-dependent, unusable, or highly progression-distorting moves need deliberate treatment.

**Recommended resolution:** Define a TM-specific exclusion policy and audit the candidate pool. At minimum, decide whether Tera Blast should be excluded. Add tests for every explicit exclusion.

### 7. Five Poké Balls become five copies of one randomized item

**Severity:** Medium
**Type:** Design and reward quantity policy
**Status:** Resolved. The Birch Lab catching tutorial again grants the authored
five Poké Balls. This preserves its dialogue and avoids multiplying a randomized
held or evolution item by five.

The two Birch's Lab scripts use `giveitem_randomized ITEM_POKE_BALL, 5`. This deterministically grants five copies of one replacement item.

The six-copy Seashore House challenge reward was explicitly accepted, but the five-Poké-Ball reward has not received a final decision. Five copies of some held or evolution items may be excessive, while replacing the capture-tool introduction also changes early-game messaging and resource expectations.

**Options for later discussion:**

1. Keep five copies of the same randomized item.
2. Grant one randomized reward.
3. Restore five Poké Balls as an authored progression/resource grant.
4. Create a special early-game reward policy.

### 8. Manual integration coverage is incomplete

**Severity:** Medium
**Type:** Validation gap
**Status:** Open; carried forward from the original review

Unit tests cover deterministic functions but do not validate complete event transactions, map reachability, save/reload behavior, or full-Bag paths. Before committing, manually validate at least:

- HM acquisition and use, especially Dive after issue 1 is fixed.
- Permanent low-tide Shoal Cave navigation and access to TM07.
- The revised Shoal Cave old-man dialogue and removed Shell Bell exchange.
- Shoal Salt and Shoal Shell exclusion from randomized reward pools.
- The Route 117 exterior Daycare block.
- Scanner exchange choice among retained evolution methods.
- Restored authored exchanges, fossils, bikes, Berry Powder purchases, and E-Reader Enigma Berry delivery.
- One-time Route 120, Kiri, and Berry Master's wife rewards, including full-Bag retries.
- Visible and hidden randomized item identity, persistence, and local object ID handling.
- Both deterministic randomized vendors, including full-Bag behavior.
- Mirage Tower's permanent appearance and authored fossil choice.
- Oldale's 200 Ultra Balls and ¥200,000 starting resources.
- Randomized TM name/description/move consistency.
- Save and reload with the 64-slot Items pocket.

Use `AgentDocs/manual-validation-checklist.md` as the detailed checklist and update it when fixes add new cases.

### 9. Zero-flag hidden-item entries are not truly inert

**Severity:** Low
**Type:** Technical debt and future integration risk
**Status:** Resolved. Zero-flag placeholders now point to an explicit no-op event
script rather than a null script, so interaction cannot fall through to the test
signpost message.

The build workaround in `asm/macros/map.inc` emits zero-flag hidden-item placeholders as background events with a null script and describes them as inert. `GetInteractedBackgroundEventScript` in `src/field_control_avatar.c` converts a null background-event script into `EventScript_TestSignpostMsg`.

The affected events reportedly belong to unavailable cross-version maps, so they are probably unreachable in the current Emerald build. However, the representation is not actually inert and could expose the test-signpost message if those maps become reachable.

**Recommended resolution:** Prefer omitting these events during generation, or point them to a true no-op event if the map format requires an entry. Update the comment to reflect the actual behavior until resolved.

### 10. Randomizer algorithm version documentation is stale

**Severity:** Low
**Type:** Documentation
**Status:** Resolved. The implementation handoff records algorithm version 4 and
the mapping changes that required versions 3 and 4.

At review time, `include/config/randomizer.h` defined
`RANDOMIZER_ALGORITHM_VERSION` as 4 while the implementation handoff still said
version 3. The handoff now records version 4 and its mapping changes.

**Recommended resolution:** Update the implementation document to version 4 and briefly record which mapping changes required the bump.

### 11. Six scripts contain only end-of-file newline churn

**Severity:** Low
**Type:** Changelist hygiene
**Status:** Resolved. The six final newlines were restored and the files no
longer appear in the pending diff.

The following files have no semantic changes and only remove the final newline:

- `data/maps/BattleFrontier_ScottsHouse/scripts.inc`
- `data/maps/DesertUnderpass/scripts.inc`
- `data/maps/FallarborTown_CozmosHouse/scripts.inc`
- `data/maps/MauvilleCity_BikeShop/scripts.inc`
- `data/maps/MauvilleCity_House2/scripts.inc`
- `data/maps/MirageTower_4F/scripts.inc`

**Recommended resolution:** Restore their original final newlines or otherwise remove them from the pending diff before committing.

## Previously reviewed design decisions

These are not currently classified as defects, but should remain visible while completing manual validation:

- Shoal Cave is permanently low tide.
- The Shell Bell exchange system has been removed.
- Reachable Shoal Salt spots are one-time randomized pickups; Shoal Shell is no longer collectible under permanent low tide.
- The Shoal Cave old man comments that the cave is always at low tide.
- The Daycare is blocked from the exterior; an interior redesign is not required.
- EV gain is disabled both from battles and EV items; EV items are not protected from item randomization.
- Oldale grants 200 Ultra Balls and ¥200,000.
- The Scanner trade lets the player choose among the retained evolution stones and Linking Cord.
- Prof. Cozmo still requires the Meteorite and gives the authored TM27.
- The Harbor Mail trade and Coin Case logic are restored. Removing the Coin Case remains a future streamlining task.
- Bike Shop grants are restored.
- Berry Powder purchases are restored. Removing the Berry Powder minigame remains a future streamlining task.
- Mt. Chimney and Seashore House vendors intentionally retain deterministic randomized stock at their authored prices.
- The Seashore House challenge retains six copies of the same randomized item. Disabling the challenge fights, reward, and vendor is a future streamlining option.
- Trick House retains its current mixed authored/randomized behavior.
- Battle Frontier symbol prizes are restored, though they are postgame and outside the main Nuzlocke run.
- Route 120, Kiri, and Berry Master's wife are redesigned as one-time rewards.
- Size judging remains enabled with its current reward behavior after reversing the earlier disable decision.
- Contest scarves are restored; disabling Contests is a future streamlining consideration.
- Mirage Tower always appears and currently uses authored Root/Claw Fossils. A future design may select from fossils across generations.
- Desert Underpass restores the remaining authored fossil, despite being postgame.
- E-Reader Enigma Berry delivery is restored. E-Reader-only items, events, and unlocks such as Deoxys need a future accessibility review.
- All vendors should receive a future case-by-case review before expanding vendor randomization.

## Final automated validation snapshot

Validation performed after resolving the review queue:

- Production ROM build: passed.
- `git diff --check`: passed.
- Randomizer tests: 60/60 passed, including authored-HM coverage and direct HM08
  Dive resolution/reverse lookup.
- Bag tests: 4/4 passed, including Kiri's full-existing-stack regression.
- Save-layout tests: 4/4 passed.
- EV and level-cap tests: 4/4 passed.
- Trainer-control tests: 22/22 passed after classifying global randomized
  abilities and updating the algorithm-version-4 deterministic mapping.
- Production EWRAM usage: approximately 244,268 bytes, or 93.18%.

Emulator-backed filters must run serially because concurrent runs share temporary
artifacts. The final serial runs passed.

## Remaining release work

1. Complete the gameplay checks in `AgentDocs/manual-validation-checklist.md`.
2. Record any defects found by manual validation in a new dated review note;
   preserve this file as the closed 1.1.0 implementation review.
3. Merge into `romhack/main` only with explicit user approval.
