# Wild Encounter Randomization

## Design

The wild encounter randomizer lets the vanilla encounter system decide whether
an encounter occurs, which table slot is selected, and what level the Pokemon
has. It then deterministically replaces only the selected species.

This preserves:

- Encounter rates.
- Encounter methods.
- Weighted slot rarity.
- Encounter levels.
- Repel and Keen Eye checks.
- Vanilla slot selection influenced by overworld abilities.

## Feature Gates

`include/config/randomizer.h` defines two relevant gates:

```c
#define RANDOMIZER_ENABLED TRUE
#define RANDOMIZER_ENCOUNTERS TRUE
```

The master gate controls the overall per-save randomizer. The encounter gate
controls wild-species substitution.

## Encounter Identities

`include/randomizer.h` defines distinct encounter identities:

```c
enum RandomizerEncounterType
{
    RANDOMIZER_ENCOUNTER_LAND,
    RANDOMIZER_ENCOUNTER_WATER,
    RANDOMIZER_ENCOUNTER_ROCK_SMASH,
    RANDOMIZER_ENCOUNTER_FISHING,
    RANDOMIZER_ENCOUNTER_FEEBAS,
    RANDOMIZER_ENCOUNTER_MASS_OUTBREAK,
};
```

The encounter identity is part of the deterministic hash input. For example,
land slot 3 and Surf slot 3 do not necessarily resolve to the same species.

## Standard Land, Water, and Rock Smash Path

Normal walking and surfing encounters enter `StandardWildEncounter` in
`src/wild_encounter.c`. The function performs the original checks:

1. Confirm that the map has the appropriate encounter table.
2. Check the metatile transition.
3. Roll against the vanilla encounter rate.
4. Check for a roamer.
5. Check for a mass outbreak on land.
6. Generate a standard land or water encounter.

Supported callers pass `WILD_CHECK_RANDOMIZE` to `TryGenerateWildMon` in
addition to the existing flags:

```c
TryGenerateWildMon(
    wildMonInfo,
    WILD_AREA_LAND,
    WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE | WILD_CHECK_RANDOMIZE
);
```

`WILD_CHECK_RANDOMIZE` is an opt-in call-site flag. Shared encounter code used
by unsupported special systems keeps its original species when the flag is
absent.

Inside `TryGenerateWildMon`, the vanilla code selects a slot first:

```c
wildMonIndex = ChooseWildMonIndex_Land();
wildMonIndex = ChooseWildMonIndex_Water();
wildMonIndex = ChooseWildMonIndex_Rocks();
```

Land and water selection still considers Magnet Pull, Static, Lightning Rod,
Flash Fire, Harvest, and Storm Drain before performing the normal weighted
roll.

These abilities act on the original encounter table. For example, Magnet Pull
favors a slot whose original species is Steel-type, but the randomized
replacement is not guaranteed to be Steel-type.

The game then selects the vanilla level and performs Repel and Keen Eye checks:

```c
level = ChooseWildMonLevel(wildMonInfo->wildPokemon, wildMonIndex, area);
```

Only after these checks pass does the code read and replace the species.

## Encounter Context

The current map group and number are packed into one value:

```c
u16 mapId = (gSaveBlock1Ptr->location.mapGroup << 8)
          | gSaveBlock1Ptr->location.mapNum;
```

The vanilla area is translated to a randomizer encounter identity:

```text
WILD_AREA_LAND  -> RANDOMIZER_ENCOUNTER_LAND
WILD_AREA_WATER -> RANDOMIZER_ENCOUNTER_WATER
WILD_AREA_ROCKS -> RANDOMIZER_ENCOUNTER_ROCK_SMASH
```

The selected species is passed to the resolver with all explicit context:

```c
species = GetRandomizedSpeciesForEncounter(
    originalSpecies,
    mapId,
    encounterType,
    wildMonIndex,
    encounterDifficulty
);
```

`encounterDifficulty` filters the candidate pool but is not part of the hash
identity. It is derived from fixed table data and is therefore stable for a
given map and encounter method.

## Deterministic Species Resolution

`GetRandomizedSpeciesForEncounter` is implemented in `src/randomizer.c`.
It first counts every ordinary species accepted by the shared eligibility
rules and the selected BST band.

It then calculates:

```c
selectedIndex = RandomizerHash(
    GetRandomizerSeed(),
    RANDOMIZER_CATEGORY_ENCOUNTER,
    mapId,
    originalSpecies,
    ((u32)encounterType << 8) | slot
) % ordinarySpeciesCount;
```

The complete mapping identity is:

```text
saved randomizer seed
+ encounter category
+ current map
+ original species
+ encounter method
+ selected slot
```

The resolver walks the eligible species a second time and returns the species
at `selectedIndex`.

The result has the following properties:

- Repeating the same context in one save produces the same species.
- Different maps, slots, or methods can produce different species.
- A different save seed produces a different mapping.
- Resolution does not read or advance either mutable global RNG.

The replacement is finally created at the original level:

```c
CreateWildMon(species, level);
```

## Species Eligibility

`IsSpeciesRandomizerEligible` rejects:

