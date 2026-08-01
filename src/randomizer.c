#include "global.h"
#include "move.h"
#include "pokemon.h"
#include "random.h"
#include "random_mon_generation.h"
#include "randomizer.h"

#define RANDOMIZER_ZERO_SEED_FALLBACK 0x6D2B79F5
#define RANDOMIZER_STARTER_COUNT 3
#define RANDOMIZER_STARTER_MIN_BST 300
#define RANDOMIZER_STARTER_MAX_BST 350
#define RANDOMIZER_MOVE_TYPE_WEIGHT_STAB 3
#define RANDOMIZER_MOVE_TYPE_WEIGHT_COVERAGE 2
#define RANDOMIZER_MOVE_TYPE_WEIGHT_STATUS 1
#define RANDOMIZER_MOVE_POWER_WEIGHT_MAX 12

static const u8 sRandomizerLevelUpMoveLevels[RANDOMIZER_LEVEL_UP_MOVE_COUNT] =
{
    1, 1, 1, 1,
    5, 10, 15,
    16, 18, 19,
    22, 24,
    27, 29,
    31,
    33,
    42,
    46,
    58,
    80,
};

static EWRAM_DATA u16 sEvolutionFamilyCache[NUM_SPECIES] = {0};
static EWRAM_DATA bool8 sEvolutionFamilyHasProtectedAbility[NUM_SPECIES] = {0};
static EWRAM_DATA u16 sEvolutionFamilyAbilityCache[NUM_SPECIES] = {0};
static EWRAM_DATA u32 sEvolutionFamilyAbilityCacheSeed = 0;
static EWRAM_DATA bool8 sEvolutionFamilyCacheInitialized = FALSE;

static u32 MixRandomizerValue(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    value ^= value >> 16;
    return value;
}

void InitRandomizerData(void)
{
#if RANDOMIZER_ENABLED
    u32 seed = Random32();

    if (seed == 0)
        seed = RANDOMIZER_ZERO_SEED_FALLBACK;

    gSaveBlock3Ptr->randomizerSeed = seed;
    gSaveBlock3Ptr->randomizerVersion = RANDOMIZER_ALGORITHM_VERSION;
#endif
}

u32 GetRandomizerSeed(void)
{
#if RANDOMIZER_ENABLED
    return gSaveBlock3Ptr->randomizerSeed;
#else
    return 0;
#endif
}

u32 RandomizerHash(u32 seed, enum RandomizerCategory category, u32 key1, u32 key2, u32 key3)
{
    u32 hash = MixRandomizerValue(seed ^ 0x9E3779B9);

    hash = MixRandomizerValue(hash ^ (u32)category);
    hash = MixRandomizerValue(hash ^ key1);
    hash = MixRandomizerValue(hash ^ key2);
    hash = MixRandomizerValue(hash ^ key3);
    return hash;
}

bool32 IsMoveRandomizerEligible(enum Move move)
{
    if (move <= MOVE_NONE || move >= MOVES_COUNT)
        return FALSE;

    switch (GetMoveEffect(move))
    {
    case EFFECT_PLACEHOLDER:
    case EFFECT_TRANSFORM:
    case EFFECT_SKETCH:
    case EFFECT_DARK_VOID:
    case EFFECT_SPECIES_POWER_OVERRIDE:
    case EFFECT_HYPERSPACE_FURY:
    case EFFECT_AURA_WHEEL:
        return FALSE;
    default:
        break;
    }

    return move != MOVE_STRUGGLE;
}

static bool32 IsExcludedRandomizerMove(enum Move move, const u16 *excludedMoves, u8 excludedMoveCount)
{
    for (u32 i = 0; i < excludedMoveCount; i++)
    {
        if (excludedMoves[i] == move)
            return TRUE;
    }
    return FALSE;
}

