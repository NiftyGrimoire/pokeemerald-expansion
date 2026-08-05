#ifndef GUARD_RANDOMIZER_H
#define GUARD_RANDOMIZER_H

enum RandomizerCategory
{
    RANDOMIZER_CATEGORY_ENCOUNTER = 1,
    RANDOMIZER_CATEGORY_ABILITY,
    RANDOMIZER_CATEGORY_LEARNSET,
    RANDOMIZER_CATEGORY_LEGENDARY_ENCOUNTER,
    RANDOMIZER_CATEGORY_STARTER,
    RANDOMIZER_CATEGORY_TRAINER,
    RANDOMIZER_CATEGORY_EVOLUTION, // Reserved to preserve category IDs from older saves.
    RANDOMIZER_CATEGORY_WORLD_ITEM,
    RANDOMIZER_CATEGORY_FREE_ITEM,
    RANDOMIZER_CATEGORY_TM,
};

enum RandomizerEncounterType
{
    RANDOMIZER_ENCOUNTER_LAND,
    RANDOMIZER_ENCOUNTER_WATER,
    RANDOMIZER_ENCOUNTER_ROCK_SMASH,
    RANDOMIZER_ENCOUNTER_FISHING,
    RANDOMIZER_ENCOUNTER_FEEBAS,
    RANDOMIZER_ENCOUNTER_MASS_OUTBREAK,
    RANDOMIZER_ENCOUNTER_SCRIPTED,
};

#define RANDOMIZER_LEVEL_UP_MOVE_COUNT 20

void InitRandomizerData(void);
u32 GetRandomizerSeed(void);
u32 RandomizerHash(u32 seed, enum RandomizerCategory category, u32 key1, u32 key2, u32 key3);
enum Species GetRandomizerEvolutionFamily(enum Species species);
u8 GetRandomizerFriendshipEvolutionLevel(enum Species species);
bool32 ShouldRandomizerIgnoreEvolutionCondition(enum Species species, u16 condition);
bool32 IsAbilityRandomizerEligible(enum Ability ability);
bool32 IsMoveRandomizerEligible(enum Move move);
u32 GetRandomizerMoveWeightForLevel(enum Species species, enum Move move, u8 level);
enum Ability GetRandomizedAbilityForSpecies(enum Species species, enum Ability originalAbility);
enum Move GetRandomizedLevelUpMove(enum Species species, u8 learnsetSlot, const u16 *excludedMoves, u8 excludedMoveCount, enum Move fallbackMove);
u8 GetRandomizerLevelUpMoveLevel(u8 learnsetSlot);
bool32 IsSpeciesRandomizerEligible(enum Species species);
bool32 IsSpeciesRandomizerLegendaryEncounterEligible(enum Species species);
bool32 IsSpeciesRandomizerStarterEligible(enum Species species);
enum Species GetRandomizedSpeciesForEncounter(enum Species originalSpecies, u16 mapId, enum RandomizerEncounterType encounterType, u8 slot, u8 encounterDifficulty);
enum Species GetRandomizedSpeciesForLegendaryEncounter(enum Species originalSpecies, u16 mapId, u8 slot);
enum Species GetRandomizedSpeciesForTrainer(enum Species originalSpecies, u16 trainerId, u8 partySlot);
enum Species GetRandomizedStarterSpecies(enum Species originalSpecies, u8 slot);
bool32 IsWorldItemRandomizerEligible(enum Item item);
enum Item GetRandomizedWorldItem(enum Item originalItem, u8 mapGroup, u8 mapNum, u8 objectId);
bool32 IsFreeItemRandomizerEligible(enum Item item);
enum Item GetRandomizedFreeItem(enum Item originalItem, u8 mapGroup, u8 mapNum, u8 contextId);

#endif // GUARD_RANDOMIZER_H
