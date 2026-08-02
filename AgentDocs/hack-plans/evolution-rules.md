# Gameplay Randomizer Evolution Rules

## Purpose and status

This document is the species-level ledger for Phase 9 of the gameplay
randomizer. It records every evolution changed by
`RANDOMIZER_EVOLUTIONS`, the replacement trigger, conditions that remain
required, and deterministic alternate-form choices.

Implemented on `romhack/randomizer-evolution-rules`:

- Friendship requirements are replaced by explicit levels.
- Audited day, night, and evening restrictions are removed.
- Audited regional pairs choose one result deterministically per save.
- Paired clock branches choose one result deterministically per save.
- Milcery evolves with one of seven ordinary stones; the stone chooses its
  decoration while the save selects one of nine cream flavors.
- Location-based level-up routes are removed in favor of their existing
  location-free evolution stones.
- Trade routes are removed in favor of direct-use evolution items.
- Move and move-type requirements are replaced by explicit levels.

No other evolution mechanic is globally broadened.

For a spreadsheet-friendly, one-row-per-route summary of every change, see
[`evolution-changes.csv`](evolution-changes.csv).

## Determinism contract

Alternate results use `RANDOMIZER_CATEGORY_EVOLUTION` with the saved seed,
source species, and the two candidate targets. The selected target is stable
within a save and does not consume either mutable RNG stream. A different save
can select the other target.

Only the listed alternatives are filtered. Unrelated evolutions from the same
source remain available. Eevee's stone evolutions remain player-controlled,
while Sylveon, Espeon, and Umbreon form one seeded level-30 choice.

## Friendship replacements

The listed source species uses its replacement level instead of checking
friendship. Any other authored condition on that evolution remains required.

### Level 20: baby and early evolutions

| Source | Target | Preserved conditions |
| --- | --- | --- |
| Pichu | Pikachu | None |
| Cleffa | Clefairy | None |
| Igglybuff | Jigglypuff | None |
| Togepi | Togetic | None |
| Azurill | Marill | None |
| Budew | Roselia | Clock restriction removed |
| Chingling | Chimecho | Clock restriction removed |
| Munchlax | Snorlax | None |

### Level 30: ordinary midgame evolutions

| Source | Target | Preserved conditions |
| --- | --- | --- |
| Golbat | Crobat | None |
| Alolan Meowth | Alolan Persian | None |
| Eevee | Sylveon, Espeon, or Umbreon | One target selected per save; move and clock restrictions removed |
| Buneary | Lopunny | None |
| Riolu | Lucario | Clock restriction removed |
| Woobat | Swoobat | None |
| Swadloon | Leavanny | None |
| Snom | Frosmoth | Clock restriction removed |

### Level 40: late or exceptional evolutions

| Source | Target | Preserved conditions |
| --- | --- | --- |
| Chansey | Blissey | None |
| Type: Null | Silvally | None |

## Clock restrictions removed

These evolutions no longer require a particular time of day. Their authored
level, item, move, form, and other non-clock requirements remain intact.

| Source | Target or outcome | Trigger retained |
| --- | --- | --- |
| Alolan Rattata | Alolan Raticate | Level 20 |
| Cubone | Marowak or Alolan Marowak | Level 28; deterministic form selection |
| Happiny | Chansey | Shiny Stone; Oval Stone and clock requirements removed |
| Eevee | Espeon or Umbreon | Level 30 replacement for friendship; deterministic pair |
| Gligar | Gliscor | Moon Stone; Razor Fang and clock requirements removed |
| Sneasel | Weavile | Dusk Stone; Razor Claw and clock requirements removed |
| Hisuian Sneasel | Sneasler | Dawn Stone; Razor Claw and clock requirements removed |
| Galarian Linoone | Obstagoon | Level 35 |
| Budew | Roselia | Level 20 replacement for friendship |
| Chingling | Chimecho | Level 20 replacement for friendship |
| Riolu | Lucario | Level 30 replacement for friendship |
| Tyrunt | Tyrantrum | Level 39 |
| Amaura | Aurorus | Level 39 |
| Yungoos | Gumshoos | Level 20 |
| Rockruff | Midday or Midnight Lycanroc | Level 25; deterministic pair |
| Own Tempo Rockruff | Dusk Lycanroc | Level 25 |
| Fomantis | Lurantis | Level 34 |
| Cosmoem | Solgaleo or Lunala | Level 53; deterministic pair |
| Snom | Frosmoth | Level 30 replacement for friendship |
| Greavard | Houndstone | Level 30 |
| Ursaring | Ursaluna | Moon Stone; Peat Block, Hisui, and night checks removed |
| Milcery | Selected Alcremie flavor | Use a decoration-specific stone; Sweet, clock, and spin requirements removed |

## Regional restrictions removed and randomized

Each listed source selects one of its two forms per save. The authored level,
item, move, and other non-region requirements remain intact.

