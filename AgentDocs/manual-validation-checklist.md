# Gameplay Randomizer Manual Validation Checklist

## Purpose

Use this checklist to validate the integrated randomizer and quality-of-life
features in an emulator or on hardware. Automated tests remain the primary gate
for deterministic logic; these checks cover presentation, complete gameplay
flows, performance, event flags, and interactions that are difficult to exercise
in unit tests.

Record the ROM commit, emulator or hardware, save seed, and result for every test
session. Do not treat a feature as manually validated from source review alone.

Agent task note: open manual checks do not block the next implementation task unless
the user explicitly asks for manual validation. Keep appending new checklist items as
code review or implementation uncovers additional behavior that should be verified
later.

Release note: this checklist is the remaining gameplay gate for commit
`4ebbfa896c` on `romhack/nuzlocke-1.1.0`. It does not authorize merging the
release branch into `romhack/main`.

## Test record

- ROM commit:
- Build date:
- Emulator and version, or hardware/flash cart:
- Save seed A:
- Save seed B:
- Tester:
- Date:
- Notes or issue links:

Result notation:

- `[ ]` Not run
- `[x]` Passed
- `[!]` Failed; record reproduction steps and an issue or follow-up
- `[-]` Not applicable; record why
- `[~]` Deferred or low priority; does not block the next implementation task

## Recommended save coverage

Maintain at least these saves or save states:

- Seed A: primary badge-to-Champion playthrough.
- Seed B: new game used to compare deterministic randomized results.
- Early game: before the first badge.
- Midgame: after Surf and before the seventh badge.
- Late game: after the eighth badge and before the Champion.
- Postgame: Champion defeated, with the level-cap fallback at 100.

When using save states, also perform a normal in-game save and reload before
marking persistence-sensitive checks complete.

## 1. Save seed and determinism

- [ ] Start a new game and confirm ordinary randomizer features function without
      initialization errors.
- [ ] Save, reset, and reload; confirm starter, species-family abilities,
      learnsets, encounters, and repeated trainer teams remain unchanged.
- [ ] Start Seed B and confirm representative starters, trainer teams, abilities,
      and learnsets differ from Seed A.
- [ ] Confirm repeated lookups and battles do not visibly advance or perturb
      unrelated randomized results.

## 2. Ordinary wild encounters

- [ ] Confirm early land encounters draw from visibly low-BST species pools.
- [ ] On one map with land and water, confirm Surf can access a stronger pool than
      the land encounter table.
- [ ] On one map with multiple rods, confirm improved rods can access stronger
      pools than the Old Rod.
- [ ] Confirm late-game encounter tables draw from high-BST pools.
- [ ] Confirm encounter method, selected-slot rarity, and generated level still
      behave normally.
- [ ] Exercise a mass outbreak.
- [ ] Exercise Feebas tiles and confirm Feebas-path encounters randomize normally.
- [ ] Trigger an encounter with Sweet Scent.
- [ ] Trigger a double wild battle and confirm both opponents are valid.
- [ ] Exercise Rock Smash encounters.
- [ ] Confirm Repel checks and encounter-influencing overworld abilities still
      behave as intended when the Repel Toggle is disabled.
- [ ] Walk through several encounter-heavy areas and confirm there is no
      perceptible delay from species-pool scanning.
- [ ] Confirm roamers and disabled DexNav or overworld-visible encounters remain
      outside the randomizer policy.

## 3. Scripted and Legendary encounters

For each encounter, confirm the replacement belongs to the permitted restricted
Legendary, sub-Legendary, or Paradox pool, and that level and battle setup remain
correct.

- [ ] Groudon: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Kyogre: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Rayquaza: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Regirock: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Regice: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Registeel: battle, capture/defeat flag, and repeat-entry behavior.
- [ ] Confirm Mythical Pokemon and Ultra Beasts do not appear in this pool.
- [ ] Exercise a non-special `setwildbattle` encounter and confirm it uses the
      ordinary level/BST-scaled pool.
- [-] FRLG static maps are outside Emerald gameplay scope; no FRLG coverage is
      required for this randomizer.

## 4. Level caps and zero EVs

- [ ] Confirm battle EXP stops exactly at the active cap.
- [ ] Confirm a Pokemon below the cap gains ordinary EXP and levels normally.
- [ ] Confirm Rare Candy cannot raise a Pokemon above the cap.
- [ ] Confirm every enabled EXP Candy cannot raise a Pokemon above the cap.
- [ ] Confirm cap progression after each badge: 15, 19, 24, 29, 31, 33, 42, 46,
      and 58.
