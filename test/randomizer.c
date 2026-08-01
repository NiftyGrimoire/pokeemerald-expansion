#include "global.h"
#include "event_data.h"
#include "item.h"
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

static bool32 HasTestEvolutionMethod(enum Species species, enum Species targetSpecies, u16 method)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    for (u32 i = 0; evolutions != NULL && evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (evolutions[i].targetSpecies == targetSpecies && evolutions[i].method == method)
            return TRUE;
    }
    return FALSE;
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

TEST("Learnset randomizer lets every enabled Pokemon learn every TM")
{
    for (enum TMHMIndex tm = 1; tm <= NUM_TECHNICAL_MACHINES; tm++)
        EXPECT(CanLearnTeachableMove(SPECIES_MAGIKARP, GetTMHMMoveId(tm)));
}

TEST("Learnset randomizer does not broaden HM, tutor, egg, or invalid compatibility")
{
    EXPECT(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_SURF));
    EXPECT(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_SPLASH));
    EXPECT(!CanLearnTeachableMove(SPECIES_EGG, MOVE_THUNDERBOLT));
    EXPECT(!CanLearnTeachableMove(SPECIES_NONE, MOVE_THUNDERBOLT));
    EXPECT(!CanLearnTeachableMove(NUM_SPECIES, MOVE_THUNDERBOLT));
}

TEST("Level-up learnset randomization is deterministic, distinct, and follows cap levels")
{
    struct LevelUpMove firstLearnset[MAX_LEVEL_UP_MOVES + 1];
    const struct LevelUpMove *randomizedLearnset;
    u32 learnsetCount = 0;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    randomizedLearnset = GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR);
    while (learnsetCount < MAX_LEVEL_UP_MOVES && randomizedLearnset[learnsetCount].move != LEVEL_UP_MOVE_END)
    {
        EXPECT_EQ(randomizedLearnset[learnsetCount].level, GetRandomizerLevelUpMoveLevel(learnsetCount));
        EXPECT(IsMoveRandomizerEligible(randomizedLearnset[learnsetCount].move));
        for (u32 prior = 0; prior < learnsetCount; prior++)
            EXPECT_NE(randomizedLearnset[learnsetCount].move, randomizedLearnset[prior].move);
        firstLearnset[learnsetCount] = randomizedLearnset[learnsetCount];
        learnsetCount++;
    }
    EXPECT_EQ(randomizedLearnset[learnsetCount].move, LEVEL_UP_MOVE_END);
    EXPECT_EQ(learnsetCount, RANDOMIZER_LEVEL_UP_MOVE_COUNT);

    randomizedLearnset = GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR);
    for (u32 i = 0; i < learnsetCount; i++)
    {
        EXPECT_EQ(randomizedLearnset[i].move, firstLearnset[i].move);
        EXPECT_EQ(randomizedLearnset[i].level, firstLearnset[i].level);
    }
}

TEST("Standard learnset schedule front-loads moves within level-cap bands")
{
    static const u8 capBandStarts[] = {1, 15, 19, 24, 29, 31, 33, 42, 46, 58};
    static const u8 capBandEnds[] = {15, 19, 24, 29, 31, 33, 42, 46, 58, 100};
    static const u8 expectedMoveCounts[] = {3, 3, 2, 2, 1, 1, 1, 1, 1, 1};

    EXPECT_EQ(GetRandomizerLevelUpMoveLevel(0), 1);
    EXPECT_EQ(GetRandomizerLevelUpMoveLevel(1), 1);
    EXPECT_EQ(GetRandomizerLevelUpMoveLevel(2), 1);
    EXPECT_EQ(GetRandomizerLevelUpMoveLevel(3), 1);

    for (u32 band = 0; band < ARRAY_COUNT(capBandEnds); band++)
    {
        u32 moveCount = 0;

        for (u32 slot = 4; slot < RANDOMIZER_LEVEL_UP_MOVE_COUNT; slot++)
        {
            u8 level = GetRandomizerLevelUpMoveLevel(slot);

            if (level > capBandStarts[band] && level <= capBandEnds[band])
                moveCount++;
            else if (band == 0 && level > 1 && level <= capBandEnds[band])
                moveCount++;
        }
        EXPECT_EQ(moveCount, expectedMoveCounts[band]);
    }
    EXPECT_EQ(GetRandomizerLevelUpMoveLevel(RANDOMIZER_LEVEL_UP_MOVE_COUNT), 0);
}