| Source | Deterministic targets | Trigger retained |
| --- | --- | --- |
| Pikachu | Raichu / Alolan Raichu | Thunder Stone |
| Exeggcute | Exeggutor / Alolan Exeggutor | Leaf Stone |
| Cubone | Marowak / Alolan Marowak | Level 28 |
| Koffing | Weezing / Galarian Weezing | Level 35 |
| Mime Jr. | Mr. Mime / Galarian Mr. Mime | Level 32 |
| Quilava | Typhlosion / Hisuian Typhlosion | Level 36 |
| Dewott | Samurott / Hisuian Samurott | Level 36 |
| Petilil | Lilligant / Hisuian Lilligant | Sun Stone |
| Rufflet | Braviary / Hisuian Braviary | Level 54 |
| Goomy | Sliggoo / Hisuian Sliggoo | Level 40 |
| Bergmite | Avalugg / Hisuian Avalugg | Level 37 |
| Dartrix | Decidueye / Hisuian Decidueye | Level 34 for both forms |
| Ursaring | Ursaluna | Moon Stone; region check removed |

## Move requirements replaced

Randomized learnsets cannot guarantee access to an authored evolution move.
Every move or move-type condition is therefore replaced by an explicit level.
Other form-selection conditions remain as noted.

| Source | Target | Former move | Replacement level | Preserved behavior |
| --- | --- | --- | ---: | --- |
| Lickitung | Lickilicky | Rollout | 30 | None |
| Tangela | Tangrowth | Ancient Power | 24 | None |
| Mime Jr. | Mr. Mime / Galarian Mr. Mime | Mimic | 32 | Seeded form selection |
| Eevee | Sylveon / Espeon / Umbreon | Fairy-type move for Sylveon | 30 | One target selected per save |
| Bonsly | Sudowoodo | Mimic | 16 | None |
| Aipom | Ambipom | Double Hit | 32 | None |
| Yanma | Yanmega | Ancient Power | 33 | None |
| Girafarig | Farigiraf | Twin Beam | 32 | None |
| Dunsparce | Dudunsparce | Hyper Drill | 32 | Personality-based segment form retained |
| Hisuian Qwilfish | Overqwil | Barb Barrage | 28 | None |
| Piloswine | Mamoswine | Ancient Power | 34 | None |
| Steenee | Tsareena | Stomp | 28 | None |
| Poipole | Naganadel | Dragon Pulse | 40 | None |
| Dipplin | Hydrapple | Dragon Cheer | 40 | None |
| Clobbopus | Grapploct | Taunt | 35 | None |

## Location restrictions removed

All six authored location-based level-up routes already had an equivalent
evolution-stone route. The duplicate location route is removed, leaving the
stone as the sole trigger everywhere in the game.

| Source | Target | Location removed | Replacement trigger |
| --- | --- | --- | --- |
| Magneton | Magnezone | New Mauville | Thunder Stone |
| Nosepass | Probopass | New Mauville | Thunder Stone |
| Charjabug | Vikavolt | New Mauville | Thunder Stone |
| Crabrawler | Crabominable | Shoal Cave ice room | Ice Stone |
| Eevee | Leafeon | Petalburg Woods | Leaf Stone |
| Eevee | Glaceon | Shoal Cave ice room | Ice Stone |

Using stones for Eevee keeps Leafeon and Glaceon under player control and
prevents an unrestricted level-up route from preempting its level-30
Sylveon/Espeon/Umbreon logic.

## Trade evolutions removed

No enabled species retains an `EVO_TRADE` method. Plain trades and the former
Karrablast/Shelmet partner trade use the Linking Cord. Former held-item trades
are distributed across ordinary evolution stones to keep those items similarly
useful. `I_USE_EVO_HELD_ITEMS_FROM_BAG` is disabled because no evolution route
uses those former held items directly from the Bag.

### Linking Cord replacements

| Source | Target |
| --- | --- |
| Kadabra | Alakazam |
| Machoke | Machamp |
| Graveler | Golem |
| Alolan Graveler | Alolan Golem |
| Haunter | Gengar |
| Boldore | Gigalith |
| Gurdurr | Conkeldurr |
| Karrablast | Escavalier |
| Shelmet | Accelgor |
| Phantump | Trevenant |
| Pumpkaboo, all four sizes | Matching-size Gourgeist |

### Former held-item trade replacements

| Source | Target | Direct-use item |
| --- | --- | --- |
| Poliwhirl | Politoed | Dawn Stone |
| Slowpoke | Slowking | Dawn Stone |
| Onix | Steelix | Dawn Stone |
| Scyther | Scizor | Dusk Stone |
| Seadra | Kingdra | Water Stone |
| Rhydon | Rhyperior | Moon Stone |
| Electabuzz | Electivire | Thunder Stone |
| Magmar | Magmortar | Fire Stone |
| Porygon | Porygon2 | Dawn Stone |
| Porygon2 | Porygon-Z | Dusk Stone |
| Feebas | Milotic | Shiny Stone |
| Dusclops | Dusknoir | Dusk Stone |
| Clamperl | Huntail | Water Stone |
| Clamperl | Gorebyss | Shiny Stone |
| Spritzee | Aromatisse | Shiny Stone |
| Swirlix | Slurpuff | Sun Stone |

### Other held-item evolution replacements

