# Trainer AI Investigation

## Scope

This document audits the AI behavior available in the current
pokeemerald-expansion base and proposes fair trainer tiers for randomized enemy
species and learnsets. It covers move selection, voluntary mid-battle switching,
post-KO replacement selection, knowledge, prediction, and special-purpose flags.

Implementation status: complete. Commit `5193a8111d` added the ordinary and boss trainer tiers; follow-up commits `81e473f853` and `d932605110` cover randomized-ability awareness and absent-trainer handling. Focused automated coverage exists in `test/trainer_ai.c`; broader battle-scenario validation remains separate follow-up work.

## Current trainer data

Trainer AI is authored in `src/data/trainers.party` and compiled into the
generated `src/data/trainers.h`. The generated file must not be edited directly.

Current authored flag combinations across the generated trainer tables are:

| Entries | Authored flags |
| ---: | --- |
| 640 | Check Bad Move |
| 173 | Basic Trainer |
| 13 | Check Bad Move, Try To Faint, Force Setup First Turn |
| 7 | Check Bad Move, Try To Faint |
| 5 | Basic Trainer, Risky |
| 1 | Basic Trainer, Force Setup First Turn |

`AI_FLAG_BASIC_TRAINER` expands to Check Bad Move, Try To Faint, and Check
Viability. Most ordinary trainers therefore use only the weakest failure-avoidance
logic, while Gym Leaders, the Elite Four, the Champion, Steven, rivals, and Gym
rematches generally use Basic Trainer. Winona additionally uses Risky, and Sidney
uses Force Setup First Turn.

The existing data does not assign Smart Switching, Smart Mon Choices,
Omniscient, prediction, restricted knowledge, randomized switch-ins, or smart
Tera behavior to normal Emerald story trainers.

Facility, e-Reader, Trainer Hill, and Secret Base battles are handled through a
separate basic-flag fallback in `GetAiFlags`; the ordinary trainer tier work
should not silently retier those excluded systems.

## Baseline move-selection modes

### Check Bad Move

Avoids moves that fail or are clearly ineffective because of immunities,
invulnerability, terrain, abilities, and similar current-state constraints. It
does not by itself compare all usable moves well.

### Try To Faint

Prioritizes an available KO and accounts for priority when the AI is slower and
at risk of being KOed. This is the minimum offensive competence expected from a
trainer intended to challenge the player.

### Check Viability

Compares damaging moves and scores move effects in context. Together with Check
Bad Move and Try To Faint, this forms `AI_FLAG_BASIC_TRAINER` and is the best
general-purpose move-selection baseline.

### Additional move preferences

- `AI_FLAG_HP_AWARE`: avoids inappropriate setup, healing, status, or
  self-sacrifice choices at unsuitable HP thresholds.
- `AI_FLAG_TRY_TO_2HKO`: rewards reliable one- and two-hit KOs, but is weaker than
  Try To Faint for immediate KOs.
- `AI_FLAG_PREFER_HIGHEST_DAMAGE_MOVE`: biases toward maximum raw damage without
  considering accuracy or secondary value as strongly.
- `AI_FLAG_POWERFUL_STATUS`: prioritizes field and side setup even when damage is
  available.
- `AI_FLAG_PREFER_STATUS_MOVES`: broadly rewards status moves and should always be
  paired with Check Bad Move.
- `AI_FLAG_FORCE_SETUP_FIRST_TURN`: applies a blunt first-turn setup bonus. With
  randomized learnsets it may produce arbitrary or poor behavior and should not
  be used as a generic boss upgrade.
- `AI_FLAG_RISKY`: favors high rolls, damage over accuracy, low-probability strong
  effects, recoil or self-sacrifice, and more offensive switching.
- `AI_FLAG_CONSERVATIVE`: assumes low damage rolls.
- `AI_FLAG_WILL_SUICIDE`, Stall, and Prefer Baton Pass are strategy-specific and
  should be used only when a trainer is deliberately built around that strategy.

Because trainer moves are randomized, authored strategy flags can lose their
original meaning. General scoring flags remain useful, but specialist flags must
be audited or removed when the generated team no longer supports the strategy.

