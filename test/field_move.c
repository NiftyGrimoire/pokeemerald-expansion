#include "global.h"
#include "event_data.h"
#include "field_move.h"
#include "pokemon.h"
#include "test/test.h"

static void SetFieldMoveTestParty(bool32 firstMonIsEgg)
{
    bool8 isEgg = TRUE;

    for (u32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);

    CreateMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_PIKACHU, 10, 0, OTID_STRUCT_PRESET(0));
    if (firstMonIsEgg)
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, &isEgg);
    CreateMon(&gParties[B_TRAINER_PLAYER][1], SPECIES_EEVEE, 10, 0, OTID_STRUCT_PRESET(0));
    gPartiesCount[B_TRAINER_PLAYER] = 2;
}

TEST("HM field actions require their badge but not a learned move")
{
    SetFieldMoveTestParty(FALSE);

    EXPECT_EQ(GetFieldMoveUser(FIELD_MOVE_CUT), PARTY_SIZE);
    FlagSet(FLAG_BADGE01_GET);
    EXPECT_EQ(GetFieldMoveUser(FIELD_MOVE_CUT), 0);
}

TEST("Badge-authorized HM field actions skip Eggs")
{
    SetFieldMoveTestParty(TRUE);
    FlagSet(FLAG_BADGE05_GET);

    EXPECT_EQ(GetFieldMoveUser(FIELD_MOVE_SURF), 1);
}

TEST("Non-HM field actions still require a Pokemon that knows the move")
{
    SetFieldMoveTestParty(FALSE);

    EXPECT_EQ(GetFieldMoveUser(FIELD_MOVE_DIG), PARTY_SIZE);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][1], MOVE_DIG, 0);
    EXPECT_EQ(GetFieldMoveUser(FIELD_MOVE_DIG), 1);
}
