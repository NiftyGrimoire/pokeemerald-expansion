# Pokémon Emerald Randomizer — Evolution Guide

This is the player-facing reference for every evolution changed by this ROM
hack. If an evolution is not listed here, use its normal requirement from the
base game or `pokeemerald-expansion`.

The hack removes requirements that are unreliable or inconvenient in a short
randomized run: trading, friendship grinding, specific moves, time windows,
regions, evolution locations, unavailable currencies or scripts, and lengthy
battle or walking counters. Conditions not listed in this guide remain unchanged.

## How deterministic branches work

Some Pokémon can evolve into regional, clock-based, or otherwise alternate
forms. Where the original condition is unavailable, one target is selected by
the save's randomizer seed.

- The selected target is stable for the entire save.
- Cancelling and retrying an evolution does not reroll it.
- A different new game may select the other target.
- Only the listed branch is restricted. Other item evolutions remain available.

For example, one save may make level-30 Eevee evolve into Espeon, while another
selects Umbreon or Sylveon. Eevee's stone evolutions remain player-controlled.

## Friendship evolutions become levels

Friendship is not checked for these evolutions.

### Level 20

| Source | Evolves into | Notes |
| --- | --- | --- |
| Pichu | Pikachu | No friendship requirement |
| Cleffa | Clefairy | No friendship requirement |
| Igglybuff | Jigglypuff | No friendship requirement |
| Togepi | Togetic | No friendship requirement |
| Azurill | Marill | No friendship requirement |
| Budew | Roselia | Day restriction also removed |
| Chingling | Chimecho | Night restriction also removed |
| Munchlax | Snorlax | No friendship requirement |

### Level 30

| Source | Evolves into | Notes |
| --- | --- | --- |
| Golbat | Crobat | No friendship requirement |
| Alolan Meowth | Alolan Persian | No friendship requirement |
| Eevee | Espeon, Umbreon, or Sylveon | One target selected per save; clock and Fairy-move requirements removed |
| Buneary | Lopunny | No friendship requirement |
| Riolu | Lucario | Day restriction also removed |
| Woobat | Swoobat | No friendship requirement |
| Swadloon | Leavanny | No friendship requirement |
| Snom | Frosmoth | Night restriction also removed |

### Level 40

| Source | Evolves into | Notes |
| --- | --- | --- |
| Chansey | Blissey | No friendship requirement |
| Type: Null | Silvally | No friendship requirement |

## Time restrictions removed

These Pokémon keep the listed level or item trigger but no longer care about
day, night, or evening. Friendship cases already listed above are not repeated.

| Source | Evolves into | New requirement | Notes |
| --- | --- | --- | --- |
| Alolan Rattata | Alolan Raticate | Level 20 | Night removed |
| Cubone | Marowak or Alolan Marowak | Level 28 | One form selected per save; clock and region removed |
| Galarian Linoone | Obstagoon | Level 35 | Night removed |
| Tyrunt | Tyrantrum | Level 39 | Day removed |
| Amaura | Aurorus | Level 39 | Night removed |
| Yungoos | Gumshoos | Level 20 | Day removed |
| Rockruff | Midday or Midnight Lycanroc | Level 25 | One form selected per save; clock removed |
| Own Tempo Rockruff | Dusk Lycanroc | Level 25 | Evening removed; Dusk form remains fixed |
| Fomantis | Lurantis | Level 34 | Day removed |
| Cosmoem | Solgaleo or Lunala | Level 53 | One target selected per save; clock removed |
| Greavard | Houndstone | Level 30 | Night removed |

Happiny, Gligar, both Sneasel forms, Ursaring, and Milcery also lose clock
requirements, but their replacement stones are listed in the item sections
below.

## Regional outcomes selected per save

Using the item or reaching the level below produces one of the listed forms,
selected permanently by the save seed. No region check remains.

| Source | Possible targets | Requirement |
| --- | --- | --- |
| Pikachu | Raichu or Alolan Raichu | Thunder Stone |
| Exeggcute | Exeggutor or Alolan Exeggutor | Leaf Stone |
| Cubone | Marowak or Alolan Marowak | Level 28 |
| Koffing | Weezing or Galarian Weezing | Level 35 |
| Mime Jr. | Mr. Mime or Galarian Mr. Mime | Level 32 |
| Quilava | Typhlosion or Hisuian Typhlosion | Level 36 |
| Dewott | Samurott or Hisuian Samurott | Level 36 |
| Petilil | Lilligant or Hisuian Lilligant | Sun Stone |
| Rufflet | Braviary or Hisuian Braviary | Level 54 |
| Goomy | Sliggoo or Hisuian Sliggoo | Level 40 |
| Bergmite | Avalugg or Hisuian Avalugg | Level 37 |
| Dartrix | Decidueye or Hisuian Decidueye | Level 34 |

Ursaring's region restriction is removed as part of its Moon Stone evolution to
Ursaluna.

## Move requirements become levels

The Pokémon does not need to know the former move or move type.

