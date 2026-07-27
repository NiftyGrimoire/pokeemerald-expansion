#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

enum RandomizerCategory
{
    RANDOMIZER_CATEGORY_ENCOUNTER = 1,
    RANDOMIZER_CATEGORY_ABILITY,
    RANDOMIZER_CATEGORY_LEARNSET,
};

void InitRandomizerData(void);
u32 GetRandomizerSeed(void);
u32 RandomizerHash(u32 seed, enum RandomizerCategory category, u32 key1, u32 key2, u32 key3);
bool32 IsSpeciesRandomizerEligible(enum Species species);

#endif // GUARD_RANDOMIZER_H