static u32 GetRandomizerStatusMoveTier(enum Move move)
{
    switch (move)
    {
    case MOVE_SPORE:
    case MOVE_BELLY_DRUM:
    case MOVE_SHELL_SMASH:
    case MOVE_GEOMANCY:
    case MOVE_QUIVER_DANCE:
    case MOVE_TAIL_GLOW:
    case MOVE_SHIFT_GEAR:
    case MOVE_NO_RETREAT:
    case MOVE_VICTORY_DANCE:
    case MOVE_TIDY_UP:
    case MOVE_REVIVAL_BLESSING:
    case MOVE_SHED_TAIL:
        return 2;
    case MOVE_SWORDS_DANCE:
    case MOVE_NASTY_PLOT:
    case MOVE_DRAGON_DANCE:
    case MOVE_CALM_MIND:
    case MOVE_BULK_UP:
    case MOVE_AGILITY:
    case MOVE_RECOVER:
    case MOVE_ROOST:
    case MOVE_SOFT_BOILED:
    case MOVE_SLACK_OFF:
    case MOVE_MILK_DRINK:
    case MOVE_MOONLIGHT:
    case MOVE_MORNING_SUN:
    case MOVE_SYNTHESIS:
    case MOVE_WILL_O_WISP:
    case MOVE_THUNDER_WAVE:
    case MOVE_TOXIC:
    case MOVE_SLEEP_POWDER:
    case MOVE_HYPNOSIS:
    case MOVE_GLARE:
    case MOVE_STICKY_WEB:
    case MOVE_STEALTH_ROCK:
    case MOVE_SPIKES:
    case MOVE_TOXIC_SPIKES:
    case MOVE_REFLECT:
    case MOVE_LIGHT_SCREEN:
    case MOVE_AURORA_VEIL:
    case MOVE_TRICK_ROOM:
        return 1;
    default:
        if (GetMoveEffect(move) == EFFECT_WEATHER || GetMoveEffect(move) == EFFECT_WEATHER_AND_SWITCH)
            return 1;
        return 0;
    }
}

static u32 GetRandomizerStatusMoveWeight(enum Move move, u8 level)
{
    u32 moveTier = GetRandomizerStatusMoveTier(move);
    u32 targetTier = (level < 24 ? 0 : level < 42 ? 1 : 2);
    u32 tierDistance = (moveTier > targetTier ? moveTier - targetTier : targetTier - moveTier);

    if (tierDistance == 0)
        return 10;
    if (tierDistance == 1)
        return 3;
    return 1;
}

u32 GetRandomizerMoveWeightForLevel(enum Species species, enum Move move, u8 level)
{
    u32 typeWeight;

    if (!IsMoveRandomizerEligible(move) || species <= SPECIES_NONE || species >= NUM_SPECIES)
        return 0;

    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        typeWeight = RANDOMIZER_MOVE_TYPE_WEIGHT_STATUS;
    else if (GetMoveType(move) == GetSpeciesType(species, 0) || GetMoveType(move) == GetSpeciesType(species, 1))
        typeWeight = RANDOMIZER_MOVE_TYPE_WEIGHT_STAB;
    else
        typeWeight = RANDOMIZER_MOVE_TYPE_WEIGHT_COVERAGE;

    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return typeWeight * GetRandomizerStatusMoveWeight(move, level);
    else
    {
        u32 power = GetMovePower(move);
        u32 targetPower = min(35 + level, 120);
        u32 powerDistance;
        u32 powerWeight;

        // Fixed and level-based damage moves have no listed base power.
        if (power == 0)
            power = 50;
        powerDistance = (power > targetPower ? power - targetPower : targetPower - power);
        powerWeight = max(1, RANDOMIZER_MOVE_POWER_WEIGHT_MAX - powerDistance / 10);
        return typeWeight * powerWeight;
    }
}

enum Move GetRandomizedLevelUpMove(enum Species species, u8 learnsetSlot, const u16 *excludedMoves, u8 excludedMoveCount, enum Move fallbackMove)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_LEARNSETS
    u32 totalWeight = 0;
    u32 selection;
    u8 learnLevel = GetRandomizerLevelUpMoveLevel(learnsetSlot);

    if (species <= SPECIES_NONE || species >= NUM_SPECIES || !IsSpeciesEnabled(species))
        return fallbackMove;

    for (enum Move move = MOVE_NONE + 1; move < MOVES_COUNT; move++)
    {
        if (IsMoveRandomizerEligible(move) && !IsExcludedRandomizerMove(move, excludedMoves, excludedMoveCount))
            totalWeight += GetRandomizerMoveWeightForLevel(species, move, learnLevel);
    }

    if (totalWeight == 0)
        return fallbackMove;

    selection = RandomizerHash(GetRandomizerSeed(), RANDOMIZER_CATEGORY_LEARNSET, species, learnsetSlot, 0) % totalWeight;
    for (enum Move move = MOVE_NONE + 1; move < MOVES_COUNT; move++)
    {
        u32 weight;

        if (!IsMoveRandomizerEligible(move) || IsExcludedRandomizerMove(move, excludedMoves, excludedMoveCount))
            continue;

        weight = GetRandomizerMoveWeightForLevel(species, move, learnLevel);
        if (selection < weight)
            return move;
        selection -= weight;
    }