| Source | Evolves into | New requirement | Former requirement removed |
| --- | --- | ---: | --- |
| Lickitung | Lickilicky | Level 30 | Know Rollout |
| Tangela | Tangrowth | Level 24 | Know Ancient Power |
| Mime Jr. | Mr. Mime or Galarian Mr. Mime | Level 32 | Know Mimic; target selected per save |
| Eevee | Espeon, Umbreon, or Sylveon | Level 30 | Friendship, clock, and Fairy-type move; target selected per save |
| Bonsly | Sudowoodo | Level 16 | Know Mimic |
| Aipom | Ambipom | Level 32 | Know Double Hit |
| Yanma | Yanmega | Level 33 | Know Ancient Power |
| Girafarig | Farigiraf | Level 32 | Know Twin Beam |
| Dunsparce | Dudunsparce | Level 32 | Know Hyper Drill; two-/three-segment result remains personality-based |
| Hisuian Qwilfish | Overqwil | Level 28 | Know Barb Barrage |
| Piloswine | Mamoswine | Level 34 | Know Ancient Power |
| Steenee | Tsareena | Level 28 | Know Stomp |
| Poipole | Naganadel | Level 40 | Know Dragon Pulse |
| Dipplin | Hydrapple | Level 40 | Know Dragon Cheer |
| Clobbopus | Grapploct | Level 35 | Know Taunt |

## Other special requirements become levels

These routes no longer require party composition, weather, walking, battle
counters, unavailable currency, or an unsupported map script.

| Source | Evolves into | New requirement | Former requirement removed |
| --- | --- | ---: | --- |
| Galarian Farfetch'd | Sirfetch'd | Level 30 | Land three critical hits in one battle |
| Mantyke | Mantine | Level 30 | Have Remoraid in the party |
| Primeape | Annihilape | Level 40 | Use Rage Fist 20 times |
| Stantler | Wyrdeer | Level 40 | Use Psyshield Bash 20 times |
| White-Striped Basculin | Basculegion | Level 40 | Take 294 recoil damage; gender still determines the evolved form |
| Galarian Yamask | Runerigus | Level 34 | Take 49 damage and pass beneath the Dusty Bowl arch |
| Pancham | Pangoro | Level 32 | Have a Dark-type Pokémon in the party |
| Sliggoo | Goodra | Level 50 | Level up in rain or fog |
| Hisuian Sliggoo | Hisuian Goodra | Level 50 | Level up in rain or fog |
| Bisharp | Kingambit | Level 55 | Defeat three Leader's Crest Bisharp |
| Pawmo | Pawmot | Level 30 | Walk 1,000 steps together |
| Bramblin | Brambleghast | Level 30 | Walk 1,000 steps together |
| Rellor | Rabsca | Level 30 | Walk 1,000 steps together |
| Chest Form Gimmighoul | Gholdengo | Level 50 | Collect 999 Gimmighoul Coins |
| Roaming Form Gimmighoul | Gholdengo | Level 50 | Collect 999 Gimmighoul Coins |

### Nincada and boxed evolution

Nincada still evolves into Ninjask at level 20. If there is an open party slot
and at least one regular Poké Ball in the Bag, the evolution also creates
Shedinja in the party and consumes one Poké Ball.

This also works when Nincada is raised from the PC with `LEVEL TO CAP`: Ninjask
returns to Nincada's original box slot, while Shedinja is added to the party.
With a full party or no regular Poké Ball, Nincada still becomes Ninjask but no
Shedinja is created.

## Location evolutions become stones

Leveling up in the original special location no longer evolves these Pokémon.
Use the listed stone anywhere.

| Source | Evolves into | New requirement | Former location |
| --- | --- | --- | --- |
| Magneton | Magnezone | Thunder Stone | New Mauville |
| Nosepass | Probopass | Thunder Stone | New Mauville |
| Charjabug | Vikavolt | Thunder Stone | New Mauville |
| Crabrawler | Crabominable | Ice Stone | Shoal Cave ice room |
| Eevee | Leafeon | Leaf Stone | Petalburg Woods |
| Eevee | Glaceon | Ice Stone | Shoal Cave ice room |

## Trade evolutions use the Linking Cord

Use a Linking Cord directly. No trade or partner species is required.

| Source | Evolves into |
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
| Pumpkaboo (Small) | Gourgeist (Small) |
| Pumpkaboo (Average) | Gourgeist (Average) |
| Pumpkaboo (Large) | Gourgeist (Large) |
| Pumpkaboo (Super) | Gourgeist (Super) |

Karrablast and Shelmet no longer need to be traded for one another. Pumpkaboo
retains its size when it evolves.

## Held-item trades become stones

Use the stone directly; do not make the Pokémon hold its former trade item.