- [ ] Confirm the post-Champion fallback cap is 100.
- [ ] Starting from a zero-EV Pokemon, confirm battle participation produces no
      positive EVs, including ordinary battles, Exp. Share recipients, and a
      level-100 Pokemon receiving battle rewards.
- [ ] Confirm vitamins, feathers, and EV-raising items cannot change EVs while
      EV-reducing items retain their documented behavior.
- [ ] Confirm vitamins cannot produce positive EVs.
- [ ] Confirm feathers cannot produce positive EVs.
- [ ] Confirm EV-affecting berries cannot produce positive EVs or underflow EVs.

## 5. Starters

- [ ] Confirm all three starter choices are distinct.
- [ ] Confirm each choice is an enabled base-stage Pokemon with the intended BST
      and complete three-stage-line policy.
- [ ] For all three slots, confirm label, sprite, cry, and confirmation text agree.
- [ ] Confirm the selected Pokemon is the Pokemon granted to the party.
- [ ] Confirm starter-dependent party checks recognize the granted Pokemon.
- [ ] Confirm credits display the selected randomized starter correctly.
- [ ] Compare Seed A and Seed B and confirm the choices vary.

## 6. Enemy trainer parties

- [ ] Repeat an ordinary trainer encounter and confirm its team is stable within
      one save.
- [ ] Fight the same trainer on Seed A and Seed B and confirm its team changes.
- [ ] Exercise May or Brendan at multiple story encounters.
- [ ] Exercise a Gym Leader.
- [ ] Exercise an override trainer.
- [ ] Exercise a trainer-pool encounter.
- [ ] Confirm party size and levels match the authored encounter.
- [ ] Confirm replacement species remain near the intended original BST.
- [ ] Confirm generated initial moves are legal and usable for replacements.
- [ ] Confirm held items persist and incompatible species-specific effects fail
      safely rather than transforming the wrong species.
- [ ] Exercise held Mega and Z items on replacement species.
- [ ] Exercise Dynamax permission and level settings.
- [ ] Exercise an originally Gigantamax-configured party entry.
- [ ] Exercise configured Tera settings.
- [ ] Confirm player-controlled trainer parties are unchanged.
- [ ] Confirm Secret Base, Battle Frontier, Trainer Hill, and e-Reader parties are
      unchanged.

## 7. Randomized abilities and forms

- [ ] On Seed A, confirm one ordinary evolution family retains one ability before
      and after evolution.
- [ ] Confirm the same family receives a different ability on at least one other
      seed.
- [ ] Confirm summary, party, Storage, Pokedex, and battle displays agree.
- [ ] Exercise representative overworld abilities.
- [ ] Confirm an ordinary randomized trainer Pokemon uses its displayed ability.
- [ ] Test Castform weather forms.
- [ ] Test Cherrim.
- [ ] Test Aegislash.
- [ ] Test Wishiwashi.
- [ ] Test Minior.
- [ ] Test Mimikyu.
- [ ] Test Cramorant.
- [ ] Test Eiscue.
- [ ] Test Morpeko.
- [ ] Test Zygarde.
- [ ] Test Silvally.
- [ ] Test Arceus.
- [ ] Test Terapagos.

For every form test, confirm the family is either deliberately protected or
changes form correctly without losing data, crashing, or becoming stuck.

## 8. Learnsets and teachable moves

- [ ] Confirm a newly created low-level Pokemon receives two deterministic
      starting moves.
- [ ] Confirm an ordinary level-up move is offered at the expected level.
- [ ] Confirm an evolution move is offered during evolution.
- [ ] Confirm Move Reminder output matches the randomized learnset.
- [ ] Confirm Pokedex learnset output matches gameplay.
- [ ] Confirm wild Pokemon initial moves match their species and level.
- [ ] Confirm randomized trainer Pokemon initial moves match their species and
      level.
- [ ] Confirm gift Pokemon initial moves match their species and level.
- [ ] Confirm every enabled real species can learn representative configured TMs.
- [ ] Confirm authored HM compatibility remains intact.
- [ ] Confirm all 50 TM slots teach distinct moves for a save and that the
      mapping changes across seeds.
- [ ] Confirm TM descriptions, bag use, compatibility, Move Reminder, Pokedex,
      reverse move-to-TM lookups, and debug helpers all use the same mapping.
- [ ] Confirm HM descriptions, field use, compatibility, and move mappings remain
      authored after TM randomization.