#endif
    return fallbackMove;
}

u8 GetRandomizerLevelUpMoveLevel(u8 learnsetSlot)
{
    if (learnsetSlot >= RANDOMIZER_LEVEL_UP_MOVE_COUNT)
        return 0;
    return sRandomizerLevelUpMoveLevels[learnsetSlot];
}

static enum Species FindRandomizerEvolutionFamily(enum Species species)
{
    enum Species parent = sEvolutionFamilyCache[species];

    if (parent != species)
        sEvolutionFamilyCache[species] = FindRandomizerEvolutionFamily(parent);

    return sEvolutionFamilyCache[species];
}

static bool32 IsProtectedRandomizerAbility(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_WONDER_GUARD:
    case ABILITY_FORECAST:
    case ABILITY_FLOWER_GIFT:
    case ABILITY_MULTITYPE:
    case ABILITY_ZEN_MODE:
    case ABILITY_STANCE_CHANGE:
    case ABILITY_BATTLE_BOND:
    case ABILITY_POWER_CONSTRUCT:
    case ABILITY_SCHOOLING:
    case ABILITY_RKS_SYSTEM:
    case ABILITY_SHIELDS_DOWN:
    case ABILITY_DISGUISE:
    case ABILITY_GULP_MISSILE:
    case ABILITY_ICE_FACE:
    case ABILITY_HUNGER_SWITCH:
    case ABILITY_ZERO_TO_HERO:
    case ABILITY_COMMANDER:
    case ABILITY_EMBODY_ASPECT_TEAL_MASK:
    case ABILITY_EMBODY_ASPECT_HEARTHFLAME_MASK:
    case ABILITY_EMBODY_ASPECT_WELLSPRING_MASK:
    case ABILITY_EMBODY_ASPECT_CORNERSTONE_MASK:
    case ABILITY_TERA_SHIFT:
    case ABILITY_TERA_SHELL:
    case ABILITY_TERAFORM_ZERO:
    case ABILITY_EELEVATE:
    case ABILITY_314:
    case ABILITY_FIRE_MANE:
    case ABILITY_317:
        return TRUE;
    default:
        return FALSE;
    }
}

bool32 IsAbilityRandomizerEligible(enum Ability ability)
{
    return ability > ABILITY_NONE
        && ability < ABILITIES_COUNT
        && !IsProtectedRandomizerAbility(ability);
}

static bool32 IsRandomizerEvolutionFamilyEdge(const struct Evolution *evolution)
{
    enum Species target = evolution->targetSpecies;

    return evolution->method != EVO_SPLIT_FROM_EVO
        && target > SPECIES_NONE
        && target < NUM_SPECIES
        && IsSpeciesEnabled(target);
}

static void InitRandomizerEvolutionFamilyCache(void)
{
    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesEnabled(species))
            sEvolutionFamilyCache[species] = species;
    }

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        const struct Evolution *evolutions;

        if (!IsSpeciesEnabled(species))
            continue;

        evolutions = GetSpeciesEvolutions(species);
        if (evolutions == NULL)
            continue;

        for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            enum Species target = evolutions[i].targetSpecies;

            if (IsRandomizerEvolutionFamilyEdge(&evolutions[i]))
            {
                enum Species sourceFamily = FindRandomizerEvolutionFamily(species);
                enum Species targetFamily = FindRandomizerEvolutionFamily(target);
                enum Species lowerFamily = min(sourceFamily, targetFamily);
                enum Species higherFamily = max(sourceFamily, targetFamily);

                sEvolutionFamilyCache[higherFamily] = lowerFamily;
            }
        }
    }

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesEnabled(species))
            sEvolutionFamilyCache[species] = FindRandomizerEvolutionFamily(species);
    }

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        enum Species family;

        if (!IsSpeciesEnabled(species))
            continue;

        family = sEvolutionFamilyCache[species];
        for (u32 abilityNum = 0; abilityNum < NUM_ABILITY_SLOTS; abilityNum++)
        {
            if (IsProtectedRandomizerAbility(GetSpeciesAbility(species, abilityNum)))
            {
                sEvolutionFamilyHasProtectedAbility[family] = TRUE;
                break;
            }
        }
    }

    sEvolutionFamilyCacheInitialized = TRUE;
}

