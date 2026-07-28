#include "global.h"
#include "pokemon.h"
#include "random.h"
#include "randomizer.h"
#include "test/test.h"

static u16 GetTestSpeciesBaseStatTotal(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u16 bst = (u16)info->baseHP
            + (u16)info->baseAttack
            + (u16)info->baseDefense
            + (u16)info->baseSpeed
            + (u16)info->baseSpAttack
            + (u16)info->baseSpDefense;

    return bst;
}

TEST("Randomizer hash is deterministic and keeps categories and keys separate")
{
    u32 hash = RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ENCOUNTER, 1, 2, 3);

    EXPECT_EQ(hash, RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ENCOUNTER, 1, 2, 3));
    EXPECT_NE(hash, RandomizerHash(0x87654321, RANDOMIZER_CATEGORY_ENCOUNTER, 1, 2, 3));
    EXPECT_NE(hash, RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ABILITY, 1, 2, 3));
    EXPECT_NE(hash, RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ENCOUNTER, 4, 2, 3));
    EXPECT_NE(hash, RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ENCOUNTER, 1, 4, 3));
    EXPECT_NE(hash, RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_ENCOUNTER, 1, 2, 4));
}

TEST("Randomizer hash does not advance either global RNG")
{
    rng_value_t rng = gRngValue;
    rng_value_t rng2 = gRng2Value;

    RandomizerHash(0x12345678, RANDOMIZER_CATEGORY_LEARNSET, 1, 2, 3);

    EXPECT_EQ(rng.a, gRngValue.a);
    EXPECT_EQ(rng.b, gRngValue.b);
    EXPECT_EQ(rng.c, gRngValue.c);
    EXPECT_EQ(rng.ctr, gRngValue.ctr);
    EXPECT_EQ(rng2.a, gRng2Value.a);
    EXPECT_EQ(rng2.b, gRng2Value.b);
    EXPECT_EQ(rng2.c, gRng2Value.c);
    EXPECT_EQ(rng2.ctr, gRng2Value.ctr);
}

TEST("Randomizer data initialization stores a versioned nonzero seed")
{
    gSaveBlock3Ptr->randomizerSeed = 0;
    gSaveBlock3Ptr->randomizerVersion = 0;

    InitRandomizerData();

    EXPECT_NE(gSaveBlock3Ptr->randomizerSeed, 0);
    EXPECT_EQ(GetRandomizerSeed(), gSaveBlock3Ptr->randomizerSeed);
    EXPECT_EQ(gSaveBlock3Ptr->randomizerVersion, RANDOMIZER_ALGORITHM_VERSION);
}

TEST("Species randomizer eligibility rejects invalid and battle-only species")
{
    EXPECT(IsSpeciesRandomizerEligible(SPECIES_BULBASAUR));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_NONE));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_EGG));
    EXPECT(!IsSpeciesRandomizerEligible(NUM_SPECIES));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_VENUSAUR_MEGA));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_GROUDON_PRIMAL));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_NECROZMA_ULTRA));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_CHARIZARD_GMAX));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_DARMANITAN_ZEN));
    EXPECT(!IsSpeciesRandomizerEligible(SPECIES_KYUREM_BLACK));
}

TEST("Encounter randomization is deterministic, eligible, and context separated")
{
    enum Species species;
    enum Species differentSeedSpecies;
    const u8 encounterDifficulty = 20;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0010, RANDOMIZER_ENCOUNTER_LAND, 3, encounterDifficulty);

    EXPECT(IsSpeciesRandomizerEligible(species));
    EXPECT_EQ(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0010, RANDOMIZER_ENCOUNTER_LAND, 3, encounterDifficulty));
    EXPECT_NE(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0011, RANDOMIZER_ENCOUNTER_LAND, 3, encounterDifficulty));
    EXPECT_NE(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0010, RANDOMIZER_ENCOUNTER_WATER, 3, encounterDifficulty));
    EXPECT_NE(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0010, RANDOMIZER_ENCOUNTER_LAND, 4, encounterDifficulty));

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    differentSeedSpecies = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0010, RANDOMIZER_ENCOUNTER_LAND, 3, encounterDifficulty);
    EXPECT_NE(species, differentSeedSpecies);
}

TEST("Encounter randomization BST stays within each difficulty's preferred band")
{
    enum Species species;
    u16 bst;
    const struct
    {
        u8 difficulty;
        u16 minBST;
        u16 maxBST;
    } cases[] =
    {
        { 10, 180, 360 },
        { 11, 240, 420 },
        { 20, 240, 420 },
        { 21, 300, 480 },
        { 30, 300, 480 },
        { 31, 360, 540 },
        { 40, 360, 540 },
        { 41, 420, 600 },
        { 50, 420, 600 },
        { 51, 480, 720 },
    };

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;

    for (u32 i = 0; i < ARRAY_COUNT(cases); i++)
    {
        species = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0020, RANDOMIZER_ENCOUNTER_LAND, 5, cases[i].difficulty);
        const struct SpeciesInfo *info = &gSpeciesInfo[species];

        EXPECT(IsSpeciesRandomizerEligible(species));
        EXPECT(!info->isRestrictedLegendary);
        EXPECT(!info->isSubLegendary);
        EXPECT(!info->isMythical);
        EXPECT(!info->isUltraBeast);
        EXPECT(!info->isParadox);
        bst = GetTestSpeciesBaseStatTotal(species);
        EXPECT_GE(bst, cases[i].minBST);
        EXPECT_LE(bst, cases[i].maxBST);
    }
}

TEST("Encounter randomization low difficulty BST stays under high-difficulty floor")
{
    enum Species lowDifficultySpecies;
    enum Species highDifficultySpecies;
    u16 lowBST;
    u16 highBST;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    lowDifficultySpecies = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0040, RANDOMIZER_ENCOUNTER_LAND, 1, 10);
    highDifficultySpecies = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0040, RANDOMIZER_ENCOUNTER_LAND, 1, 51);

    EXPECT(IsSpeciesRandomizerEligible(lowDifficultySpecies));
    EXPECT(IsSpeciesRandomizerEligible(highDifficultySpecies));

    lowBST = GetTestSpeciesBaseStatTotal(lowDifficultySpecies);
    highBST = GetTestSpeciesBaseStatTotal(highDifficultySpecies);

    EXPECT_LE(lowBST, 360);
    EXPECT_GE(highBST, 480);
}