TEST("Learnset weights favor appropriate damaging move power by level")
{
    u32 earlyEmber = GetRandomizerMoveWeightForLevel(SPECIES_CHARMANDER, MOVE_EMBER, 5);
    u32 earlyFireBlast = GetRandomizerMoveWeightForLevel(SPECIES_CHARMANDER, MOVE_FIRE_BLAST, 5);
    u32 lateEmber = GetRandomizerMoveWeightForLevel(SPECIES_CHARMANDER, MOVE_EMBER, 80);
    u32 lateFireBlast = GetRandomizerMoveWeightForLevel(SPECIES_CHARMANDER, MOVE_FIRE_BLAST, 80);

    EXPECT_GT(earlyEmber, earlyFireBlast);
    EXPECT_GT(lateFireBlast, lateEmber);
    EXPECT_GT(earlyFireBlast, 0);
    EXPECT_GT(lateEmber, 0);
}

TEST("Learnset weights shift status moves from basic to elite by level")
{
    u32 earlyGrowl = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_GROWL, 5);
    u32 earlyQuiverDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_QUIVER_DANCE, 5);
    u32 middleCalmMind = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_CALM_MIND, 24);
    u32 middleGrowl = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_GROWL, 24);
    u32 lateGrowl = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_GROWL, 80);
    u32 lateQuiverDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_QUIVER_DANCE, 80);

    EXPECT_GT(earlyGrowl, earlyQuiverDance);
    EXPECT_GT(middleCalmMind, middleGrowl);
    EXPECT_GT(lateQuiverDance, lateGrowl);
    EXPECT_EQ(earlyGrowl, 10);
    EXPECT_EQ(earlyQuiverDance, 3);
    EXPECT_EQ(middleGrowl, 2);
    EXPECT_EQ(middleCalmMind, 24);
    EXPECT_EQ(lateGrowl, 1);
    EXPECT_EQ(lateQuiverDance, 36);
}

TEST("Learnset weights treat weather-setting status moves as strong tier")
{
    u32 earlyGrowl = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_GROWL, 5);
    u32 earlyRainDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_RAIN_DANCE, 5);
    u32 middleGrowl = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_GROWL, 24);
    u32 middleRainDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_RAIN_DANCE, 24);
    u32 lateRainDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_RAIN_DANCE, 80);
    u32 lateQuiverDance = GetRandomizerMoveWeightForLevel(SPECIES_BULBASAUR, MOVE_QUIVER_DANCE, 80);

    EXPECT_GT(earlyGrowl, earlyRainDance);
    EXPECT_GT(middleRainDance, middleGrowl);
    EXPECT_GT(lateQuiverDance, lateRainDance);
    EXPECT_EQ(earlyRainDance, 6);
    EXPECT_EQ(middleRainDance, 24);
    EXPECT_EQ(lateRainDance, 10);
}

TEST("Level-up learnsets separate seed and species identity")
{
    enum Move bulbasaurMoves[RANDOMIZER_LEVEL_UP_MOVE_COUNT];
    bool32 speciesDiffers = FALSE;
    bool32 seedDiffers = FALSE;
    const struct LevelUpMove *learnset;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    learnset = GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR);
    for (u32 i = 0; i < RANDOMIZER_LEVEL_UP_MOVE_COUNT; i++)
        bulbasaurMoves[i] = learnset[i].move;

    learnset = GetSpeciesLevelUpLearnset(SPECIES_CHARMANDER);
    for (u32 i = 0; i < RANDOMIZER_LEVEL_UP_MOVE_COUNT; i++)
        speciesDiffers |= bulbasaurMoves[i] != learnset[i].move;
    EXPECT(speciesDiffers);

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    learnset = GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR);
    for (u32 i = 0; i < RANDOMIZER_LEVEL_UP_MOVE_COUNT; i++)
        seedDiffers |= bulbasaurMoves[i] != learnset[i].move;
    EXPECT(seedDiffers);
}