enum Species GetRandomizerEvolutionFamily(enum Species species)
{
    if (species <= SPECIES_NONE || species >= NUM_SPECIES || species == SPECIES_EGG)
        return species;
    if (!IsSpeciesEnabled(species))
        return species;

    if (!sEvolutionFamilyCacheInitialized)
        InitRandomizerEvolutionFamilyCache();

    return sEvolutionFamilyCache[species];
}

enum Ability GetRandomizedAbilityForSpecies(enum Species species, enum Ability originalAbility)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_ABILITIES
    enum Species family;
    u32 seed;
    u32 abilityCount = 0;
    u32 selectedIndex;

    if (!IsSpeciesRandomizerEligible(species))
        return originalAbility;

    family = GetRandomizerEvolutionFamily(species);
    if (family == species && !IsSpeciesEnabled(family))
        return originalAbility;
    if (sEvolutionFamilyHasProtectedAbility[family])
        return originalAbility;

    seed = GetRandomizerSeed();
    if (sEvolutionFamilyAbilityCacheSeed != seed)
    {
        memset(sEvolutionFamilyAbilityCache, 0, sizeof(sEvolutionFamilyAbilityCache));
        sEvolutionFamilyAbilityCacheSeed = seed;
    }
    if (sEvolutionFamilyAbilityCache[family] != ABILITY_NONE)
        return sEvolutionFamilyAbilityCache[family];

    for (enum Ability ability = ABILITY_NONE + 1; ability < ABILITIES_COUNT; ability++)
    {
        if (IsAbilityRandomizerEligible(ability))
            abilityCount++;
    }

    if (abilityCount == 0)
        return originalAbility;

    selectedIndex = RandomizerHash(seed,
                                   RANDOMIZER_CATEGORY_ABILITY,
                                   family,
                                   RANDOMIZER_ALGORITHM_VERSION,
                                   0)
                  % abilityCount;

    for (enum Ability ability = ABILITY_NONE + 1; ability < ABILITIES_COUNT; ability++)
    {
        if (IsAbilityRandomizerEligible(ability) && selectedIndex-- == 0)
        {
            sEvolutionFamilyAbilityCache[family] = ability;
            return sEvolutionFamilyAbilityCache[family];
        }
    }
#endif

    return originalAbility;
}