- [ ] Confirm authored tutor compatibility remains intact.
- [ ] Confirm egg moves remain authored.
- [ ] Across Seeds A and B, review representative early-, mid-, and late-game
      species for useful move variety.
- [ ] Confirm early moves trend toward appropriate power and basic status effects.
- [ ] Confirm midgame moves introduce strong status and higher-power attacks.
- [ ] Confirm late moves can include elite utility and high-power attacks.
- [ ] Confirm no duplicate move appears in one randomized level-up table.
- [ ] Confirm excluded or species-locked moves do not appear.
- [ ] If any move lookup or encounter visibly stalls, record the species, level,
      seed, call path, and measured delay for profiling. Otherwise mark this item
      passed without adding a broader cache.

## 9. Evolution rules

- [ ] Exercise one former friendship evolution from the level-20 tier.
- [ ] Exercise one former friendship evolution from the level-30 tier.
- [ ] Exercise one former friendship evolution from the level-40 tier.
- [ ] Evolve Happiny with its replacement stone.
- [ ] Evolve Gligar with its replacement stone.
- [ ] Evolve Johto Sneasel with its replacement stone.
- [ ] Evolve Hisuian Sneasel with its replacement stone.
- [ ] Exercise all three stone-based Applin branches.
- [ ] Evolve Dipplin into Hydrapple at level 40.
- [ ] Exercise a stone-based regional pair on Seeds A and B.
- [ ] Exercise a level-based regional pair on Seeds A and B.
- [ ] Confirm Eevee selects exactly one of Espeon, Umbreon, or Sylveon at level 30.
- [ ] Confirm Eevee's five stone branches remain player-controlled.
- [ ] Cancel a seeded evolution and retry; confirm the target does not change.
- [ ] Use at least two different stones on Milcery and confirm the decoration
      follows the stone.
- [ ] Confirm Milcery's cream flavor remains fixed within the save.
- [ ] Use the Thunder Stone replacement route outside New Mauville.
- [ ] Use the Leaf Stone replacement route outside Petalburg Woods.
- [ ] Use the Ice Stone replacement route outside Shoal Cave.
- [ ] Exercise one plain trade replacement using the Linking Cord.
- [ ] Exercise one former held-item trade replacement.
- [ ] Exercise Karrablast and Shelmet replacement routes.
- [ ] Exercise at least two Pumpkaboo sizes.
- [ ] Evolve Primeape into Annihilape at level 40 without Rage Fist.
- [ ] Exercise one replacement each from the former party/weather,
      battle-counter, walking, and unavailable-item/script groups.
- [ ] Confirm male and female White-Striped Basculin still evolve into their
      matching Basculegion forms at level 40.
- [ ] Confirm Nincada still requires a Poké Ball and an open party slot to create
      Shedinja when it evolves.
- [ ] Raise a boxed Nincada through level 20 with a regular Poké Ball and an open
      party slot; confirm boxed Ninjask replaces it, party Shedinja is created,
      and one Poké Ball is consumed. Repeat with a full party and without a Poké
      Ball; confirm Ninjask still evolves but Shedinja is not created.
- [ ] Open Captain Stern's Scanner exchange and confirm the scrollable menu lists
      all ten retained evolution stones, the Linking Cord, and Exit.
- [ ] Select and confirm both a representative stone and the Linking Cord on
      separate saves; confirm the selected item is received unchanged and the
      Scanner is consumed exactly once.
- [ ] Back out of the Scanner menu, decline its confirmation, and attempt the
      exchange with a full applicable Bag pocket; confirm each path preserves
      the Scanner and permits a later retry.

## 10. Quality-of-life features

### Global Set battle style

- [ ] Confirm Battle Style is absent from the Options menu and remaining rows are
      aligned and selectable.
- [ ] On a new save, defeat an opposing trainer Pokemon and confirm its
      replacement enters without offering a free switch.
- [ ] Repeat on an older save whose stored option was Shift and confirm Set flow.
- [ ] Confirm ordinary voluntary switching remains available during battle.
- [ ] Confirm wild-battle faint and escape prompts remain correct.

### Level to cap

- [ ] Raise a party Pokemon multiple levels to the active cap.
- [ ] Raise a boxed Pokemon multiple levels without moving it into the party.
- [ ] Confirm every intermediate level is processed in order.
- [ ] Confirm stat pages show each level's gains and resulting stats.
- [ ] Confirm every eligible move-learning prompt appears.
- [ ] Accept and decline move replacement, then confirm persistence.
- [ ] Confirm every eligible evolution scene runs.
- [ ] Cancel an evolution and confirm the larger operation stops at that point.
- [ ] Save and reload after party and Storage operations; confirm all completed
      levels, moves, and evolutions persist.