TEST("Level-up learnsets reject special-case moves and preserve invalid species behavior")
{
    EXPECT(!IsMoveRandomizerEligible(MOVE_NONE));
    EXPECT(!IsMoveRandomizerEligible(MOVE_TRANSFORM));
    EXPECT(!IsMoveRandomizerEligible(MOVE_SKETCH));
    EXPECT(!IsMoveRandomizerEligible(MOVE_DARK_VOID));
    EXPECT(!IsMoveRandomizerEligible(MOVE_HYPERSPACE_FURY));
    EXPECT(!IsMoveRandomizerEligible(MOVE_AURA_WHEEL));
    EXPECT(!IsMoveRandomizerEligible(MOVE_STRUGGLE));
    EXPECT_EQ(GetSpeciesLevelUpLearnset(SPECIES_NONE)[0].move, gSpeciesInfo[SPECIES_NONE].levelUpLearnset[0].move);
    EXPECT_EQ(GetSpeciesLevelUpLearnset(SPECIES_EGG)[0].move, gSpeciesInfo[SPECIES_EGG].levelUpLearnset[0].move);
}

TEST("Randomizer evolution families include linear and branched evolutions")
{
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_BULBASAUR), SPECIES_BULBASAUR);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_IVYSAUR), SPECIES_BULBASAUR);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_VENUSAUR), SPECIES_BULBASAUR);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_EEVEE), SPECIES_EEVEE);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_VAPOREON), SPECIES_EEVEE);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_JOLTEON), SPECIES_EEVEE);
    EXPECT_NE(GetRandomizerEvolutionFamily(SPECIES_VULPIX),
              GetRandomizerEvolutionFamily(SPECIES_VULPIX_ALOLA));
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_NONE), SPECIES_NONE);
    EXPECT_EQ(GetRandomizerEvolutionFamily(SPECIES_EGG), SPECIES_EGG);
    EXPECT_EQ(GetRandomizerEvolutionFamily(NUM_SPECIES), NUM_SPECIES);
}

TEST("Friendship evolutions use explicit short-run level tiers")
{
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_PICHU), 20);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_TOGEPI), 20);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_GOLBAT), 30);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_EEVEE), 30);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_CHANSEY), 40);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_TYPE_NULL), 40);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_BULBASAUR), 0);
    EXPECT_EQ(GetRandomizerFriendshipEvolutionLevel(SPECIES_NONE), 0);
}

TEST("Friendship evolution condition uses level instead of friendship")
{
    struct Pokemon mon;
    u8 friendship = 0;

    CreateMon(&mon, SPECIES_PICHU, 19, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, NULL, CHECK_EVO), SPECIES_NONE);

    CreateMon(&mon, SPECIES_PICHU, 20, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &friendship);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, NULL, CHECK_EVO), SPECIES_PIKACHU);
}

TEST("Evolution randomizer removes declared clock and region barriers")
{
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_RIOLU, IF_NOT_TIME));
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_GLIGAR, IF_TIME));
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_PIKACHU, IF_REGION));
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_DARTRIX, IF_NOT_REGION));
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_MILCERY, IF_TIME));
    EXPECT(!ShouldRandomizerIgnoreEvolutionCondition(SPECIES_BULBASAUR, IF_REGION));
}

TEST("Alternate evolution target selection is deterministic and seed separated")
{
    bool32 firstSeedMidday;
    bool32 foundDifferentSeed = FALSE;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    firstSeedMidday = IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_MIDDAY);
    EXPECT_NE(firstSeedMidday, IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_MIDNIGHT));
    EXPECT_EQ(firstSeedMidday, IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_MIDDAY));
    EXPECT(IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_DUSK));

    for (u32 seed = 1; seed <= 64; seed++)
    {
        gSaveBlock3Ptr->randomizerSeed = seed;
        if (firstSeedMidday != IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_MIDDAY))
        {
            foundDifferentSeed = TRUE;
            break;
        }
    }
    EXPECT(foundDifferentSeed);
}

TEST("Selected time branch evolves without consulting the clock")
{
    struct Pokemon mon;
    enum Species expectedTarget;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    expectedTarget = IsRandomizerEvolutionTargetSelected(SPECIES_ROCKRUFF, SPECIES_LYCANROC_MIDDAY)
                   ? SPECIES_LYCANROC_MIDDAY
                   : SPECIES_LYCANROC_MIDNIGHT;
    CreateMon(&mon, SPECIES_ROCKRUFF, 25, 0, OTID_STRUCT_PLAYER_ID);

    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_NORMAL, ITEM_NONE, NULL, NULL, CHECK_EVO), expectedTarget);
}

