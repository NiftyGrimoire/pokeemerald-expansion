#include "global.h"
struct AiLogicData;
#include "battle_ai_main.h"
#include "constants/battle_ai.h"
#include "constants/trainers.h"
#include "test/test.h"

TEST("Ordinary randomized trainers use competent AI without voluntary switching")
{
    u64 flags = ResolveGameplayTrainerAIFlags(TRAINER_CLASS_YOUNGSTER, AI_FLAG_CHECK_BAD_MOVE);

    EXPECT((flags & AI_FLAG_BASIC_TRAINER) == AI_FLAG_BASIC_TRAINER);
    EXPECT(flags & AI_FLAG_SMART_MON_CHOICES);
    EXPECT(flags & AI_FLAG_RANDOMIZE_SWITCHIN);
    EXPECT(!(flags & AI_FLAG_SMART_SWITCHING));
    EXPECT(!(flags & AI_FLAG_OMNISCIENT));
    EXPECT(!(flags & AI_FLAG_SMART_TERA));
}

TEST("Boss randomized trainers use smart switching without hidden knowledge or Tera")
{
    u64 flags = ResolveGameplayTrainerAIFlags(TRAINER_CLASS_LEADER, AI_FLAG_BASIC_TRAINER);

    EXPECT((flags & AI_FLAG_BASIC_TRAINER) == AI_FLAG_BASIC_TRAINER);
    EXPECT(flags & AI_FLAG_SMART_SWITCHING);
    EXPECT(flags & AI_FLAG_SMART_MON_CHOICES);
    EXPECT(flags & AI_FLAG_RANDOMIZE_SWITCHIN);
    EXPECT(flags & AI_FLAG_ASSUME_STAB);
    EXPECT(flags & AI_FLAG_ASSUME_STATUS_MOVES);
    EXPECT(flags & AI_FLAG_PP_STALL_PREVENTION);
    EXPECT(!(flags & AI_FLAG_OMNISCIENT));
    EXPECT(!(flags & AI_FLAG_SMART_TERA));
}

TEST("Trainer AI tiers retain only the reviewed authored Risky modifier")
{
    u64 flags = ResolveGameplayTrainerAIFlags(TRAINER_CLASS_ELITE_FOUR,
                                              AI_FLAG_RISKY | AI_FLAG_FORCE_SETUP_FIRST_TURN);

    EXPECT(flags & AI_FLAG_RISKY);
    EXPECT(!(flags & AI_FLAG_FORCE_SETUP_FIRST_TURN));
}