| Source | Target | Former item | Replacement stone |
| --- | --- | --- | --- |
| Happiny | Chansey | Oval Stone | Shiny Stone |
| Gligar | Gliscor | Razor Fang | Moon Stone |
| Sneasel | Weavile | Razor Claw | Dusk Stone |
| Hisuian Sneasel | Sneasler | Razor Claw | Dawn Stone |

### Apple evolution replacements

| Source | Target | Former item | Replacement stone |
| --- | --- | --- | --- |
| Applin | Flapple | Tart Apple | Fire Stone |
| Applin | Appletun | Sweet Apple | Sun Stone |
| Applin | Dipplin | Syrupy Apple | Leaf Stone |

All three Applin branches remain directly player-controlled. Dipplin then
evolves into Hydrapple at level 40 as documented above.

### Remaining special-item replacements

| Source | Target | Former item | Replacement stone |
| --- | --- | --- | --- |
| Scyther | Kleavor | Black Augurite | Leaf Stone |
| Galarian Slowpoke | Galarian Slowbro | Galarica Cuff | Ice Stone |
| Galarian Slowpoke | Galarian Slowking | Galarica Wreath | Shiny Stone |
| Phony Sinistea | Phony Polteageist | Cracked Pot | Dawn Stone |
| Antique Sinistea | Antique Polteageist | Chipped Pot | Dawn Stone |
| Ursaring | Ursaluna | Peat Block | Moon Stone |
| Kubfu | Single Strike Urshifu | Scroll of Darkness | Dusk Stone |
| Kubfu | Rapid Strike Urshifu | Scroll of Waters | Water Stone |
| Charcadet | Armarouge | Auspicious Armor | Sun Stone |
| Charcadet | Ceruledge | Malicious Armor | Dusk Stone |
| Duraludon | Archaludon | Metal Alloy | Thunder Stone |
| Counterfeit Poltchageist | Unremarkable Sinistcha | Unremarkable Teacup | Ice Stone |
| Artisan Poltchageist | Masterpiece Sinistcha | Masterpiece Teacup | Ice Stone |

### Alcremie decoration stones

| Decoration | Replacement stone |
| --- | --- |
| Strawberry | Fire Stone |
| Berry | Water Stone |
| Love | Shiny Stone |
| Star | Thunder Stone |
| Clover | Leaf Stone |
| Flower | Sun Stone |
| Ribbon | Dawn Stone |

The selected stone controls Alcremie's decoration. The save seed still selects
one common cream flavor across all seven decorations.

## Explicitly unchanged evolution behavior

- Evolution stones, held items, known-move requirements, gender checks, stat
  comparisons, weather, party composition, battle trackers, and script
  triggers remain authored unless a row above says otherwise. The six former
  location evolutions explicitly use their listed stones.
- No trade trigger remains. Former held-item trades use the stones listed above,
  and plain or partner-specific trades use the Linking Cord.
- Eevee's Jolteon, Vaporeon, Flareon, Leafeon, and Glaceon routes remain
  player-controlled. Sylveon, Espeon, and Umbreon form one deterministic
  level-30 choice.
- Own Tempo Rockruff still evolves only into Dusk Lycanroc; only its evening
  restriction is removed.
- Using the listed stone on Milcery determines its Strawberry, Berry, Love,
  Star, Clover, Flower, or Ribbon decoration. The save seed selects one common
  cream flavor from Vanilla, Ruby, Matcha, Mint, Lemon, Salted, Ruby Swirl,
  Caramel Swirl, or Rainbow Swirl. No Sweet, spin, or clock condition remains.

## Validation

Focused tests cover:

- Representative level 20, 30, and 40 friendship tiers.
- A zero-friendship Pichu failing at level 19 and evolving at level 20.
- Explicit clock/region bypass allowlists and unchanged fallbacks.
- Exactly one selected target in a declared pair.
- Stability within a save and variation across seeds.
- A Rockruff evolving into its selected form without consulting the clock.
- Exactly one Milcery cream flavor being shared across all seven Sweets.
- Milcery evolving to the selected result with each decoration stone.
- All six former location targets retaining their item route and having no
  remaining level-up route.
- Every enabled evolution table containing no trade method, with representative
  Linking Cord and direct-use item routes retained.
- Every enabled evolution table containing no move or move-type condition.

Manual gameplay checks still required:

- Exercise at least one evolution from each friendship tier.
- Exercise the replacement stones for Happiny, Gligar, and both Sneasel forms.
- Exercise all three stone-based Applin branches and Dipplin's level-40 follow-up.
- Exercise a stone-based regional pair and a level-based regional pair across
  multiple saves.
- Verify Eevee's Fairy-move precedence and selected Espeon/Umbreon fallback.
- Verify cancellation and retry do not change the selected target.
- Exercise at least two different stones on Milcery and confirm that decoration
  follows the used stone while cream flavor stays fixed within the save.
- Use each of the Thunder, Leaf, and Ice Stone replacement routes outside the
  former required location.
- Exercise one plain trade replacement, one former held-item trade replacement,
  both Karrablast and Shelmet, and at least two Pumpkaboo sizes.
