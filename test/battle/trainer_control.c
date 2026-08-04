#include "global.h"
#include "test/test.h"
#include "battle.h"
#include "battle_main.h"
#include "data.h"
#include "malloc.h"
#include "random.h"
#include "randomizer.h"
#include "string_util.h"
#include "trainer_pools.h"
#include "constants/item.h"
#include "constants/abilities.h"
#include "constants/trainers.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"

TEST("Battle style is globally locked to Set")
{
    EXPECT(GetEffectiveBattleStyle() == OPTIONS_BATTLE_STYLE_SET);
}

TEST("CreateNPCTrainerPartyForTrainer generates customized Pokémon")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 3;
    u8 nickBuffer[20];
    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(IsMonShiny(&testParty[0]));
    EXPECT(!IsMonShiny(&testParty[1]));

    EXPECT(GetMonData(&testParty[0], MON_DATA_POKEBALL, 0) == BALL_MASTER);
    EXPECT(GetMonData(&testParty[1], MON_DATA_POKEBALL, 0) == BALL_POKE);

    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES, 0) == SPECIES_WOBBUFFET);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES, 0) == SPECIES_WOBBUFFET);

    EXPECT_EQ(GetMonAbility(&testParty[0]), ABILITY_SLOW_START);
    EXPECT_EQ(GetMonAbility(&testParty[1]), ABILITY_SLOW_START);
    EXPECT_EQ(GetMonAbility(&testParty[2]), ABILITY_SLOW_START);

    EXPECT(GetMonData(&testParty[0], MON_DATA_FRIENDSHIP, 0) == 42);
    EXPECT(GetMonData(&testParty[1], MON_DATA_FRIENDSHIP, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HELD_ITEM, 0) == ITEM_ASSAULT_VEST);
    EXPECT(GetMonData(&testParty[1], MON_DATA_HELD_ITEM, 0) == ITEM_NONE);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HP_IV, 0) == 25);
    EXPECT(GetMonData(&testParty[0], MON_DATA_ATK_IV, 0) == 26);
    EXPECT(GetMonData(&testParty[0], MON_DATA_DEF_IV, 0) == 27);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPEED_IV, 0) == 28);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPATK_IV, 0) == 29);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPDEF_IV, 0) == 30);

    EXPECT(GetMonData(&testParty[1], MON_DATA_HP_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_ATK_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_DEF_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPEED_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPATK_IV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPDEF_IV, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_HP_EV, 0) == 252);
    EXPECT(GetMonData(&testParty[0], MON_DATA_ATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[0], MON_DATA_DEF_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPEED_EV, 0) == 252);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPATK_EV, 0) == 4);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPDEF_EV, 0) == 0);

    EXPECT(GetMonData(&testParty[1], MON_DATA_HP_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_ATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_DEF_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPEED_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPATK_EV, 0) == 0);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPDEF_EV, 0) == 0);

    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL, 0) == 67);
    EXPECT(GetMonData(&testParty[1], MON_DATA_LEVEL, 0) == 5);

    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE1, 0) == MOVE_AIR_SLASH);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE2, 0) == MOVE_BARRIER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE3, 0) == MOVE_SOLAR_BEAM);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE4, 0) == MOVE_EXPLOSION);

    GetMonData(&testParty[0], MON_DATA_NICKNAME, nickBuffer);
    EXPECT(StringCompare(nickBuffer, COMPOUND_STRING("Bubbles")) == 0);

    GetMonData(&testParty[1], MON_DATA_NICKNAME, nickBuffer);
    EXPECT(StringCompare(nickBuffer, COMPOUND_STRING("Wobbuffet")) == 0);

    EXPECT(GetMonGender(&testParty[0]) == MON_FEMALE);
    EXPECT(GetNature(&testParty[0]) == NATURE_HASTY);
    EXPECT(GetNature(&testParty[1]) == NATURE_HARDY);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_DYNAMAX_LEVEL), 5);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_DYNAMAX_LEVEL), 10);

    Free(testParty);
}

TEST("Randomized enemy trainer creation preserves tuning and generates species-safe data")
{
    struct Pokemon *testParty = Alloc(PARTY_SIZE * sizeof(struct Pokemon));
    u32 trainerId = 3;
    const struct Trainer *trainer = GetTrainerStructFromId(trainerId);
    const struct TrainerMon *partyEntry = &trainer->party[0];
    enum Species species;

    gSaveBlock3Ptr->randomizerSeed = 0x12345678;
    species = GetRandomizedSpeciesForTrainer(partyEntry->species, trainerId, 0);
    EXPECT_NE(species, partyEntry->species);

    CreateRandomizedNPCTrainerPartyFromTrainer(testParty, trainer, TRUE, BATTLE_TYPE_TRAINER, trainerId);
    EXPECT_EQ(species, SPECIES_PORYGON);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE1), MOVE_MOONBLAST);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE2), MOVE_GUILLOTINE);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE3), MOVE_FIRST_IMPRESSION);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE4), MOVE_WEATHER_BALL);
    EXPECT_LT(GetMonData(&testParty[0], MON_DATA_ABILITY_NUM), NUM_ABILITY_SLOTS);
    EXPECT_EQ(GetMonAbility(&testParty[0]), ABILITY_MIRROR_ARMOR);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_LEVEL), partyEntry->lvl);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HELD_ITEM), partyEntry->heldItem);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_FRIENDSHIP), partyEntry->friendship);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_POKEBALL), partyEntry->ball);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HP_IV), partyEntry->iv & 31);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HP_EV), partyEntry->ev[0]);
    EXPECT(IsMonShiny(&testParty[0]));

    Free(testParty);
}

