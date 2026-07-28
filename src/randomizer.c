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

static bool32 IsOrdinaryEncounterCandidate(enum Species species, u16 minBST, u16 maxBST)
{
    const struct SpeciesInfo *speciesInfo;
    u32 bst;

    if (!IsSpeciesRandomizerEligible(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    if (speciesInfo->isRestrictedLegendary
     || speciesInfo->isSubLegendary
     || speciesInfo->isMythical
     || speciesInfo->isUltraBeast
     || speciesInfo->isParadox)
    {
        return FALSE;
    }

    bst = GetSpeciesBaseStatTotal(species);
    if (bst < minBST || bst > maxBST)
        return FALSE;

    return TRUE;
}

enum Species GetRandomizedSpeciesForEncounter(enum Species originalSpecies, u16 mapId, enum RandomizerEncounterType encounterType, u8 slot, u8 encounterDifficulty)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_ENCOUNTERS
    u16 minBST;
    u16 maxBST;
    u32 ordinaryCount = 0;
    u32 selectedIndex;

    if (encounterDifficulty <= 10)
    {
        minBST = 180;
        maxBST = 360;
    }
    else if (encounterDifficulty <= 20)
    {
        minBST = 240;
        maxBST = 420;
    }
    else if (encounterDifficulty <= 30)
    {
        minBST = 300;
        maxBST = 480;
    }
    else if (encounterDifficulty <= 40)
    {
        minBST = 360;
        maxBST = 540;
    }
    else if (encounterDifficulty <= 50)
    {
        minBST = 420;
        maxBST = 600;
    }
    else
    {
        minBST = 480;
        maxBST = 720;
    }

    for (;;)
    {
        ordinaryCount = 0;
        for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
        {
            if (IsOrdinaryEncounterCandidate(species, minBST, maxBST))
                ordinaryCount++;
        }

        if (ordinaryCount > 0)
            break;

        if (minBST == 0 && maxBST == 1530)
            return originalSpecies;

        if (minBST > 60)
            minBST -= 60;
        else
            minBST = 0;

        if (maxBST < 1470)
            maxBST += 60;
        else
            maxBST = 1530;
    }

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_ENCOUNTER,
                                   mapId,
                                   originalSpecies,
                                   ((u32)encounterType << 8) | slot)
                  % ordinaryCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsOrdinaryEncounterCandidate(species, minBST, maxBST) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}

bool32 IsSpeciesRandomizerLegendaryEncounterEligible(enum Species species)
{
    const struct SpeciesInfo *speciesInfo;

    if (!IsSpeciesRandomizerEligible(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    if (!(speciesInfo->isRestrictedLegendary
       || speciesInfo->isSubLegendary
       || speciesInfo->isParadox))
    {
        return FALSE;
    }

    if (speciesInfo->isMythical || speciesInfo->isUltraBeast)
        return FALSE;

    return TRUE;
}

enum Species GetRandomizedSpeciesForLegendaryEncounter(enum Species originalSpecies, u16 mapId, u8 slot)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_LEGENDARY_ENCOUNTERS
    u32 eligibleCount = 0;
    u32 selectedIndex;

    if (!IsSpeciesRandomizerLegendaryEncounterEligible(originalSpecies))
        return originalSpecies;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerLegendaryEncounterEligible(species))
            eligibleCount++;
    }

    if (eligibleCount == 0)
        return originalSpecies;

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_LEGENDARY_ENCOUNTER,
                                   mapId,
                                   (u32)originalSpecies,
                                   slot)
                  % eligibleCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerLegendaryEncounterEligible(species) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}
