#include "global.h"
#include "event_data.h"
#include "item.h"
#include "item_use.h"
#include "pokemon.h"
#include "test/test.h"
#include "wild_encounter.h"

TEST("Portable Healer and Repel Toggle are reusable Key Items")
{
    EXPECT_EQ(GetItemPocket(ITEM_PORTABLE_HEALER), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemImportance(ITEM_PORTABLE_HEALER), 1);
    EXPECT_EQ(GetItemFieldFunc(ITEM_PORTABLE_HEALER), ItemUseOutOfBattle_PortableHealer);

    EXPECT_EQ(GetItemPocket(ITEM_REPEL_TOGGLE), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemImportance(ITEM_REPEL_TOGGLE), 1);
    EXPECT_EQ(GetItemFieldFunc(ITEM_REPEL_TOGGLE), ItemUseOutOfBattle_RepelToggle);
}

TEST("Portable Healer restores HP and status for the whole party")
{
    u32 hp = 1;
    u32 status = STATUS1_POISON;

    gPartiesCount[B_TRAINER_PLAYER] = 2;
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_PIKACHU, 10, 0);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_EEVEE, 10, 0);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_STATUS, &status);
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP, &hp);
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_STATUS, &status);

    HealPlayerPartyWithPortableHealer();

    for (u32 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HP),
                  GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_MAX_HP));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_STATUS), 0);
    }
}

TEST("Portable Repel remains active until explicitly disabled")
{
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_PIKACHU, 10, 0);

    SetPortableRepelActive(FALSE);
    EXPECT(!IsPortableRepelActive());

    SetPortableRepelActive(TRUE);
    EXPECT(IsPortableRepelActive());
    EXPECT_EQ(VarGet(VAR_REPEL_STEP_COUNT), PORTABLE_REPEL_STEPS);
    EXPECT(!IsWildLevelAllowedByRepel(1));
    EXPECT(!IsWildLevelAllowedByRepel(10));
    EXPECT(!IsWildLevelAllowedByRepel(MAX_LEVEL));
    EXPECT(!UpdateRepelCounter());
    EXPECT(IsPortableRepelActive());

    SetPortableRepelActive(FALSE);
    EXPECT(!IsPortableRepelActive());
    EXPECT_EQ(VarGet(VAR_REPEL_STEP_COUNT), 0);
}
