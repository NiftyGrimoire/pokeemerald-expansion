#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

enum RandomizerCategory
{
    RANDOMIZER_CATEGORY_ENCOUNTER = 1,
    RANDOMIZER_CATEGORY_ABILITY,
    RANDOMIZER_CATEGORY_LEARNSET,
    RANDOMIZER_CATEGORY_LEGENDARY_ENCOUNTER,
};

enum RandomizerEncounterType
{
    RANDOMIZER_ENCOUNTER_LAND,
    RANDOMIZER_ENCOUNTER_WATER,
    RANDOMIZER_ENCOUNTER_ROCK_SMASH,
    RANDOMIZER_ENCOUNTER_FISHING,
    RANDOMIZER_ENCOUNTER_FEEBAS,
    RANDOMIZER_ENCOUNTER_MASS_OUTBREAK,
};

void InitRandomizerData(void);
u32 GetRandomizerSeed(void);
u32 RandomizerHash(u32 seed, enum RandomizerCategory category, u32 key1, u32 key2, u32 key3);
bool32 IsSpeciesRandomizerEligible(enum Species species);
bool32 IsSpeciesRandomizerLegendaryEncounterEligible(enum Species species);
enum Species GetRandomizedSpeciesForEncounter(enum Species originalSpecies, u16 mapId, enum RandomizerEncounterType encounterType, u8 slot, u8 encounterDifficulty);
enum Species GetRandomizedSpeciesForLegendaryEncounter(enum Species originalSpecies, u16 mapId, u8 slot);

#endif // GUARD_RANDOMIZER_H
