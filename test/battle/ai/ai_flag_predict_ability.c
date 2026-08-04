#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

AI_SINGLE_BATTLE_TEST("AI treats an opposing randomized ability as unknown before it activates")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_WEIGH_ABILITY_PREDICTION);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_VOLT_ABSORB); }
        OPPONENT(SPECIES_LANTURN) { Moves(MOVE_THUNDERBOLT); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_THUNDERBOLT); }
    } THEN {
        EXPECT_EQ(gBattleHistory->abilities[B_POSITION_PLAYER_LEFT], ABILITY_VOLT_ABSORB);
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids an immunity after the randomized ability activates")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LIGHTNING_ROD); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_LANTURN) { Moves(MOVE_THUNDERBOLT, MOVE_TACKLE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_THUNDERBOLT); }
        TURN { EXPECT_MOVE(opponent, MOVE_TACKLE); }
    }
}
