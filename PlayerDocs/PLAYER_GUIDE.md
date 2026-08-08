# Pokémon Emerald Randomizer — Player Guide

This ROM hack keeps the story and region of Pokémon Emerald, but redesigns team
building and progression around a fast, deterministic randomized Nuzlocke-style
playthrough. This guide describes the differences a player should expect relative
to vanilla Emerald.

The guide contains mechanical and reward spoilers, but avoids revealing specific
randomized results because those depend on the save file.

## The randomizer seed

Each new game creates its own randomizer seed. The seed determines starters,
encounters, trainer teams, abilities, learnsets, TM moves, item pickups, and other
randomized rewards.

- A result is stable throughout one save. Reloading does not reroll it.
- A different new game normally produces different results.
- Duplicate Pokémon and item results can occur unless a feature explicitly says
  otherwise.
- Randomization does not consume the normal battle or field random-number stream.

Saves from older development builds may be incompatible. Randomizer algorithm
updates can also change deterministic results expected from an older build.

## Pokémon encounters

### Ordinary wild encounters

Grass, cave, Surf, fishing, Rock Smash, outbreak, Feebas, and ordinary scripted
encounters have randomized species. The original encounter method, rarity slot,
and level are retained.

Replacement Pokémon are selected from a base-stat range suited to the encounter
table's level. Early areas therefore favor weaker species, while later and
stronger encounter tables can produce stronger species. Ordinary pools exclude
Legendary, Mythical, Ultra Beast, Paradox, battle-only, and unusable forms.
Cosmetic forms do not make a species family more likely: the game selects that
species group first, then deterministically chooses an eligible appearance.
Mechanically distinct forms, including regional forms, remain separate choices.

### Legendary encounters

Emerald's scripted Legendary encounters are randomized separately. A qualifying
Legendary, sub-Legendary, or Paradox encounter becomes another usable Pokémon
from those classifications. Its authored level and story behavior remain intact.
Mythical Pokémon and Ultra Beasts are not part of this pool. Every Pokémon from
one of these special encounters has at least three perfect IVs.

### Starters

The three starter choices are randomized and distinct. Choices are balanced
base-stage Pokémon with a complete three-stage evolution line and an appropriate
starter-level base-stat total. The selection screen, granted Pokémon, and other
starter references all use the same result. The chosen starter has at least four
perfect IVs.

## Trainers and battles

Enemy trainer species are randomized independently for each trainer and party
slot. Replacements remain near the original Pokémon's base-stat total, preserve
authored party sizes and levels, and receive legal randomized level-up moves.
Separate rival battles are separate randomized encounters; the rival is not
required to keep or evolve one persistent starter family.

Trainer AI is stronger than vanilla:

- Ordinary trainers use competent move scoring and can choose randomized
  equivalent switch-ins.
- Boss trainers receive stronger switching and strategy logic.
- Trainers do not begin a battle with omniscient knowledge of randomized
  abilities; abilities become known when revealed.
- Battle facilities and player-controlled partners retain their specialized
  behavior.

Battle Style is permanently **Set**. The Options menu does not include a Battle
Style setting, and the player does not receive a free switch after defeating an
opposing trainer Pokémon.

## Abilities

Ordinary Pokémon evolution families receive one deterministic randomized
ability. Members of the same family keep consistent ability behavior through
evolution, and the result applies to player, wild, gift, and trainer Pokémon.

Families and forms whose authored abilities are required for safe form changes
or unique mechanics retain those authored abilities. Examples include form and
battle transformations that would break without their signature ability.

## Level-up moves

Level-up learnsets are randomized per species and save. A species has the same
learnset everywhere in one save, including wild Pokémon, trainer Pokémon,
level-up prompts, evolution learning, the Move Reminder, and Pokédex displays.

Each species has a 20-move schedule designed around the level caps:

- Two moves are available at level 1.
- Opening attacks strongly favor 40 power or less. Attacks from 41 through 60
  are uncommon fallbacks, and stronger attacks cannot be starting moves.
- Only basic status moves can appear at level 1.
- Later damaging moves trend upward in power. Equivalent same-type attacks are
  three times as likely as coverage attacks.
- Strong setup, recovery, weather, hazard, protection, and utility moves become
  more likely later in the game; elite late-game status moves can be weighted as
  highly as the best on-target same-type attacks.
- Moves are not duplicated within one species' learnset.
- Unusable or species-dependent special cases such as Transform, Sketch, Dark
  Void, Aura Wheel, Tera Blast, and Struggle are excluded.

Egg moves remain authored, but the Day Care is inaccessible in normal play.

## TMs, HMs, and tutors

