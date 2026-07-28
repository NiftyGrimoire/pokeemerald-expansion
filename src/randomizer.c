#include "global.h"
#include "pokemon.h"
#include "random.h"
#include "random_mon_generation.h"
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
    const u16 *formTable;

    if (species <= SPECIES_NONE || species >= NUM_SPECIES || species == SPECIES_EGG)
        return FALSE;
    if (!IsSpeciesEnabled(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    formTable = GetSpeciesFormTable(GET_BASE_SPECIES_ID(species));
    return !(speciesInfo->isMegaEvolution
          || speciesInfo->isPrimalReversion
          || speciesInfo->isUltraBurst
          || speciesInfo->isGigantamax
          || speciesInfo->isTeraForm
          || speciesInfo->isTotem
          || !IsSpeciesFormUsableOutsideSpecialContext(species, formTable));
}

enum Species GetRandomizedSpeciesForEncounter(enum Species originalSpecies, u16 mapId, enum RandomizerEncounterType encounterType, u8 slot)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_ENCOUNTERS
    u32 eligibleSpeciesCount = 0;
    u32 selectedIndex;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerEligible(species))
            eligibleSpeciesCount++;
    }

    if (eligibleSpeciesCount == 0)
        return originalSpecies;

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_ENCOUNTER,
                                   mapId,
                                   originalSpecies,
                                   ((u32)encounterType << 8) | slot)
                  % eligibleSpeciesCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerEligible(species) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}