- [ ] Confirm a Pokemon already at the cap cannot advance.

### Numeric IV summary

- [ ] On the Skills page, press A and confirm all six stored numeric IVs appear.
- [ ] Press A again and confirm the normal stats view returns.
- [ ] Confirm the label changes between STATS and IVs.
- [ ] Confirm Hyper Training does not replace the displayed stored IV with 31.
- [ ] Confirm viewing IVs does not mutate Pokemon data.

### Portable Healer

- [ ] Confirm the item is granted with the starter.
- [ ] Outside battle, heal a party containing HP loss, status, and reduced PP.
- [ ] Confirm every party member is fully restored.
- [ ] Confirm the item is not consumed.
- [ ] Confirm it cannot be used to disrupt an active battle.

### Repel Toggle

- [ ] Confirm the item is granted with the starter.
- [ ] Enable it and confirm standard random encounters stop regardless of lead
      level.
- [ ] Save and reload; confirm the enabled state persists.
- [ ] Confirm scripted and deliberately triggered encounters remain available.
- [ ] Disable it and confirm standard encounters resume.
- [ ] Exercise an ordinary Repel and a Lure; confirm their shared-state behavior
      matches the documented policy.

### Badge-authorized HM actions

- [ ] Before each required badge, confirm the corresponding field action remains
      locked.
- [ ] After each badge, confirm the action works with a non-Egg party Pokemon that
      does not know the HM.
- [ ] Confirm direct overworld interactions and party-menu actions agree.
- [ ] Confirm learned non-HM field moves retain their original requirements.
- [ ] Confirm an all-Egg or otherwise invalid party cannot provide an HM user.

### Oldale supply grant

- [ ] Confirm the Oldale NPC grants 200 Ultra Balls and ¥200,000 once.
- [ ] Fill the relevant Bag pocket, attempt the grant, make room, and confirm the
      reward can be retried.
- [ ] Confirm the completion flag is set only after the item grant succeeds.
- [ ] Leave and re-enter Oldale; confirm the reward cannot be claimed twice.

### Hidden items removed

- [ ] Interact with several known hidden-item tiles and confirm nothing is
      collected.
- [ ] Use the Itemfinder near known hidden-item tiles and confirm none are
      detected.
- [ ] Confirm ordinary visible item balls remain collectible.
- [ ] Exercise any script that checks a former hidden-item flag and record broken
      progression or dialogue for follow-up.

### Player-facing move descriptions

- [~] Low priority: inspect Blazing Torque, Combat Torque, Magical Torque,
      Noxious Torque, and Wicked Torque wherever their descriptions are displayed.
- [~] Low priority: record every remaining placeholder description for the Torque
      description cleanup.

## 11. Trainer AI implementation and future validation

Trainer AI tier assignment is implemented and has focused automated coverage.
The remaining items below are gameplay validation or broader scenario coverage.

- [ ] Confirm ordinary trainers use the approved ordinary-tier flags.
- [ ] Confirm bosses use the approved boss-tier flags.
- [ ] Confirm rematches use their approved tier.
- [ ] Test smart move scoring with good, bad, resisted, immune, setup, recovery,
      and status choices.
- [ ] Test smart mid-battle switching.
- [ ] Test post-KO replacement selection under globally locked Set flow.
- [ ] Confirm ordinary trainers do not receive unintended omniscient knowledge.
- [ ] Playtest representative early-, mid-, and late-game trainers for fairness.

## 12. World items and streamlined progression

Emerald visible item-ball pickups and eligible direct gift scripts are
implemented. Run the item checks below for manual gameplay validation and audit
the remaining excluded item sources separately.

### World items

- [ ] Confirm ordinary visible item-ball pickups are deterministic within a save
      and differ across seeds.
- [ ] Confirm duplicate useful rewards are possible and no duplicate-tracking
      state is required.
- [ ] Confirm Poke Ball variants, HMs, TMs as replacement items, and pure money
      rewards never appear in generated results.
- [ ] Confirm authored Poke Ball and money pickups convert to useful rewards.
- [ ] Confirm authored key items, HMs, TM item slots, and berry gifts remain unchanged.
- [ ] Confirm no evolution stone, Linking Cord, or other resource is guaranteed;
      record seeds where a needed resource is unavailable.
- [ ] Confirm every replacement belongs to the approved Nuzlocke-useful pool.
- [ ] Across recorded seeds, confirm the 61-result pool can produce evolution
      items, type boosters, Choice items, Focus/Band items, lenses, and general
      competitive held items, including weather, terrain, and screen extenders.