## Switching modes

### Default switching

Without Smart Switching, the AI retains a set of older emergency switch checks
but does not perform the complete player-like matchup and survival analysis.
Without Smart Mon Choices, post-KO and forced replacement selection uses the
vanilla candidate selector.

### Smart Mon Choices

`AI_FLAG_SMART_MON_CHOICES` improves which Pokemon is sent in after a KO or
switch. In singles it prioritizes, as applicable:

1. A trapper that wins the matchup.
2. A revenge killer after a KO.
3. A favorable type matchup with a super-effective move.
4. A favorable type matchup without a super-effective move.
5. A sufficiently durable candidate or Baton Pass user.
6. The highest-damage fallback.

This flag improves post-KO replacement choices without requiring the AI to make
voluntary mid-battle switches. It is therefore the most useful isolated upgrade
for ordinary randomized trainers under globally locked Set mode.

Smart Mon Choices currently falls back to vanilla replacement logic in double
battles.

### Smart Switching

`AI_FLAG_SMART_SWITCHING` controls when the AI voluntarily switches and
automatically enables Smart Mon Choices. It adds checks for bad matchups, losing
the one-on-one, ineffective moves, harmful status, Encore or Choice lock, lowered
attacking stats, Regenerator or Natural Cure value, absorption opportunities,
trappers, and hazard survival.

Many checks have intentional failure chances so switching is not perfectly
predictable. This flag is appropriate for bosses and advanced rematches, but may
make ordinary trainers switch too frequently or lengthen routine fights.

### Sequence Switching

`AI_FLAG_SEQUENCE_SWITCHING` sends replacements in authored party order and
prevents voluntary mid-battle switching. It also controls U-turn-style choices.
This is deterministic but conflicts with the goal of intelligent randomized-team
replacement selection.

### Randomize Switch-in

`AI_FLAG_RANDOMIZE_SWITCHIN` randomly chooses among candidates that meet the same
selection threshold instead of consistently taking the last matching party slot.
It prevents party-order bias while retaining the candidate quality rules. It is
most valuable alongside Smart Mon Choices.

### Ace constraints

Ace Pokemon and Double Ace Pokemon reserve the final one or two party slots until
other candidates are exhausted. Randomized trainer parties preserve configured
slot order, but species randomization means the authored ace may no longer be the
most narratively or mechanically appropriate ace. Do not add these flags globally
without an explicit randomized-ace policy.

### Dynamic switching

Scripts can install a custom dynamic switching function. This is useful for a
specific designed encounter but is not appropriate as the baseline tier system.

## Knowledge and fairness modes

### Omniscient

`AI_FLAG_OMNISCIENT` gives full knowledge of the player's moves, abilities, and
held items. `AI_FLAG_SMART_TRAINER` includes Omniscient, so using that composite
globally would silently grant perfect information. This does not match the
current fairness goal.

### Restricted assumptions

- `AI_FLAG_ASSUME_STAB`: assumes the player has same-type attacks.
- `AI_FLAG_ASSUME_STATUS_MOVES`: gives a configured chance to anticipate certain
  status and utility moves.
- `AI_FLAG_WEIGH_ABILITY_PREDICTION`: guesses possible abilities using their AI
  ratings rather than an even distribution.
- `AI_FLAG_KNOW_OPPONENT_PARTY`: knows party species and fainted status but not
  unseen moves, items, or abilities.

Randomized family abilities invalidate species-based unseen-ability guesses.
The AI therefore treats an opposing ability as unknown until an activation or
another battle event records the actual ability. Once recorded, existing AI
history makes later move choices respect Lightning Rod, Volt Absorb, and similar
immunities. The AI still knows its own ability, and restricted move assumptions
remain independent of this policy.

### Prediction

- Predict Switch estimates whether the player will switch, with an intentional
  failure rate.
- Predict Incoming Mon scores against the expected replacement and implies
  Predict Switch.
- Predict Move estimates the player's next move.

Prediction is substantially stronger with Omniscient knowledge. Without it,
prediction can still add variety but must be playtested carefully against
randomized learnsets. It should not be part of the first implementation pass.

