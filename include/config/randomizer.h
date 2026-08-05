#ifndef GUARD_CONFIG_RANDOMIZER_H
#define GUARD_CONFIG_RANDOMIZER_H

// Master gate for the per-save gameplay randomizer.
#define RANDOMIZER_ENABLED TRUE

// Randomize species selected by supported wild encounter methods.
#define RANDOMIZER_ENCOUNTERS TRUE

// Randomize supported scripted Legendary and Paradox encounters.
#define RANDOMIZER_LEGENDARY_ENCOUNTERS TRUE

// Randomize the three Hoenn starter choices.
#define RANDOMIZER_STARTERS TRUE

// Guaranteed perfect IVs for special player encounters.
#define RANDOMIZER_STARTER_PERFECT_IV_COUNT 4
#define RANDOMIZER_LEGENDARY_PERFECT_IV_COUNT 3

// Randomize species selected by supported enemy trainer parties.
#define RANDOMIZER_TRAINERS TRUE

// Randomize the move taught by each TM slot per save.
#define RANDOMIZER_TMS TRUE

// Randomize ordinary visible overworld item-ball pickups.
#define RANDOMIZER_WORLD_ITEMS TRUE

// Randomize eligible direct NPC and story item gifts.
#define RANDOMIZER_FREE_ITEMS TRUE

// Give each ordinary evolution family one deterministic randomized ability.
#define RANDOMIZER_ABILITIES TRUE

// Replace impractical evolution conditions for the short-run format.
#define RANDOMIZER_EVOLUTIONS TRUE

// Allow every enabled Pokemon species to learn every configured TM.
#define RANDOMIZER_LEARNSETS TRUE

// Changing the hash algorithm or category meanings requires a new version.
#define RANDOMIZER_ALGORITHM_VERSION 7

#endif // GUARD_CONFIG_RANDOMIZER_H
