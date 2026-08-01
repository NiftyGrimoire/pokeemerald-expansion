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
- Milcery keeps its held Sweet as the decoration choice while the save selects
  one of nine cream flavors; any valid spin can trigger that selected result.
- Location-based level-up routes are removed in favor of their existing
  location-free evolution stones.

No other evolution mechanic is globally broadened.

## Determinism contract

Alternate results use `RANDOMIZER_CATEGORY_EVOLUTION` with the saved seed,
source species, and the two candidate targets. The selected target is stable
within a save and does not consume either mutable RNG stream. A different save
can select the other target.

Only the listed pair is filtered. Unrelated evolutions from the same source
remain available. For example, Eevee's stone evolutions and its Leafeon,
Glaceon, and Sylveon routes are not removed by the Espeon/Umbreon selection.

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
| Eevee | Sylveon | Must know a Fairy-type move |
| Eevee | Espeon or Umbreon | One target selected per save; clock restriction removed |
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
| Happiny | Chansey | Oval Stone requirement and supported item/level paths |
| Eevee | Espeon or Umbreon | Level 30 replacement for friendship; deterministic pair |
| Gligar | Gliscor | Razor Fang requirement and supported item/level paths |
| Sneasel | Weavile | Razor Claw requirement and supported item/level paths |
| Hisuian Sneasel | Sneasler | Razor Claw requirement and supported item/level paths |
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
| Ursaring | Ursaluna | Peat Block retained; Hisui and night checks removed |
| Milcery | Selected Alcremie flavor | Held Sweet retained; clock and precise spin requirement removed |

## Regional restrictions removed and randomized

Each listed source selects one of its two forms per save. The authored level,
item, move, and other non-region requirements remain intact.

| Source | Deterministic targets | Trigger retained |
| --- | --- | --- |
| Pikachu | Raichu / Alolan Raichu | Thunder Stone |
| Exeggcute | Exeggutor / Alolan Exeggutor | Leaf Stone |
| Cubone | Marowak / Alolan Marowak | Level 28 |
| Koffing | Weezing / Galarian Weezing | Level 35 |
| Mime Jr. | Mr. Mime / Galarian Mr. Mime | Must know Mimic |
| Quilava | Typhlosion / Hisuian Typhlosion | Level 36 |
| Dewott | Samurott / Hisuian Samurott | Level 36 |
| Petilil | Lilligant / Hisuian Lilligant | Sun Stone |
| Rufflet | Braviary / Hisuian Braviary | Level 54 |
| Goomy | Sliggoo / Hisuian Sliggoo | Level 40 |
| Bergmite | Avalugg / Hisuian Avalugg | Level 37 |
| Dartrix | Decidueye / Hisuian Decidueye | Authored levels 34 / 36 |
| Ursaring | Ursaluna | Peat Block; region check removed |

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

## Explicitly unchanged evolution behavior

- Evolution stones, held items, known-move requirements, gender checks, stat
  comparisons, weather, party composition, battle trackers, trades, and script
  triggers remain authored unless a row above says otherwise. The six former
  location evolutions explicitly use their listed stones.
- Eevee's Jolteon, Vaporeon, Flareon, Leafeon, Glaceon, and Sylveon routes remain
  available. Only Espeon versus Umbreon is a deterministic pair.
- Own Tempo Rockruff still evolves only into Dusk Lycanroc; only its evening
  restriction is removed.
- Milcery's held Sweet still determines Strawberry, Berry, Love, Star, Clover,
  Flower, or Ribbon decoration. The save seed selects one common cream flavor
  from Vanilla, Ruby, Matcha, Mint, Lemon, Salted, Ruby Swirl, Caramel Swirl,
  or Rainbow Swirl. Any valid spin action triggers the selected Sweet/flavor
  result without checking time, direction, or spin duration.

## Validation

Focused tests cover:

- Representative level 20, 30, and 40 friendship tiers.
- A zero-friendship Pichu failing at level 19 and evolving at level 20.
- Explicit clock/region bypass allowlists and unchanged fallbacks.
- Exactly one selected target in a declared pair.
- Stability within a save and variation across seeds.
- A Rockruff evolving into its selected form without consulting the clock.
- Exactly one Milcery cream flavor being shared across all seven Sweets.
- Milcery evolving to the selected held-Sweet result from an arbitrary spin.
- All six former location targets retaining their item route and having no
  remaining level-up route.

Manual gameplay checks still required:

- Exercise at least one evolution from each friendship tier.
- Exercise held-item clock conversions for Gligar and both Sneasel forms.
- Exercise a stone-based regional pair and a level-based regional pair across
  multiple saves.
- Verify Eevee's Fairy-move precedence and selected Espeon/Umbreon fallback.
- Verify cancellation and retry do not change the selected target.
- Exercise at least two different Sweets on Milcery and confirm that decoration
  follows the held item while cream flavor stays fixed within the save.
- Use each of the Thunder, Leaf, and Ice Stone replacement routes outside the
  former required location.