TEST("Selected regional stone branch evolves outside its authored region")
{
    struct Pokemon mon;
    enum Species expectedTarget;

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    expectedTarget = IsRandomizerEvolutionTargetSelected(SPECIES_PIKACHU, SPECIES_RAICHU)
                   ? SPECIES_RAICHU
                   : SPECIES_RAICHU_ALOLA;
    CreateMon(&mon, SPECIES_PIKACHU, 30, 0, OTID_STRUCT_PLAYER_ID);

    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_ITEM_CHECK, ITEM_THUNDER_STONE, NULL, NULL, CHECK_EVO), expectedTarget);
}

TEST("Former location evolutions use location-free stones only")
{
    static const struct
    {
        enum Species source;
        enum Species target;
    } cases[] =
    {
        {SPECIES_MAGNETON, SPECIES_MAGNEZONE},
        {SPECIES_NOSEPASS, SPECIES_PROBOPASS},
        {SPECIES_CHARJABUG, SPECIES_VIKAVOLT},
        {SPECIES_CRABRAWLER, SPECIES_CRABOMINABLE},
        {SPECIES_EEVEE, SPECIES_LEAFEON},
        {SPECIES_EEVEE, SPECIES_GLACEON},
    };

    for (u32 i = 0; i < ARRAY_COUNT(cases); i++)
    {
        EXPECT(HasTestEvolutionMethod(cases[i].source, cases[i].target, EVO_ITEM));
        EXPECT(!HasTestEvolutionMethod(cases[i].source, cases[i].target, EVO_LEVEL));
    }
}

TEST("Milcery selects one stable cream flavor for every Sweet")
{
    static const enum Species strawberryTargets[] =
    {
        SPECIES_ALCREMIE_STRAWBERRY_VANILLA_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_MATCHA_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_MINT_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_LEMON_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_SALTED_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_RUBY_SWIRL,
        SPECIES_ALCREMIE_STRAWBERRY_CARAMEL_SWIRL,
        SPECIES_ALCREMIE_STRAWBERRY_RAINBOW_SWIRL,
    };
    static const enum Species berryTargets[] =
    {
        SPECIES_ALCREMIE_BERRY_VANILLA_CREAM,
        SPECIES_ALCREMIE_BERRY_RUBY_CREAM,
        SPECIES_ALCREMIE_BERRY_MATCHA_CREAM,
        SPECIES_ALCREMIE_BERRY_MINT_CREAM,
        SPECIES_ALCREMIE_BERRY_LEMON_CREAM,
        SPECIES_ALCREMIE_BERRY_SALTED_CREAM,
        SPECIES_ALCREMIE_BERRY_RUBY_SWIRL,
        SPECIES_ALCREMIE_BERRY_CARAMEL_SWIRL,
        SPECIES_ALCREMIE_BERRY_RAINBOW_SWIRL,
    };
    u32 selectedFlavor = ARRAY_COUNT(strawberryTargets);
    u32 selectedCount = 0;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    for (u32 flavor = 0; flavor < ARRAY_COUNT(strawberryTargets); flavor++)
    {
        if (IsRandomizerEvolutionTargetSelected(SPECIES_MILCERY, strawberryTargets[flavor]))
        {
            selectedFlavor = flavor;
            selectedCount++;
        }
    }

    EXPECT_EQ(selectedCount, 1);
    EXPECT_LT(selectedFlavor, ARRAY_COUNT(berryTargets));
    EXPECT(IsRandomizerEvolutionTargetSelected(SPECIES_MILCERY, berryTargets[selectedFlavor]));
    EXPECT(ShouldRandomizerIgnoreEvolutionCondition(SPECIES_MILCERY, IF_TIME));
    EXPECT(ShouldRandomizerIgnoreEvolutionSpin(SPECIES_MILCERY));
    EXPECT(!ShouldRandomizerIgnoreEvolutionSpin(SPECIES_BULBASAUR));
}