- `SPECIES_NONE`.
- Eggs and out-of-range IDs.
- Species disabled by build configuration.
- Mega Evolutions.
- Primal Reversions.
- Ultra Burst forms.
- Gigantamax forms.
- Tera forms.
- Totem forms.
- Fusion and transformation-only forms.

It shares Expansion's existing form-safety logic from
`src/random_mon_generation.c`. This also rejects forms such as Zen Mode
Darmanitan and fused Kyurem that should not be generated as independent wild
Pokemon.

Ordinary randomized encounters additionally exclude restricted Legendary,
sub-Legendary, Mythical, Ultra Beast, and Paradox species. Restricted
Legendary, sub-Legendary, and Paradox species are reserved for the separate
scripted Legendary encounter randomizer.

## BST-Scaled Encounter Pools

The randomizer calculates a stable difficulty value from each encounter table.
For every slot, it takes the midpoint of the fixed minimum and maximum levels,
weights that midpoint by the vanilla slot odds, and rounds the resulting table
average down.

Land, Surf, and Rock Smash use their own tables. Fishing is divided into Old
Rod, Good Rod, and Super Rod groups, so waiting for a better rod can provide a
stronger pool on the same route. This is intentional for player-managed
one-encounter-per-route Nuzlocke rules.

The inclusive difficulty bands are:

| Encounter difficulty | Eligible BST |
| --- | --- |
| 1-10 | 180-360 |
| 11-20 | 240-420 |
| 21-30 | 300-480 |
| 31-40 | 360-540 |
| 41-50 | 420-600 |
| 51+ | 480-720 |

Every eligible species in the chosen band has one entry in the candidate pool.
Vanilla slot weights affect the difficulty calculation only; they do not make
lower-BST replacements more likely.

If a band is empty, both bounds expand by 60 BST per pass until candidates
exist. The original species is retained only if the fully expanded ordinary
pool is empty.

Feebas uses the midpoint of its fixed level range. Mass outbreaks use the
saved outbreak level.

## Fishing

`GenerateFishingWildMon` follows a separate path:

1. Select the vanilla rod-specific slot.
2. Read the original species.
3. Select the vanilla level.
4. Update the fishing chain.
5. Resolve with `RANDOMIZER_ENCOUNTER_FISHING`.
6. Create the randomized species at the original level.

The randomized result is returned to `FishingWildEncounter`, so fishing-related
tracking records the species the player actually encountered.

## Feebas

Feebas tile selection remains vanilla. If the coordinate check succeeds,
`FishingWildEncounter` resolves `SPECIES_FEEBAS` with the distinct
`RANDOMIZER_ENCOUNTER_FEEBAS` identity and creates the replacement at the
vanilla Feebas level.

Using a distinct identity prevents the Feebas result from colliding with an
ordinary fishing slot.

## Mass Outbreaks

The standard land path checks mass outbreaks before the normal encounter table.
When an outbreak succeeds, `SetUpMassOutbreakEncounter` resolves the saved
outbreak species with `RANDOMIZER_ENCOUNTER_MASS_OUTBREAK`.

The outbreak's original level and prescribed moves remain unchanged.
Overworld-visible outbreak generation does not pass `WILD_CHECK_RANDOMIZE` and
therefore remains outside the current scope.

## Sweet Scent, Double Battles, and Debug Encounters

Sweet Scent requests randomization for supported land, water, and outbreak
encounters.

For a double wild battle, the generator runs twice. Each vanilla slot roll is
resolved independently. Selecting the same original species and slot twice
produces the same randomized species; selecting different slots can produce
different species.

The standard debug encounter also requests randomization so it exercises the
same behavior as normal land encounters.

## Deliberate Bypasses

The following paths do not currently request species randomization:

- Roamers.
- Scripted or static Pokemon.
- Battle Pike encounters.
- Battle Pyramid encounters.
- DexNav.
- Overworld-visible encounters.

This boundary is enforced through the opt-in call-site flag rather than by
trying to infer special encounter state inside the resolver.

## End-to-End Flow

```text
Step or metatile encounter check
        |
        v
Vanilla encounter-rate roll
        |
        v
Roamer and outbreak checks
        |
        v
Vanilla weighted slot selection
        |
        v
Vanilla level selection
        |
        v
Repel and ability checks
        |
        v
Hash(seed, map, original species, method, slot)
        |
        v
Filter ordinary species by the method's fixed-table BST band
        |
        v
Select an eligible replacement species uniformly
        |
        v
Create the replacement at the original level
        |
        v
Start the normal wild battle
```

The core invariant is:

> Vanilla determines the shape of the encounter; the randomizer
> deterministically substitutes its species.

## Tests

`test/randomizer.c` verifies that:

- Representative valid species are accepted.
- Invalid and battle-only species are rejected.
- Encounter resolution returns an eligible species.
- Every difficulty boundary returns a species inside its preferred BST band.
- Ordinary encounters exclude all special species classifications.
- Low-level and high-level difficulties respect their intended BST limits.
- Identical context produces an identical result.
- Map, method, slot, and save-seed changes separate results.