bool32 IsSpeciesRandomizerEligible(enum Species species)
{
    const struct SpeciesInfo *speciesInfo;
    const u16 *formTable;

    if (species <= SPECIES_NONE || species >= NUM_SPECIES || species == SPECIES_EGG)
        return FALSE;
    if (!IsSpeciesEnabled(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    formTable = GetSpeciesFormTable(GET_BASE_SPECIES_ID(species));
    return !(speciesInfo->isMegaEvolution
          || speciesInfo->isPrimalReversion
          || speciesInfo->isUltraBurst
          || speciesInfo->isGigantamax
          || speciesInfo->isTeraForm
          || speciesInfo->isTotem
          || !IsSpeciesFormUsableOutsideSpecialContext(species, formTable));
}

static bool32 IsOrdinaryEncounterCandidate(enum Species species, u16 minBST, u16 maxBST)
{
    const struct SpeciesInfo *speciesInfo;
    u32 bst;

    if (!IsSpeciesRandomizerEligible(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    if (speciesInfo->isRestrictedLegendary
     || speciesInfo->isSubLegendary
     || speciesInfo->isMythical
     || speciesInfo->isUltraBeast
     || speciesInfo->isParadox)
    {
        return FALSE;
    }

    bst = GetSpeciesBaseStatTotal(species);
    if (bst < minBST || bst > maxBST)
        return FALSE;

    return TRUE;
}

enum Species GetRandomizedSpeciesForEncounter(enum Species originalSpecies, u16 mapId, enum RandomizerEncounterType encounterType, u8 slot, u8 encounterDifficulty)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_ENCOUNTERS
    u16 minBST;
    u16 maxBST;
    u32 ordinaryCount = 0;
    u32 selectedIndex;

    if (encounterDifficulty <= 10)
    {
        minBST = 180;
        maxBST = 360;
    }
    else if (encounterDifficulty <= 20)
    {
        minBST = 240;
        maxBST = 420;
    }
    else if (encounterDifficulty <= 30)
    {
        minBST = 300;
        maxBST = 480;
    }
    else if (encounterDifficulty <= 40)
    {
        minBST = 360;
        maxBST = 540;
    }
    else if (encounterDifficulty <= 50)
    {
        minBST = 420;
        maxBST = 600;
    }
    else
    {
        minBST = 480;
        maxBST = 720;
    }

    for (;;)
    {
        ordinaryCount = 0;
        for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
        {
            if (IsOrdinaryEncounterCandidate(species, minBST, maxBST))
                ordinaryCount++;
        }

        if (ordinaryCount > 0)
            break;

        if (minBST == 0 && maxBST == 1530)
            return originalSpecies;

        if (minBST > 60)
            minBST -= 60;
        else
            minBST = 0;

        if (maxBST < 1470)
            maxBST += 60;
        else
            maxBST = 1530;
    }

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_ENCOUNTER,
                                   mapId,
                                   originalSpecies,
                                   ((u32)encounterType << 8) | slot)
                  % ordinaryCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsOrdinaryEncounterCandidate(species, minBST, maxBST) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}

bool32 IsSpeciesRandomizerLegendaryEncounterEligible(enum Species species)
{
    const struct SpeciesInfo *speciesInfo;

    if (!IsSpeciesRandomizerEligible(species))
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    if (!(speciesInfo->isRestrictedLegendary
       || speciesInfo->isSubLegendary
       || speciesInfo->isParadox))
    {
        return FALSE;
    }

    if (speciesInfo->isMythical || speciesInfo->isUltraBeast)
        return FALSE;

    return TRUE;
}

enum Species GetRandomizedSpeciesForLegendaryEncounter(enum Species originalSpecies, u16 mapId, u8 slot)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_LEGENDARY_ENCOUNTERS
    u32 eligibleCount = 0;
    u32 selectedIndex;

    if (!IsSpeciesRandomizerLegendaryEncounterEligible(originalSpecies))
        return originalSpecies;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerLegendaryEncounterEligible(species))
            eligibleCount++;
    }

    if (eligibleCount == 0)
        return originalSpecies;

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_LEGENDARY_ENCOUNTER,
                                   mapId,
                                   (u32)originalSpecies,
                                   slot)
                  % eligibleCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerLegendaryEncounterEligible(species) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}

enum Species GetRandomizedSpeciesForTrainer(enum Species originalSpecies, u16 trainerId, u8 partySlot)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_TRAINERS
    u32 originalBST;
    u16 minBST;
    u16 maxBST;
    u32 ordinaryCount = 0;
    u32 selectedIndex;

    if (originalSpecies <= SPECIES_NONE || originalSpecies >= NUM_SPECIES)
        return originalSpecies;

    originalBST = GetSpeciesBaseStatTotal(originalSpecies);
    if (originalBST > 1530)
        originalBST = 1530;

    minBST = (originalBST > 50) ? (u16)(originalBST - 50) : 0;
    maxBST = (u16)originalBST + 50;
    if (maxBST > 1530)
        maxBST = 1530;

    for (;;)
    {
        ordinaryCount = 0;
        for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
        {
            if (IsOrdinaryEncounterCandidate(species, minBST, maxBST))
                ordinaryCount++;
        }

        if (ordinaryCount > 0)
            break;

        if (minBST == 0 && maxBST == 1530)
            return originalSpecies;

        if (minBST > 50)
            minBST -= 50;
        else
            minBST = 0;

        if (maxBST < 1480)
            maxBST += 50;
        else
            maxBST = 1530;
    }

    selectedIndex = RandomizerHash(GetRandomizerSeed(),
                                   RANDOMIZER_CATEGORY_TRAINER,
                                   trainerId,
                                   originalSpecies,
                                   ((u32)RANDOMIZER_ALGORITHM_VERSION << 8) | partySlot)
                  % ordinaryCount;

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsOrdinaryEncounterCandidate(species, minBST, maxBST) && selectedIndex-- == 0)
            return species;
    }
#endif

    return originalSpecies;
}