| Source | Evolves into | New requirement | Former trade item |
| --- | --- | --- | --- |
| Poliwhirl | Politoed | Dawn Stone | King's Rock |
| Slowpoke | Slowking | Dawn Stone | King's Rock |
| Onix | Steelix | Dawn Stone | Metal Coat |
| Scyther | Scizor | Dusk Stone | Metal Coat |
| Seadra | Kingdra | Water Stone | Dragon Scale |
| Rhydon | Rhyperior | Moon Stone | Protector |
| Electabuzz | Electivire | Thunder Stone | Electirizer |
| Magmar | Magmortar | Fire Stone | Magmarizer |
| Porygon | Porygon2 | Dawn Stone | Upgrade |
| Porygon2 | Porygon-Z | Dusk Stone | Dubious Disc |
| Feebas | Milotic | Shiny Stone | Prism Scale |
| Dusclops | Dusknoir | Dusk Stone | Reaper Cloth |
| Clamperl | Huntail | Water Stone | Deep Sea Tooth |
| Clamperl | Gorebyss | Shiny Stone | Deep Sea Scale |
| Spritzee | Aromatisse | Shiny Stone | Sachet |
| Swirlix | Slurpuff | Sun Stone | Whipped Dream |

## Other held evolution items become stones

| Source | Evolves into | New requirement | Former requirement |
| --- | --- | --- | --- |
| Happiny | Chansey | Shiny Stone | Hold Oval Stone during the day and level up |
| Gligar | Gliscor | Moon Stone | Hold Razor Fang at night and level up |
| Sneasel | Weavile | Dusk Stone | Hold Razor Claw at night and level up |
| Hisuian Sneasel | Sneasler | Dawn Stone | Hold Razor Claw during the day and level up |

## Applin family

| Source | Evolves into | New requirement | Former item |
| --- | --- | --- | --- |
| Applin | Flapple | Fire Stone | Tart Apple |
| Applin | Appletun | Sun Stone | Sweet Apple |
| Applin | Dipplin | Leaf Stone | Syrupy Apple |
| Dipplin | Hydrapple | Level 40 | Know Dragon Cheer |

All three Applin branches remain player-controlled.

## Other special evolution items become stones

| Source | Evolves into | New requirement | Former item |
| --- | --- | --- | --- |
| Scyther | Kleavor | Leaf Stone | Black Augurite |
| Galarian Slowpoke | Galarian Slowbro | Ice Stone | Galarica Cuff |
| Galarian Slowpoke | Galarian Slowking | Shiny Stone | Galarica Wreath |
| Phony Sinistea | Phony Polteageist | Dawn Stone | Cracked Pot |
| Antique Sinistea | Antique Polteageist | Dawn Stone | Chipped Pot |
| Ursaring | Ursaluna | Moon Stone | Peat Block at night in Hisui |
| Kubfu | Single Strike Urshifu | Dusk Stone | Scroll of Darkness |
| Kubfu | Rapid Strike Urshifu | Water Stone | Scroll of Waters |
| Charcadet | Armarouge | Sun Stone | Auspicious Armor |
| Charcadet | Ceruledge | Dusk Stone | Malicious Armor |
| Duraludon | Archaludon | Thunder Stone | Metal Alloy |
| Counterfeit Poltchageist | Unremarkable Sinistcha | Ice Stone | Unremarkable Teacup |
| Artisan Poltchageist | Masterpiece Sinistcha | Ice Stone | Masterpiece Teacup |

Sinistea and Poltchageist retain the outcome matching their source form. Kubfu,
Charcadet, Scyther, and Galarian Slowpoke retain player-controlled branches.

## Milcery and Alcremie

Milcery no longer needs a Sweet, a particular time, or a spin. Use a stone to
choose the Alcremie decoration:

| Decoration | Stone |
| --- | --- |
| Strawberry | Fire Stone |
| Berry | Water Stone |
| Love | Shiny Stone |
| Star | Thunder Stone |
| Clover | Leaf Stone |
| Flower | Sun Stone |
| Ribbon | Dawn Stone |

The save seed independently selects one cream flavor shared by every decoration
in that save: Vanilla, Ruby, Matcha, Mint, Lemon, Salted, Ruby Swirl, Caramel
Swirl, or Rainbow Swirl.

Changing the stone changes the decoration, not the save's selected cream flavor.

## Eevee quick reference

Eevee's available routes are:

| Target | Requirement |
| --- | --- |
| Vaporeon | Water Stone |
| Jolteon | Thunder Stone |
| Flareon | Fire Stone |
| Leafeon | Leaf Stone |
| Glaceon | Ice Stone |
| Espeon, Umbreon, or Sylveon | Level 30; one of these three is selected per save |

The five stone routes are always player-controlled. The former Petalburg Woods
and Shoal Cave location routes are removed so they cannot unexpectedly preempt
the save's level-30 branch.

## Requirements that remain unchanged

Unless a route appears above, its authored behavior remains active. In
particular:

- Gender-based evolutions remain gender-based.
- Tyrogue's stat-comparison outcomes remain intact.
- Other weather, party-composition, battle-tracker, walking, recoil, and
  script-based requirements remain intact unless explicitly replaced above.
- Dunsparce retains its personality-based Dudunsparce segment outcome.
- Own Tempo Rockruff still evolves only into Dusk Lycanroc.
- Ordinary evolution stones not mentioned as replacements continue to work as
  authored.

No enabled evolution requires a trade, a particular learned move or move type,
or one of the six removed evolution locations. Every item-based evolution uses
a standard evolution stone or the Linking Cord.