The 50 TM items retain their familiar TM slot names, but each slot teaches a
unique randomized move per save. TM descriptions and every teaching or lookup
screen display the randomized move. Selection favors strong attacks and useful
status moves while keeping weaker or drawback-heavy moves possible.

- Every enabled real Pokémon can learn every TM.
- Each of the 50 randomized TM moves is unique within the save.
- HM moves never occupy randomized TM slots.
- HMs retain their authored moves and authored move compatibility.
- Move Tutor and Egg Move compatibility remain authored.

Field HM actions are unlocked by the original badge requirements, but a Pokémon
does not need to know the HM. Once the badge requirement is met, the first
non-Egg Pokémon in the party can perform the action. The appropriate field
context is still required.

## Level caps and EVs

Hard level caps follow Emerald's badge progression:

| Progress | Level cap |
| --- | ---: |
| Before the first badge | 15 |
| After badge 1 | 19 |
| After badge 2 | 24 |
| After badge 3 | 29 |
| After badge 4 | 31 |
| After badge 5 | 33 |
| After badge 6 | 42 |
| After badge 7 | 46 |
| After badge 8 | 58 |
| After becoming Champion | 100 |

Pokémon at the current cap cannot gain more experience. Rare Candies and EXP
Candies cannot raise a Pokémon above it.

Normal EV gain is disabled:

- Battles do not award EVs.
- Vitamins, feathers, and other EV-raising items cannot add EVs.
- EV-focused items are not generated by the useful-item randomizer.

## Level to Cap

The party menu and Pokémon Storage provide a **LEVEL TO CAP** action. It raises
the selected Pokémon one level at a time until the current cap, preserving all
intermediate move-learning and evolution opportunities.

The Storage version works directly on a boxed Pokémon and does not require a
free party slot. Cancelling a move choice or stopping an evolution safely keeps
the progress already completed and stops the larger operation.

## Evolution changes

Evolution requirements have been streamlined so randomized learnsets, the clock,
trading, friendship grinding, or specific locations do not block a run.

- Friendship evolutions use explicit levels, generally 20, 30, or 40 according
  to their intended stage.
- Move, move-type, clock, and location requirements are replaced by levels or
  ordinary evolution stones.
- Trade evolutions use the **Linking Cord**.
- Former held-item trades and species-specific evolution items use ordinary
  stones or the Linking Cord.
- Regional and day/night branches use distinct stones so the player chooses the
  result; ordinary forms often remain the default level evolution.
- All eight Eevee outcomes are selected with distinct stones.
- Milcery uses seven stones to select a Vanilla Cream decoration.

The exact species-by-species rules are intentionally discoverable through play,
but every enabled evolution has been audited to remove unsupported trade and
move requirements.

For the complete spoiler reference covering every changed evolution, see
[EVOLUTION_GUIDE.md](EVOLUTION_GUIDE.md).

## Items and the Bag

The general Items pocket holds 64 distinct item stacks instead of vanilla's 30.
Other pocket sizes and the 999-per-item stack limit are unchanged.

### Visible item pickups

Ordinary visible item balls are randomized to useful held or evolution items.
Each pickup is fixed for the save using its map and object identity. The initial
pool contains:

- The ten standard evolution stones.
- Linking Cord.
- Six former evolution-related held items that remain useful in battle.
- All 18 single-type damage boosters.
- Choice Band, Choice Specs, and Choice Scarf.
- Focus Sash, Focus Band, Muscle Band, Wise Glasses, and Expert Belt.
- Scope Lens, Wide Lens, and Zoom Lens.
- Eviolite, Leftovers, Life Orb, Assault Vest, and Clear Amulet.
- Quick Claw, Loaded Dice, Bright Powder, White Herb, Power Herb, Covert Cloak,
  and Grip Claw.
- Damp Rock, Heat Rock, Smooth Rock, Icy Rock, Light Clay, and Terrain Extender.
- Electric Seed, Grassy Seed, Misty Seed, and Psychic Seed.

The pool contains 67 results: 11 evolution items and 56 held items. Every entry
has equal weight, so duplicates are possible and no category is guaranteed.

The pool intentionally excludes Medicine, X-items, Berries, Poké Ball variants,
pure money treasures, TMs, HMs, and Key Items as replacement results. Authored
Poké Ball and money-treasure pickups can still be replaced by a useful item.
There are no guaranteed evolution-item placements or progression bands.

### Hidden items

Hidden item spots are disabled. They cannot be collected and the Itemfinder does
not detect them. Obtainable overworld pickups are intended to be visible.

### Direct gifts and protected transactions

