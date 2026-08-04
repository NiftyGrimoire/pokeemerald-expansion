# Gameplay Randomizer Manual Validation Checklist

## Purpose

Use this checklist to validate the integrated randomizer and quality-of-life
features in an emulator or on hardware. Automated tests remain the primary gate
for deterministic logic; these checks cover presentation, complete gameplay
flows, performance, event flags, and interactions that are difficult to exercise
in unit tests.

Record the ROM commit, emulator or hardware, save seed, and result for every test
session. Do not treat a feature as manually validated from source review alone.

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
- [ ] Determine whether the included FRLG static maps are reachable. If they are,
      add and run coverage for Mewtwo and the Kanto birds; otherwise record `[-]`.

## 4. Level caps and zero EVs

- [ ] Confirm battle EXP stops exactly at the active cap.
- [ ] Confirm a Pokemon below the cap gains ordinary EXP and levels normally.
- [ ] Confirm Rare Candy cannot raise a Pokemon above the cap.
- [ ] Confirm every enabled EXP Candy cannot raise a Pokemon above the cap.
- [ ] Confirm cap progression after each badge: 15, 19, 24, 29, 31, 33, 42, 46,
      and 58.
- [ ] Confirm the post-Champion fallback cap is 100.
- [ ] Starting from a zero-EV Pokemon, confirm battle participation produces no
      positive EVs.
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

- [ ] Confirm a newly created low-level Pokemon receives four deterministic
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

- [ ] Inspect Blazing Torque, Combat Torque, Magical Torque, Noxious Torque, and
      Wicked Torque wherever their descriptions are displayed.
- [ ] Record every remaining placeholder description for the pending Torque
      description cleanup.

## 11. Trainer AI review and future validation

Run this section after the AI tier policy is implemented.

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

Run these sections after their policies and implementations are complete.

### World items

- [ ] Confirm visible pickups are deterministic within a save and differ across
      seeds.
- [ ] Confirm key items, TMs, duplicates, respawns, and progression-critical items
      follow the approved policy.
- [ ] Confirm every replacement belongs to the approved Nuzlocke-useful pool.
- [ ] Confirm retired evolution items, redundant medicine, and disallowed X-items
      do not appear.
- [ ] Confirm required evolution resources are obtainable before they are needed.
- [ ] Confirm any new pickup spots are visible and optional rather than hidden.

### Streamlined progression

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