- [ ] Confirm retired evolution items, redundant medicine, disallowed X-items,
      and unapproved mixed-use sellables do not appear.
- [ ] Confirm invalid special templates remain unchanged and hidden items remain
      disabled.
- [ ] Confirm Emerald item-ball scripts and eligible NPC/story gifts use the
      deterministic mapping, while key items, HMs, TM slots, berries, shops, prize
      tables, Battle Pyramid items, and FRLG content remain unchanged or out of
      scope.
- [ ] Confirm any future pickup spots are visible and optional rather than hidden.

### Streamlined progression

- [ ] Confirm the Route 117 Day Care closure NPC blocks the entrance and explains
      that the service is closed.
- [ ] Confirm the interior Day Care cannot be reached through the normal entrance.
- [ ] Complete a badge-to-Champion playthrough while recording party level versus
      each cap.
- [ ] Record every mandatory grinding point.
- [ ] Confirm breeding is not required for progression, rewards, encounters, or
      useful moves.
- [ ] Confirm berry planting, watering, growth, and harvesting are not required.
- [ ] Confirm important berries have direct finite acquisition paths.
- [ ] Confirm EV-focused rewards, dialogue, items, and UI no longer create dead
      progression choices.
- [ ] Record forced detours, repeated conversations, and backtracking candidates.
- [ ] Confirm all streamlined plot changes preserve flags, tutorials, major
      encounters, and story progression.

## Final release pass

- [ ] Run the focused automated suites documented in the implementation handoff.
- [ ] Complete a clean production ROM build and record memory usage.
- [ ] Start a new game from the release candidate rather than relying only on old
      saves or save states.
- [ ] Complete the main Emerald badge-to-Champion route.
- [ ] Save and reload at representative early-, mid-, late-, and postgame points.
- [ ] Review all `[!]` entries and confirm each is fixed, explicitly accepted, or
      tracked before release.
- [ ] Record the final validated commit in this document or the release notes.

## Agent Implementation Backlog

These are non-manual tasks to pick up before treating validation-only items as the
next work queue.

- [x] Review trainer AI flag assignments for randomized teams and learnsets.
- [x] Define fair AI tiers for ordinary trainers, bosses, and rematches without
      giving every opponent omniscient knowledge by default.
- [ ] Optional follow-up: expand automated coverage for smart move scoring,
      mid-battle switching, and post-KO replacement selection under locked Set
      battle style where the existing test harness can support it.
- [x] Specify the first visible world-item slice: stable pickup identity;
      ordinary visible item balls only; key items and HMs remain authored; TM
      item slots remain authored for separate move randomization; invalid special
      templates remain unchanged.
- [x] Decide the world-item assignment policy: stateless pure random selection,
      duplicates allowed, no progression guarantees, no progression bands, and no
      added save state or pickup counters.
- [x] Exclude Poke Ball variants, HMs and TMs as replacement items, and pure money
      rewards; convert authored Poke Ball and money pickups into useful rewards.
- [x] Define the initial Nuzlocke-useful replacement pool: standard evolution
      stones, Linking Cord, selected evolution-held items with independent
      utility, and broadly useful held items.
- [x] Exclude retired species-specific evolution items, redundant medicine,
      X-items, mixed-use sellables, berries, and other low-value rewards from the
      initial general useful-item pool; broader distribution review remains.
- [x] Implement Emerald visible item-ball and eligible direct-gift mapping with
      protected key items, HMs, TM slots, and berries; exclude shops and prize
      tables. Focused automated coverage passes; manual gameplay validation remains.
- [x] Implement the 50-slot per-save unique TM move mapping with high-power
      and high-tier-status weighting, route every TM lookup path through it, and
      preserve authored HMs. Focused automated coverage passes; manual gameplay
      validation remains.
- [~] Low priority: audit and replace the remaining Torque move descriptions.
- [x] Block the Emerald Day Care entrance with a closure NPC and document the
      remaining breeding and dependent-reward cleanup.
- [x] Preserve the existing no-EV-gain toggle and exclude EV-effect items from the random item pool.
- [ ] Disable or replace the remaining breeding, egg, and inherited-move paths
      after auditing their progression and reward dependencies.
- [ ] When changing learnset randomizer weighting, include detrimental-move review
      cases such as `IsExplosionMove` moves, recoil-heavy attacks, fixed-damage
      edge cases, and Misty Explosion as the late-game sentinel.
