#include "global.h"
#include "pokemon.h"
#include "random.h"
#include "randomizer.h"

#define RANDOMIZER_ZERO_SEED_FALLBACK 0x6D2B79F5

static u32 MixRandomizerValue(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    value ^= value >> 16;
    return value;
}

void InitRandomizerData(void)
{
#if RANDOMIZER_ENABLED
    u32 seed = Random32();

    if (seed == 0)
        seed = RANDOMIZER_ZERO_SEED_FALLBACK;

    gSaveBlock3Ptr->randomizerSeed = seed;
    gSaveBlock3Ptr->randomizerVersion = RANDOMIZER_ALGORITHM_VERSION;
#endif
}

u32 GetRandomizerSeed(void)
{
#if RANDOMIZER_ENABLED
    return gSaveBlock3Ptr->randomizerSeed;
#else
    return 0;
#endif
}

u32 RandomizerHash(u32 seed, enum RandomizerCategory category, u32 key1, u32 key2, u32 key3)
{
    u32 hash = MixRandomizerValue(seed ^ 0x9E3779B9);

    hash = MixRandomizerValue(hash ^ (u32)category);
    hash = MixRandomizerValue(hash ^ key1);
    hash = MixRandomizerValue(hash ^ key2);
    hash = MixRandomizerValue(hash ^ key3);
    return hash;
}

bool32 IsSpeciesRandomizerEligible(enum Species species)
{
    const struct SpeciesInfo *speciesInfo;

    if (species <= SPECIES_NONE || species >= NUM_SPECIES || species == SPECIES_EGG)
        return FALSE;
    if (!IsSpeciesEnabled(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    return !(speciesInfo->isMegaEvolution
          || speciesInfo->isPrimalReversion
          || speciesInfo->isUltraBurst
          || speciesInfo->isGigantamax
          || speciesInfo->isTeraForm
          || speciesInfo->isTotem);
}