### PP-stall prevention

`AI_FLAG_PP_STALL_PREVENTION` learns from repeated player switches into
immunities and decays the score of exploited moves. It does not require globally
omniscient knowledge and is a reasonable boss-tier safeguard after focused tests.

## Gimmick behavior

- Double Battle is added automatically when applicable.
- Smart Tera avoids unconditional immediate Terastallization, but its enhanced
  behavior is not supported in doubles.
- Mega Evolution, Z-Moves, Dynamax, and Tera retain their configured permission
  paths. AI tier changes should be tested with randomized replacement species,
  since some original gimmicks become unusable after species replacement.

## Recommended initial tier policy

### Tier 0: deliberately simple encounters

Use Check Bad Move only when an encounter is intentionally tutorial-like or weak.
Do not preserve this as the default for hundreds of ordinary trainers merely
because it is inherited from vanilla data.

### Tier 1: ordinary trainers

Recommended flags:

```c
AI_FLAG_BASIC_TRAINER
| AI_FLAG_SMART_MON_CHOICES
| AI_FLAG_RANDOMIZE_SWITCHIN
```

This gives every trainer competent move scoring and better post-KO replacements,
which matters more under globally locked Set mode, without adding voluntary
player-like switching or hidden knowledge.

### Tier 2: rivals, admins, Gym Leaders, Elite Four, and Champion

Recommended flags:

```c
AI_FLAG_BASIC_TRAINER
| AI_FLAG_SMART_SWITCHING
| AI_FLAG_RANDOMIZE_SWITCHIN
| AI_FLAG_ASSUME_STAB
| AI_FLAG_ASSUME_STATUS_MOVES
| AI_FLAG_PP_STALL_PREVENTION
```

Smart Switching automatically adds Smart Mon Choices. This tier uses plausible
assumptions instead of Omniscient knowledge and improves both voluntary switches
and post-KO replacement choices. Smart Tera is intentionally excluded because
Terastallization is not available in this game.

### Tier 3: selected late rematches or optional superbosses

Begin with Tier 2. Consider Know Opponent Party or selected prediction flags only
after playtesting. Do not add Omniscient globally; if used at all, limit it to an
explicit optional superboss policy and document it to the player.

## Implementation considerations

Two implementation strategies are available:

1. Edit AI fields throughout `src/data/trainers.party`. This is explicit but
   mechanically large and makes later retiering cumbersome.
2. Resolve a gameplay-randomizer AI tier from trainer identity in
   `GetAiFlags`, preserving special-purpose authored flags where appropriate.
   This centralizes policy but requires a maintained classification for bosses,
   rivals, rematches, facilities, partners, and exceptions.

The recommended approach is a central ordinary/boss tier resolver with an
explicit allowlist for Tier 2 and exceptions. Preserve only reviewed strategy
flags such as Risky; do not blindly OR every authored flag into randomized teams.
Keep facilities and player-controlled partners outside the first pass.

## Required tests before integration

- Ordinary tier contains Basic, Smart Mon Choices, and Randomize Switch-in but
  not Smart Switching or Omniscient.
- Boss tier contains Smart Switching and restricted assumptions but not
  Omniscient.
- Facilities and excluded trainer systems retain their existing flags.
- Post-KO selection chooses a strong randomized-team candidate in singles.
- Ordinary trainers do not voluntarily switch solely because of the tier policy.
- Bosses can make a beneficial voluntary switch.
- Sequence and ace exceptions retain their documented behavior if preserved.
- Double battles fall back safely where Smart Mon Choices is unsupported.
- Tera-related AI flags remain disabled.
- Randomized abilities do not leak impossible authored-ability knowledge.

## Manual validation focus

Use the trainer AI section of `AgentDocs/manual-validation-checklist.md`. Compare
ordinary trainers, rivals, Gym Leaders, Elite Four members, rematches, and at
least one double battle. Specifically observe post-KO replacement quality,
mid-battle switch frequency, move scoring, battle duration, and whether the AI
appears to know unseen player information.