static bool32 HasUsableEvolution(enum Species species)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (evolutions == NULL)
        return FALSE;

    for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        enum Species target = evolutions[i].targetSpecies;

        if (evolutions[i].method != EVO_SPLIT_FROM_EVO
         && target > SPECIES_NONE
         && target < NUM_SPECIES
         && IsSpeciesRandomizerEligible(target))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static bool32 IsSpeciesRandomizerStarterEligibleInternal(enum Species species, bool32 hasPreEvolution)
{
    const struct SpeciesInfo *speciesInfo;
    const struct Evolution *evolutions;
    u32 bst;

    if (!IsSpeciesRandomizerEligible(species))
        return FALSE;
    if (hasPreEvolution)
        return FALSE;

    speciesInfo = &gSpeciesInfo[species];
    if (speciesInfo->isRestrictedLegendary
     || speciesInfo->isSubLegendary
     || speciesInfo->isMythical
     || speciesInfo->isUltraBeast
     || speciesInfo->isParadox)
    {
        return FALSE;
    }

    bst = GetSpeciesBaseStatTotal(species);
    if (bst < RANDOMIZER_STARTER_MIN_BST || bst > RANDOMIZER_STARTER_MAX_BST)
        return FALSE;

    evolutions = GetSpeciesEvolutions(species);
    if (evolutions == NULL)
        return FALSE;

    for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        enum Species middleStage = evolutions[i].targetSpecies;

        if (evolutions[i].method != EVO_SPLIT_FROM_EVO
         && middleStage > SPECIES_NONE
         && middleStage < NUM_SPECIES
         && IsSpeciesRandomizerEligible(middleStage)
         && HasUsableEvolution(middleStage))
        {
            return TRUE;
        }
    }

    return FALSE;
}

bool32 IsSpeciesRandomizerStarterEligible(enum Species species)
{
    return IsSpeciesRandomizerStarterEligibleInternal(species, GetSpeciesPreEvolution(species) != SPECIES_NONE);
}

enum Species GetRandomizedStarterSpecies(enum Species originalSpecies, u8 slot)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_STARTERS
    static u32 sCachedSeed;
    static enum Species sCachedChoices[RANDOMIZER_STARTER_COUNT];
    static bool32 sCacheValid;
    enum Species selected[RANDOMIZER_STARTER_COUNT] = {SPECIES_NONE};
    bool8 hasPreEvolution[NUM_SPECIES] = {FALSE};
    u32 seed = GetRandomizerSeed();
    u32 eligibleCount = 0;

    if (slot >= RANDOMIZER_STARTER_COUNT)
        return originalSpecies;
    if (sCacheValid && sCachedSeed == seed)
        return sCachedChoices[slot];

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        const struct Evolution *evolutions;

        if (!IsSpeciesEnabled(species))
            continue;

        evolutions = GetSpeciesEvolutions(species);

        if (evolutions == NULL)
            continue;

        for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            enum Species target = evolutions[i].targetSpecies;

            if (evolutions[i].method != EVO_SPLIT_FROM_EVO
             && target > SPECIES_NONE
             && target < NUM_SPECIES)
            {
                hasPreEvolution[target] = TRUE;
            }
        }
    }

    for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
    {
        if (IsSpeciesRandomizerStarterEligibleInternal(species, hasPreEvolution[species]))
            eligibleCount++;
    }

    if (eligibleCount < RANDOMIZER_STARTER_COUNT)
        return originalSpecies;

    for (u32 choice = 0; choice < RANDOMIZER_STARTER_COUNT; choice++)
    {
        u32 selectedIndex = RandomizerHash(seed,
                                           RANDOMIZER_CATEGORY_STARTER,
                                           choice,
                                           RANDOMIZER_STARTER_COUNT,
                                           RANDOMIZER_ALGORITHM_VERSION)
                          % (eligibleCount - choice);

        for (enum Species species = SPECIES_NONE + 1; species < NUM_SPECIES; species++)
        {
            bool32 alreadySelected = FALSE;

            if (!IsSpeciesRandomizerStarterEligibleInternal(species, hasPreEvolution[species]))
                continue;

            for (u32 previous = 0; previous < choice; previous++)
            {
                if (selected[previous] == species)
                {
                    alreadySelected = TRUE;
                    break;
                }
            }

            if (!alreadySelected && selectedIndex-- == 0)
            {
                selected[choice] = species;
                break;
            }
        }
    }

    for (u32 choice = 0; choice < RANDOMIZER_STARTER_COUNT; choice++)
    {
        if (selected[choice] == SPECIES_NONE)
            return originalSpecies;
        sCachedChoices[choice] = selected[choice];
    }

    sCachedSeed = seed;
    sCacheValid = TRUE;
    return sCachedChoices[slot];
#endif

    return originalSpecies;
}