TEST("Milcery evolves to its selected flavor with any spin and the held Sweet")
{
    static const enum Species strawberryTargets[] =
    {
        SPECIES_ALCREMIE_STRAWBERRY_VANILLA_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_MATCHA_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_MINT_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_LEMON_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_SALTED_CREAM,
        SPECIES_ALCREMIE_STRAWBERRY_RUBY_SWIRL,
        SPECIES_ALCREMIE_STRAWBERRY_CARAMEL_SWIRL,
        SPECIES_ALCREMIE_STRAWBERRY_RAINBOW_SWIRL,
    };
    struct Pokemon mon;
    enum Item heldItem = ITEM_STRAWBERRY_SWEET;
    enum Species expectedTarget = SPECIES_NONE;

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    for (u32 flavor = 0; flavor < ARRAY_COUNT(strawberryTargets); flavor++)
    {
        if (IsRandomizerEvolutionTargetSelected(SPECIES_MILCERY, strawberryTargets[flavor]))
            expectedTarget = strawberryTargets[flavor];
    }
    CreateMon(&mon, SPECIES_MILCERY, 20, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);
    gSpecialVar_0x8000 = 0;

    EXPECT_NE(expectedTarget, SPECIES_NONE);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_OVERWORLD_SPECIAL, ITEM_NONE, NULL, NULL, CHECK_EVO), expectedTarget);
}

TEST("Ability randomizer excludes unsafe and unimplemented abilities")
{
    EXPECT(IsAbilityRandomizerEligible(ABILITY_OVERGROW));
    EXPECT(IsAbilityRandomizerEligible(ABILITY_INTIMIDATE));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_NONE));
    EXPECT(!IsAbilityRandomizerEligible(ABILITIES_COUNT));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_WONDER_GUARD));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_STANCE_CHANGE));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_SCHOOLING));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_DISGUISE));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_ZERO_TO_HERO));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_TERA_SHIFT));
    EXPECT(!IsAbilityRandomizerEligible(ABILITY_EELEVATE));
}

TEST("Ability randomizer is deterministic and stable across evolution families")
{
    enum Ability ability;
    enum Ability differentSeedAbility;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    ability = GetRandomizedAbilityForSpecies(SPECIES_BULBASAUR, ABILITY_OVERGROW);

    EXPECT(IsAbilityRandomizerEligible(ability));
    EXPECT_EQ(ability, GetRandomizedAbilityForSpecies(SPECIES_BULBASAUR, ABILITY_OVERGROW));
    EXPECT_EQ(ability, GetRandomizedAbilityForSpecies(SPECIES_IVYSAUR, ABILITY_OVERGROW));
    EXPECT_EQ(ability, GetRandomizedAbilityForSpecies(SPECIES_VENUSAUR, ABILITY_OVERGROW));
    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_VAPOREON, ABILITY_WATER_ABSORB),
              GetRandomizedAbilityForSpecies(SPECIES_JOLTEON, ABILITY_VOLT_ABSORB));

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    differentSeedAbility = GetRandomizedAbilityForSpecies(SPECIES_BULBASAUR, ABILITY_OVERGROW);
    EXPECT_NE(ability, differentSeedAbility);
}

TEST("Ability randomizer preserves protected families and invalid species")
{
    gSaveBlock3Ptr->randomizerSeed = 0x12345678;

    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_CASTFORM, ABILITY_FORECAST), ABILITY_FORECAST);
    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_CHERRIM, ABILITY_FLOWER_GIFT), ABILITY_FLOWER_GIFT);
    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_CHERUBI, ABILITY_CHLOROPHYLL), ABILITY_CHLOROPHYLL);
    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_NONE, ABILITY_NONE), ABILITY_NONE);
    EXPECT_EQ(GetRandomizedAbilityForSpecies(SPECIES_EGG, ABILITY_NONE), ABILITY_NONE);
    EXPECT_EQ(GetRandomizedAbilityForSpecies(NUM_SPECIES, ABILITY_OVERGROW), ABILITY_OVERGROW);
}