Eligible direct NPC and story gifts use the same useful pool with their own
stable identities. Progression-sensitive items, Berries, TMs, HMs, Key Items,
shops, exchanges, prize systems, and internal transfers normally remain
authored unless listed as a specific exception below.

## Important progression and reward changes

### Early-game supplies

The Birch Lab catching tutorial gives the authored five Poké Balls. A stationary
girl in Oldale Town gives **200 Ultra Balls and ¥200,000** once. If the Balls do
not fit, the Oldale reward remains available for a later attempt.

Receiving the starter also grants two reusable Key Items:

- **Portable Healer:** fully restores party HP, PP, and status outside battle.
- **Repel Toggle:** enables or disables an indefinite block on ordinary random
  encounters regardless of the lead Pokémon's level. Scripted encounters remain
  available. Lures and ordinary Repels share the underlying state, so turn the
  toggle off before using one of those items.

### Day Care

The Route 117 Day Care entrance is blocked by a closure NPC. Breeding is not
available through normal play, although the underlying engine and egg-related
data have not been removed.

### Shoal Cave

Shoal Cave is permanently at low tide, so its low-tide route and TM07 are always
accessible. The renewable Shoal Salt/Shoal Shell collection and Shell Bell
exchange are removed.

- The four reachable former Shoal Salt spots are one-time randomized pickups.
- The high-tide Shoal Shell spots are inaccessible.
- Shoal Salt and Shoal Shell cannot appear as randomized rewards.
- The old man comments on the permanent low tide but no longer exchanges
  ingredients.

### Scanner exchange

Captain Stern exchanges the Scanner for one player-selected evolution resource:
Fire, Water, Thunder, Leaf, Ice, Sun, Moon, Shiny, Dusk, or Dawn Stone, or a
Linking Cord. The Scanner is consumed only after the chosen item successfully
enters the Bag.

### Mirage Tower and fossils

Mirage Tower remains visible until its fossil event is completed. The Root
Fossil/Claw Fossil choice remains authored. After the game, Desert Underpass
contains the fossil that was not selected.

### Berry rewards

Several renewable or phrase-dependent rewards are finite:

- The Route 120 Berry NPC gives one authored Berry once per save instead of
  daily.
- Kiri gives her original two-Berry bundle once per save. Both must fit before
  either is granted.
- Berry Master's wife no longer uses the Easy Chat phrase puzzle. She gives one
  of the five former special-phrase Berries once per save.
- Full Berry pockets do not permanently consume these rewards.

The Sootopolis Seedot/Lotad brothers still judge sizes and track records, but no
longer award items. The Slateport Fan Club condition judge still gives the five
authored Contest Scarves.

### Special randomized sellers and challenges

Most shops remain authored. Two sellers are deliberate exceptions:

- The Mt. Chimney Lava Cookie seller offers one fixed randomized useful item for
  ¥200. Repeated purchases give the same item within that save.
- After the Seashore House challenge, its owner sells one fixed randomized
  useful item for ¥300. The challenge reward is six copies of one fixed
  randomized item.

Both sellers check space for the actual replacement before charging. Their
policy does not apply to other shops.

Trick House keeps mixed rewards: eligible non-TM puzzle prizes randomize, while
its TM prize remains a TM slot and teaches that save's randomized move.

### Other authored rewards

The following notable transactions deliberately retain authored behavior:

- Prof. Cozmo's Meteorite exchange and TM27 reward.
- Harbor Mail for the Coin Case.
- Mach Bike and Acro Bike grants.
- Powder Jar and Berry Powder purchases.
- Battle Frontier Symbol Berry prizes.
- Root and Claw Fossils.
- Downloaded E-Reader Enigma Berry delivery.

The Mystery Gift Battle Card item prize is randomized and remains retryable if
the Bag is full.

## Summary-screen IV display

On the Pokémon Skills summary page, press **A** to switch from normal stats to
the Pokémon's six numeric stored IVs. Press **A** again to return to stats.
Hyper Training does not disguise the stored values shown by this view.

## What remains close to vanilla Emerald

- The Hoenn map, main story, badges, and most progression flags remain Emerald's.
- Wild encounter methods, slot rarity, and levels remain authored even when the
  species changes.
- Trainer party sizes and levels remain authored.
- Move power, type, accuracy, and effects are not randomized.
- HMs and Move Tutors teach their authored moves.
- Most shops, exchanges, Key Items, fossils, and progression rewards remain
  authored unless this guide identifies an exception.
- Berry planting, Contests, Berry Powder, the Coin Case/Game Corner, and many
  peripheral systems still exist unless access is explicitly changed above.

This hack is still under development. Some systems retained from Emerald are
scheduled for later streamlining, but planned changes are not active gameplay
rules until they are implemented and documented here.