TEST("CreateNPCTrainerPartyForTrainer generates different personalities for different mons")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 3;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(testParty[0].box.personality != testParty[1].box.personality);
    Free(testParty);
}

TEST("ModifyPersonalityForNature can set any nature")
{
    u32 personality = 0, nature = 0, j = 0, k = 0;
    for (j = 0; j < 64; j++)
    {
        for (k = 0; k < NUM_NATURES; k++)
        {
            PARAMETRIZE { personality = Random32(); nature = k; }
        }
    }
    ModifyPersonalityForNature(&personality, nature);
    EXPECT_EQ(GetNatureFromPersonality(personality), nature);
}

TEST("Trainer Class Balls apply to the entire party")
{
    ASSUME(B_TRAINER_CLASS_POKE_BALLS >= GEN_8);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 j;
    u32 currTrainer = 14;
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    CreateNPCTrainerPartyFromTrainer(testParty, trainer, FALSE, BATTLE_TYPE_TRAINER);
    for(j = 0; j < 6; j++)
    {
        EXPECT(GetMonData(&testParty[j], MON_DATA_POKEBALL, 0) == gTrainerClasses[trainer->trainerClass].ball);
    }
    Free(testParty);
}

TEST("Difficulty default to Normal if the trainer doesn't have a member for the current difficulty")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 4;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_MEWTWO);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (EASY)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METAPOD);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 1);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (HARD)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARCEUS);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 99);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for enemy trainer if defined for the difficulty (NORMAL)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 5;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_MEWTWO);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 50);
    Free(testParty);
}

TEST("Difficulty default to Normal if the partner doesn't have a member for the current difficulty")
{
    SetCurrentDifficultyLevel(DIFFICULTY_TEST);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METANG);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 42);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (EASY)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_EASY);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METAPOD);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 1);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (HARD)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARCEUS);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 99);
    Free(testParty);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("Difficulty changes which party is used for partner if defined for the difficulty (NORMAL)")
{
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = TRAINER_PARTNER(1);
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_METANG);
    EXPECT(GetMonData(&testParty[0], MON_DATA_LEVEL) == 42);
    Free(testParty);
}

TEST("Trainer Party Pool generates a party from the trainer pool")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 6;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_EEVEE);
    Free(testParty);
}

TEST("Trainer Party Pool picks a random lead and a random ace if tags exist in the pool")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 7;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_ARON);    //  Lead
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WYNAUT);  //  Not Lead or Ace
    EXPECT(GetMonData(&testParty[2], MON_DATA_SPECIES) == SPECIES_EEVEE);   //  Ace
    Free(testParty);
}

TEST("Trainer Party Pool picks according to custom rules")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 8;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_TORKOAL);    //  Lead + Weather Setter
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_BULBASAUR);  //  Lead + Weather Abuser
    EXPECT(GetMonData(&testParty[2], MON_DATA_SPECIES) == SPECIES_EEVEE);      //  Anything else
    Free(testParty);
}

TEST("Trainer Party Pool uses standard party creation if pool is illegal")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 9;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WOBBUFFET);
    Free(testParty);
}

TEST("Trainer Party Pool can be pruned before picking")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 10;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_EEVEE);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    Free(testParty);
}

TEST("Trainer Party Pool can choose which functions to use for picking mons")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u32 currTrainer = 11;
    CreateNPCTrainerPartyFromTrainer(testParty, GetTrainerStructFromId(currTrainer), TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_SPECIES) == SPECIES_WYNAUT);
    EXPECT(GetMonData(&testParty[1], MON_DATA_SPECIES) == SPECIES_WOBBUFFET);
    Free(testParty);
}

TEST("trainerproc supports both Double Battle: Yes and Battle Type: Doubles")
{
    u32 currTrainer;
    PARAMETRIZE { currTrainer = 12; }
    PARAMETRIZE { currTrainer = 13; }
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    EXPECT(trainer->battleType == TRAINER_BATTLE_TYPE_DOUBLES);
}

TEST("CreateNPCTrainerPartyForTrainer generates default moves if no moves are specified")
{
    u32 currTrainer = 1;
    const struct Trainer *trainer = GetTrainerStructFromId(currTrainer);
    ASSUME(trainer->party[0].moves[0] == MOVE_NONE);
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    CreateNPCTrainerPartyFromTrainer(testParty, trainer, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(GetMonData(&testParty[0], MON_DATA_MOVE1) != MOVE_NONE);
    Free(testParty);
}