TEST("GetAbilityBySpecies exposes the randomized family ability")
{
    enum Ability ability;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    ability = GetAbilityBySpecies(SPECIES_BULBASAUR, 0);

    EXPECT(IsAbilityRandomizerEligible(ability));
    EXPECT_EQ(ability, GetAbilityBySpecies(SPECIES_BULBASAUR, 2));
    EXPECT_EQ(ability, GetAbilityBySpecies(SPECIES_IVYSAUR, 0));
    EXPECT_EQ(ABILITY_FORECAST, GetAbilityBySpecies(SPECIES_CASTFORM, 0));
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

TEST("Scripted ordinary encounter uses level band and distinct deterministic context")
{
    enum Species species;
    const struct SpeciesInfo *info;
    u16 bst;
    const u8 level = 20;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0030, RANDOMIZER_ENCOUNTER_SCRIPTED, 0, level);
    info = &gSpeciesInfo[species];
    bst = GetTestSpeciesBaseStatTotal(species);

    EXPECT(IsSpeciesRandomizerEligible(species));
    EXPECT(!info->isRestrictedLegendary);
    EXPECT(!info->isSubLegendary);
    EXPECT(!info->isMythical);
    EXPECT(!info->isUltraBeast);
    EXPECT(!info->isParadox);
    EXPECT_GE(bst, 240);
    EXPECT_LE(bst, 420);
    EXPECT_EQ(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0030, RANDOMIZER_ENCOUNTER_SCRIPTED, 0, level));
    EXPECT_NE(species, GetRandomizedSpeciesForEncounter(SPECIES_ZIGZAGOON, 0x0030, RANDOMIZER_ENCOUNTER_LAND, 0, level));
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

TEST("Legendary encounter eligibility includes restricted, sub, and Paradox species")
{
    EXPECT(gSpeciesInfo[SPECIES_MEWTWO].isRestrictedLegendary);
    EXPECT(gSpeciesInfo[SPECIES_ARTICUNO].isSubLegendary);
    EXPECT(gSpeciesInfo[SPECIES_GREAT_TUSK].isParadox);

    EXPECT(IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_MEWTWO));
    EXPECT(IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_ARTICUNO));
    EXPECT(IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_GREAT_TUSK));
}

TEST("Legendary encounter eligibility excludes Mythical, Ultra Beast, battle-only, and ordinary species")
{
    EXPECT(gSpeciesInfo[SPECIES_ARCEUS].isMythical);
    EXPECT(gSpeciesInfo[SPECIES_NIHILEGO].isUltraBeast);
    EXPECT(gSpeciesInfo[SPECIES_ZIGZAGOON].isRestrictedLegendary == FALSE);
    EXPECT(gSpeciesInfo[SPECIES_ZIGZAGOON].isSubLegendary == FALSE);
    EXPECT(gSpeciesInfo[SPECIES_ZIGZAGOON].isMythical == FALSE);
    EXPECT(gSpeciesInfo[SPECIES_ZIGZAGOON].isUltraBeast == FALSE);
    EXPECT(gSpeciesInfo[SPECIES_ZIGZAGOON].isParadox == FALSE);

    EXPECT(!IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_ARCEUS));
    EXPECT(!IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_NIHILEGO));
    EXPECT(!IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_ZIGZAGOON));
    EXPECT(!IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_NONE));
    EXPECT(!IsSpeciesRandomizerLegendaryEncounterEligible(SPECIES_MEWTWO_MEGA_X));
}

TEST("Legendary encounter resolver is deterministic for eligible original species")
{
    enum Species species;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0050, 0);

    EXPECT(IsSpeciesRandomizerLegendaryEncounterEligible(species));
    EXPECT_EQ(species, GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0050, 0));
}

TEST("Legendary encounter resolver separates mapId, originalSpecies, slot, and seed context")
{
    enum Species species;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0060, 0);

    EXPECT_NE(species, GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0061, 0));
    EXPECT_NE(species, GetRandomizedSpeciesForLegendaryEncounter(SPECIES_RAYQUAZA, 0x0060, 0));
    EXPECT_NE(species, GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0060, 1));

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    EXPECT_NE(species, GetRandomizedSpeciesForLegendaryEncounter(SPECIES_MEWTWO, 0x0060, 0));
}

TEST("Legendary encounter resolver leaves ordinary and SPECIES_NONE originals unchanged")
{
    gSaveBlock3Ptr->randomizerSeed = 0x12345678;

    EXPECT_EQ(GetRandomizedSpeciesForLegendaryEncounter(SPECIES_ZIGZAGOON, 0x0070, 0), SPECIES_ZIGZAGOON);
    EXPECT_EQ(GetRandomizedSpeciesForLegendaryEncounter(SPECIES_NONE, 0x0070, 0), SPECIES_NONE);
    EXPECT_EQ(GetRandomizedSpeciesForLegendaryEncounter(SPECIES_NONE, 0x0070, 1), SPECIES_NONE);
}

