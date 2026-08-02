#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "pokemon.h"
#include "test/test.h"

static void ClearProgressionFlags(void)
{
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagClear(FLAG_IS_CHAMPION);
}

TEST("Level cap follows Emerald badge progression")
{
    static const u32 sExpectedCaps[] = {15, 19, 24, 29, 31, 33, 42, 46, 58, MAX_LEVEL};

    ClearProgressionFlags();
    EXPECT_EQ(GetCurrentLevelCap(), sExpectedCaps[0]);

    for (u32 badge = 0; badge < 8; badge++)
    {
        FlagSet(FLAG_BADGE01_GET + badge);
        EXPECT_EQ(GetCurrentLevelCap(), sExpectedCaps[badge + 1]);
    }

    FlagSet(FLAG_IS_CHAMPION);
    EXPECT_EQ(GetCurrentLevelCap(), sExpectedCaps[9]);
}

TEST("Hard level cap removes experience at the cap")
{
    ClearProgressionFlags();

    EXPECT_EQ(GetSoftLevelCapExpValue(14, 100), 100);
    EXPECT_EQ(GetSoftLevelCapExpValue(15, 100), 0);
    EXPECT_EQ(GetSoftLevelCapExpValue(16, 100), 0);
}

TEST("Level-to-cap stepping advances exactly one level and stops at the cap")
{
    struct Pokemon mon;
    u32 previousLevel;

    ClearProgressionFlags();
    CreateRandomMonWithIVs(&mon, SPECIES_PIKACHU, 5, 0);

    previousLevel = GetMonData(&mon, MON_DATA_LEVEL);
    EXPECT(TryAdvanceMonOneLevelToCap(&mon));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), previousLevel + 1);

    while (TryAdvanceMonOneLevelToCap(&mon))
        previousLevel = GetMonData(&mon, MON_DATA_LEVEL);

    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), GetCurrentLevelCap());
    EXPECT_EQ(previousLevel, GetCurrentLevelCap());
    EXPECT(!TryAdvanceMonOneLevelToCap(&mon));

    CreateRandomMonWithIVs(&mon, SPECIES_EGG, 5, 0);
    EXPECT(!TryAdvanceMonOneLevelToCap(&mon));
}

TEST("EV gain is disabled for battles and EV items")
{
    struct Pokemon mon;

    CreateRandomMonWithIVs(&mon, SPECIES_PIKACHU, 5, 0);

    MonGainEVs(&mon, SPECIES_CATERPIE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ATK_EV), 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DEF_EV), 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPEED_EV), 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPATK_EV), 0);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPDEF_EV), 0);

    PokemonUseItemEffects(&mon, ITEM_HP_UP, 0, 0, FALSE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), 0);
}