TEST("Starter eligibility requires a balanced base species in a three-stage line")
{
    EXPECT(IsSpeciesRandomizerStarterEligible(SPECIES_BULBASAUR));
    EXPECT(IsSpeciesRandomizerStarterEligible(SPECIES_TREECKO));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_IVYSAUR));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_VENUSAUR));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_CATERPIE));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_EEVEE));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_MEWTWO));
    EXPECT(!IsSpeciesRandomizerStarterEligible(SPECIES_NONE));
}

TEST("Starter resolver produces three distinct deterministic eligible choices")
{
    static const enum Species sOriginalStarters[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
    enum Species starters[3];

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    for (u32 slot = 0; slot < ARRAY_COUNT(starters); slot++)
    {
        starters[slot] = GetRandomizedStarterSpecies(sOriginalStarters[slot], slot);
        EXPECT(IsSpeciesRandomizerStarterEligible(starters[slot]));
        EXPECT_GE(GetTestSpeciesBaseStatTotal(starters[slot]), 300);
        EXPECT_LE(GetTestSpeciesBaseStatTotal(starters[slot]), 350);
        EXPECT_EQ(starters[slot], GetRandomizedStarterSpecies(sOriginalStarters[slot], slot));
    }

    EXPECT_NE(starters[0], starters[1]);
    EXPECT_NE(starters[0], starters[2]);
    EXPECT_NE(starters[1], starters[2]);
}

TEST("Starter resolver separates saves and preserves invalid slots")
{
    enum Species firstSeedStarter;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    firstSeedStarter = GetRandomizedStarterSpecies(SPECIES_TREECKO, 0);

    gSaveBlock3Ptr->randomizerSeed = 0x87654321;
    EXPECT_NE(firstSeedStarter, GetRandomizedStarterSpecies(SPECIES_TREECKO, 0));
    EXPECT_EQ(GetRandomizedStarterSpecies(SPECIES_TREECKO, 3), SPECIES_TREECKO);
}

TEST("Trainer randomization is deterministic, eligible, common, and within BST band")
{
    enum Species species;
    const struct SpeciesInfo *info;
    u16 bst;
    u16 originalBST;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, 0x0100, 0);

    EXPECT_EQ(species, GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, 0x0100, 0));
    EXPECT(IsSpeciesRandomizerEligible(species));

    info = &gSpeciesInfo[species];
    EXPECT(!info->isRestrictedLegendary);
    EXPECT(!info->isSubLegendary);
    EXPECT(!info->isMythical);
    EXPECT(!info->isUltraBeast);
    EXPECT(!info->isParadox);

    originalBST = GetTestSpeciesBaseStatTotal(SPECIES_BRELOOM);
    bst = GetTestSpeciesBaseStatTotal(species);
    EXPECT_GE(bst, (originalBST > 50) ? (u16)(originalBST - 50) : 0);
    EXPECT_LE(bst, (u16)originalBST + 50);
}

TEST("Trainer randomization separates seed, trainerId, and partySlot context")
{
    enum Species baseline;
    bool32 seedDiffers = FALSE;
    bool32 trainerDiffers = FALSE;
    bool32 slotDiffers = FALSE;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    baseline = GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, 0x0100, 0);

    for (u32 seed = 0x12345679; seed <= 0x12345688; seed++)
    {
        gSaveBlock3Ptr->randomizerSeed = seed;
        if (GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, 0x0100, 0) != baseline)
            seedDiffers = TRUE;
    }

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    for (u32 trainerId = 0x0101; trainerId <= 0x0110; trainerId++)
    {
        if (GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, trainerId, 0) != baseline)
            trainerDiffers = TRUE;
    }

    for (u32 slot = 1; slot < PARTY_SIZE; slot++)
    {
        if (GetRandomizedSpeciesForTrainer(SPECIES_BRELOOM, 0x0100, slot) != baseline)
            slotDiffers = TRUE;
    }

    EXPECT(seedDiffers);
    EXPECT(trainerDiffers);
    EXPECT(slotDiffers);
}

TEST("Trainer randomization returns SPECIES_NONE and NUM_SPECIES unchanged")
{
    gSaveBlock3Ptr->randomizerSeed = 0x12345678;

    EXPECT_EQ(GetRandomizedSpeciesForTrainer(SPECIES_NONE, 0x0100, 0), SPECIES_NONE);
    EXPECT_EQ(GetRandomizedSpeciesForTrainer(NUM_SPECIES, 0x0100, 0), NUM_SPECIES);
}
