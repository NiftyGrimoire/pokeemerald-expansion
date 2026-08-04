#include "global.h"
#include "item.h"
#include "move.h"
#include "pokemon.h"
#include "party_menu.h"
#include "random.h"
#include "random_mon_generation.h"
#include "randomizer.h"

#define RANDOMIZER_ZERO_SEED_FALLBACK 0x6D2B79F5
#define RANDOMIZER_STARTER_COUNT 3
#define RANDOMIZER_STARTER_MIN_BST 300
#define RANDOMIZER_STARTER_MAX_BST 350
#define RANDOMIZER_MOVE_TYPE_WEIGHT_STAB 4
#define RANDOMIZER_MOVE_TYPE_WEIGHT_COVERAGE 2
#define RANDOMIZER_MOVE_TYPE_WEIGHT_STATUS 1
#define RANDOMIZER_MOVE_POWER_WEIGHT_MAX 12
#define RANDOMIZER_INITIAL_MOVE_POWER_PREFERRED_MAX 40
#define RANDOMIZER_INITIAL_MOVE_POWER_ALLOWED_MAX 60
#define RANDOMIZER_INITIAL_MOVE_POWER_WEIGHT_PREFERRED 24
#define RANDOMIZER_INITIAL_MOVE_POWER_WEIGHT_ALLOWED 2
#define RANDOMIZER_MOVE_BUCKET_COUNT_MAX 512
#define RANDOMIZER_MOVE_BUCKET_NONE 0xFFFF
#define RANDOMIZER_MOVE_BUCKET_STATUS_TYPE NUMBER_OF_MON_TYPES

struct RandomizerMoveBucket
{
    u16 firstMove;
    u16 moveCount;
    u16 powerOrStatusTier;
    u8 type;
};

static const u8 sRandomizerLevelUpMoveLevels[RANDOMIZER_LEVEL_UP_MOVE_COUNT] =
{
    1, 1,
    5, 10, 15,
    16, 18, 19,
    22, 24,
    27, 29,
    31,
    33,
    42,
    46,
    50,
    58,
    60,
    80,
};

static EWRAM_DATA u16 sEvolutionFamilyCache[NUM_SPECIES] = {0};
static EWRAM_DATA bool8 sEvolutionFamilyHasProtectedAbility[NUM_SPECIES] = {0};
static EWRAM_DATA u16 sEvolutionFamilyAbilityCache[NUM_SPECIES] = {0};
static EWRAM_DATA u32 sEvolutionFamilyAbilityCacheSeed = 0;
static EWRAM_DATA bool8 sEvolutionFamilyCacheInitialized = FALSE;
static EWRAM_DATA struct RandomizerMoveBucket sRandomizerMoveBuckets[RANDOMIZER_MOVE_BUCKET_COUNT_MAX] = {0};
static EWRAM_DATA u16 sRandomizerBucketMoves[MOVES_COUNT] = {0};
static EWRAM_DATA u16 sRandomizerMoveBucketByMove[MOVES_COUNT] = {0};
static EWRAM_DATA u16 sRandomizerMoveBucketCount = 0;
static EWRAM_DATA bool8 sRandomizerMoveBucketsInitialized = FALSE;
static EWRAM_DATA bool8 sRandomizerMoveBucketsUsable = FALSE;
static EWRAM_DATA enum Move sRandomizerTMToMove[NUM_TECHNICAL_MACHINES + 1] = {0};
static EWRAM_DATA u16 sRandomizerTMCandidates[MOVES_COUNT] = {0};
static EWRAM_DATA u32 sRandomizerTMMappingSeed = 0;
static EWRAM_DATA bool8 sRandomizerTMMappingInitialized = FALSE;

static const enum Item sRandomizerWorldItemPool[] =
{
    ITEM_FIRE_STONE,
    ITEM_WATER_STONE,
    ITEM_THUNDER_STONE,
    ITEM_LEAF_STONE,
    ITEM_ICE_STONE,
    ITEM_SUN_STONE,
    ITEM_MOON_STONE,
    ITEM_SHINY_STONE,
    ITEM_DUSK_STONE,
    ITEM_DAWN_STONE,
    ITEM_METAL_COAT,
    ITEM_KINGS_ROCK,
    ITEM_DEEP_SEA_SCALE,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_RAZOR_CLAW,
    ITEM_RAZOR_FANG,
    ITEM_SILK_SCARF,
    ITEM_CHARCOAL,
    ITEM_MYSTIC_WATER,
    ITEM_MAGNET,
    ITEM_MIRACLE_SEED,
    ITEM_NEVER_MELT_ICE,
    ITEM_BLACK_BELT,
    ITEM_POISON_BARB,
    ITEM_SOFT_SAND,
    ITEM_SHARP_BEAK,
    ITEM_TWISTED_SPOON,
    ITEM_SILVER_POWDER,
    ITEM_HARD_STONE,
    ITEM_SPELL_TAG,
    ITEM_DRAGON_FANG,
    ITEM_BLACK_GLASSES,
    ITEM_FAIRY_FEATHER,
    ITEM_EVIOLITE,
    ITEM_LEFTOVERS,
    ITEM_LIFE_ORB,
    ITEM_FOCUS_SASH,
    ITEM_FOCUS_BAND,
    ITEM_ASSAULT_VEST,
    ITEM_CLEAR_AMULET,
    ITEM_CHOICE_BAND,
    ITEM_CHOICE_SPECS,
    ITEM_CHOICE_SCARF,
    ITEM_MUSCLE_BAND,
    ITEM_WISE_GLASSES,
    ITEM_EXPERT_BELT,
    ITEM_SCOPE_LENS,
    ITEM_WIDE_LENS,
    ITEM_ZOOM_LENS,
    ITEM_QUICK_CLAW,
    ITEM_LOADED_DICE,
    ITEM_BRIGHT_POWDER,
    ITEM_COVERT_CLOAK,
    ITEM_GRIP_CLAW,
    ITEM_DAMP_ROCK,
    ITEM_HEAT_ROCK,
    ITEM_SMOOTH_ROCK,
    ITEM_ICY_ROCK,
    ITEM_LIGHT_CLAY,
    ITEM_TERRAIN_EXTENDER,
    ITEM_LINKING_CORD,
};

STATIC_ASSERT(ARRAY_COUNT(sRandomizerWorldItemPool) == 61, RandomizerWorldItemPoolCount);

static void InitRandomizerMoveBuckets(void);
static void InitRandomizerTMMapping(void);
static u32 GetRandomizerStatusMoveTier(enum Move move);
static u32 GetRandomizerStatusTierWeight(u32 moveTier, u8 level);
static u32 GetRandomizerDamagingMovePowerWeight(u32 power, u8 level);

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
    InitRandomizerMoveBuckets();
    InitRandomizerTMMapping();
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

static bool32 IsAuthoredHMMove(enum Move move)
{
    for (u32 index = NUM_TECHNICAL_MACHINES + 1; index <= NUM_ALL_MACHINES; index++)
    {
        if (gTMHMItemMoveIds[index].moveId == move)
            return TRUE;
    }

    return FALSE;
}

static bool32 IsRandomizerTMEligible(enum Move move)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_TMS
    if (!IsMoveRandomizerEligible(move)
     || IsAuthoredHMMove(move))
        return FALSE;

    return TRUE;
#else
    return FALSE;
#endif
}

static u32 GetRandomizerTMWeight(enum Move move)
{
    u32 weight;

    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        weight = GetRandomizerStatusTierWeight(GetRandomizerStatusMoveTier(move), 80);
    else
        weight = GetRandomizerDamagingMovePowerWeight(GetMovePower(move), 80);

    // Keep severe drawbacks possible without letting raw power favor them.
    if (IsExplosionMove(move)
     || GetMoveEffect(move) == EFFECT_RECOIL
     || GetMoveEffect(move) == EFFECT_RECOIL_IF_MISS)
        weight = max(1, weight / 4);

    return max(1, weight);
}

static void InitRandomizerTMMapping(void)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_TMS
    u32 seed = GetRandomizerSeed();
    u16 candidateCount = 0;

    if (sRandomizerTMMappingInitialized && sRandomizerTMMappingSeed == seed)
        return;

    sRandomizerTMMappingInitialized = TRUE;
    sRandomizerTMMappingSeed = seed;

    for (u32 i = 0; i <= NUM_TECHNICAL_MACHINES; i++)
        sRandomizerTMToMove[i] = MOVE_NONE;

    for (enum Move move = MOVE_NONE + 1; move < MOVES_COUNT; move++)
    {
        if (IsRandomizerTMEligible(move))
            sRandomizerTMCandidates[candidateCount++] = move;
    }

    for (u32 tmIndex = 1; tmIndex <= NUM_TECHNICAL_MACHINES; tmIndex++)
    {
        u32 totalWeight = 0;
        u32 selection;
        u16 selected = 0;

        for (u16 i = 0; i < candidateCount; i++)
            totalWeight += GetRandomizerTMWeight(sRandomizerTMCandidates[i]);

        selection = RandomizerHash(seed,
                                   RANDOMIZER_CATEGORY_TM,
                                   tmIndex,
                                   candidateCount,
                                   0) % totalWeight;

        for (u16 i = 0; i < candidateCount; i++)
        {
            u32 weight = GetRandomizerTMWeight(sRandomizerTMCandidates[i]);

            if (selection < weight)
            {
                selected = i;
                break;
            }
            selection -= weight;
        }

        sRandomizerTMToMove[tmIndex] = sRandomizerTMCandidates[selected];
        sRandomizerTMCandidates[selected] = sRandomizerTMCandidates[--candidateCount];
    }
#endif
}

enum Move GetRandomizerTMHMMoveId(enum TMHMIndex index)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_TMS
    if (index > 0 && index <= NUM_TECHNICAL_MACHINES)
    {
        InitRandomizerTMMapping();
        return sRandomizerTMToMove[index];
    }
#endif

    if (index <= NUM_ALL_MACHINES)
        return gTMHMItemMoveIds[index].moveId;
    return MOVE_NONE;
}

enum Item GetRandomizerTMHMItemIdFromMoveId(enum Move move)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_TMS
    InitRandomizerTMMapping();

    for (u32 index = 1; index <= NUM_TECHNICAL_MACHINES; index++)
    {
        if (sRandomizerTMToMove[index] == move)
            return gTMHMItemMoveIds[index].itemId;
    }
#endif

    for (u32 index = NUM_TECHNICAL_MACHINES + 1; index <= NUM_ALL_MACHINES; index++)
    {
        if (gTMHMItemMoveIds[index].moveId == move)
            return gTMHMItemMoveIds[index].itemId;
    }

    return ITEM_NONE;
}

static bool32 IsRandomizerItemProtected(enum Item item)
{
    if (item <= ITEM_NONE || item >= ITEMS_COUNT)
        return TRUE;
    if (GetItemImportance(item)
     || GetItemTMHMIndex(item) != 0
     || GetItemPocket(item) == POCKET_BERRIES)
        return TRUE;

    return FALSE;
}

bool32 IsWorldItemRandomizerEligible(enum Item item)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_WORLD_ITEMS
    return !IsRandomizerItemProtected(item);
#else
    return FALSE;
#endif
}

enum Item GetRandomizedWorldItem(enum Item originalItem, u8 mapGroup, u8 mapNum, u8 objectId)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_WORLD_ITEMS
    u32 mapId;

    if (!IsWorldItemRandomizerEligible(originalItem))
        return originalItem;

    mapId = ((u32)mapGroup << 8) | mapNum;
    return sRandomizerWorldItemPool[RandomizerHash(GetRandomizerSeed(),
                                                   RANDOMIZER_CATEGORY_WORLD_ITEM,
                                                   mapId,
                                                   objectId,
                                                   originalItem)
                                   % ARRAY_COUNT(sRandomizerWorldItemPool)];
#else
    return originalItem;
#endif
}

bool32 IsFreeItemRandomizerEligible(enum Item item)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_FREE_ITEMS
    return !IsRandomizerItemProtected(item);
#else
    return FALSE;
#endif
}

enum Item GetRandomizedFreeItem(enum Item originalItem, u8 mapGroup, u8 mapNum, u8 contextId)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_FREE_ITEMS
    u32 mapId;
    if (!IsFreeItemRandomizerEligible(originalItem))
        return originalItem;
    mapId = ((u32)mapGroup << 8) | mapNum;
    return sRandomizerWorldItemPool[RandomizerHash(GetRandomizerSeed(),
                                                   RANDOMIZER_CATEGORY_FREE_ITEM,
                                                   mapId,
                                                   contextId,
                                                   originalItem)
                                   % ARRAY_COUNT(sRandomizerWorldItemPool)];
#else
    return originalItem;
#endif
}

u8 GetRandomizerFriendshipEvolutionLevel(enum Species species)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_EVOLUTIONS
    switch (species)
    {
    // Baby Pokemon and other early first-stage evolutions.
    case SPECIES_PICHU:
    case SPECIES_CLEFFA:
    case SPECIES_IGGLYBUFF:
    case SPECIES_TOGEPI:
    case SPECIES_AZURILL:
    case SPECIES_BUDEW:
    case SPECIES_CHINGLING:
    case SPECIES_MUNCHLAX:
        return 20;

    // Ordinary midgame friendship evolutions.
    case SPECIES_GOLBAT:
    case SPECIES_MEOWTH_ALOLA:
    case SPECIES_EEVEE:
    case SPECIES_BUNEARY:
    case SPECIES_RIOLU:
    case SPECIES_WOOBAT:
    case SPECIES_SWADLOON:
    case SPECIES_SNOM:
        return 30;

    // Late or exceptional friendship evolutions.
    case SPECIES_CHANSEY:
    case SPECIES_TYPE_NULL:
        return 40;
    default:
        break;
    }
#endif

    return 0;
}

static bool32 IsSelectedRandomizerEvolutionTarget(enum Species species, enum Species targetSpecies, enum Species firstTarget, enum Species secondTarget)
{
    enum Species selectedTarget;

    if (targetSpecies != firstTarget && targetSpecies != secondTarget)
        return TRUE;

    selectedTarget = RandomizerHash(GetRandomizerSeed(), RANDOMIZER_CATEGORY_EVOLUTION, species, firstTarget, secondTarget) % 2 == 0
                   ? firstTarget
                   : secondTarget;
    return targetSpecies == selectedTarget;
}

static bool32 IsSelectedEeveeLevelEvolutionTarget(enum Species targetSpecies)
{
    static const enum Species targets[] =
    {
        SPECIES_SYLVEON,
        SPECIES_ESPEON,
        SPECIES_UMBREON,
    };

    for (u32 i = 0; i < ARRAY_COUNT(targets); i++)
    {
        if (targetSpecies == targets[i])
            return i == RandomizerHash(GetRandomizerSeed(), RANDOMIZER_CATEGORY_EVOLUTION, SPECIES_EEVEE, ARRAY_COUNT(targets), 0) % ARRAY_COUNT(targets);
    }
    return TRUE;
}

static s32 GetAlcremieFlavorIndex(enum Species targetSpecies)
{
    if (targetSpecies == SPECIES_ALCREMIE_STRAWBERRY_VANILLA_CREAM)
        return 0;
    if (targetSpecies >= SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM
     && targetSpecies <= SPECIES_ALCREMIE_STRAWBERRY_RAINBOW_SWIRL)
        return targetSpecies - SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM + 1;
    if (targetSpecies >= SPECIES_ALCREMIE_BERRY_VANILLA_CREAM
     && targetSpecies <= SPECIES_ALCREMIE_RIBBON_RAINBOW_SWIRL)
        return (targetSpecies - SPECIES_ALCREMIE_BERRY_VANILLA_CREAM) % 9;
    return -1;
}

bool32 IsRandomizerEvolutionTargetSelected(enum Species species, enum Species targetSpecies)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_EVOLUTIONS
    if (species == SPECIES_MILCERY)
    {
        s32 flavorIndex = GetAlcremieFlavorIndex(targetSpecies);

        if (flavorIndex < 0)
            return TRUE;
        return flavorIndex == RandomizerHash(GetRandomizerSeed(), RANDOMIZER_CATEGORY_EVOLUTION, species, 9, 0) % 9;
    }

    switch (species)
    {
    case SPECIES_EEVEE:
        return IsSelectedEeveeLevelEvolutionTarget(targetSpecies);
    case SPECIES_ROCKRUFF:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_LYCANROC_MIDDAY, SPECIES_LYCANROC_MIDNIGHT);
    case SPECIES_COSMOEM:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_SOLGALEO, SPECIES_LUNALA);
    case SPECIES_PIKACHU:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_RAICHU, SPECIES_RAICHU_ALOLA);
    case SPECIES_EXEGGCUTE:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_EXEGGUTOR, SPECIES_EXEGGUTOR_ALOLA);
    case SPECIES_CUBONE:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_MAROWAK, SPECIES_MAROWAK_ALOLA);
    case SPECIES_KOFFING:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_WEEZING, SPECIES_WEEZING_GALAR);
    case SPECIES_MIME_JR:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_MR_MIME, SPECIES_MR_MIME_GALAR);
    case SPECIES_QUILAVA:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_TYPHLOSION, SPECIES_TYPHLOSION_HISUI);
    case SPECIES_DEWOTT:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_SAMUROTT, SPECIES_SAMUROTT_HISUI);
    case SPECIES_PETILIL:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_LILLIGANT, SPECIES_LILLIGANT_HISUI);
    case SPECIES_RUFFLET:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_BRAVIARY, SPECIES_BRAVIARY_HISUI);
    case SPECIES_GOOMY:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_SLIGGOO, SPECIES_SLIGGOO_HISUI);
    case SPECIES_BERGMITE:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_AVALUGG, SPECIES_AVALUGG_HISUI);
    case SPECIES_DARTRIX:
        return IsSelectedRandomizerEvolutionTarget(species, targetSpecies, SPECIES_DECIDUEYE, SPECIES_DECIDUEYE_HISUI);
    default:
        break;
    }
#endif

    return TRUE;
}

bool32 ShouldRandomizerIgnoreEvolutionCondition(enum Species species, u16 condition)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_EVOLUTIONS
    if (condition == IF_TIME || condition == IF_NOT_TIME)
    {
        switch (species)
        {
        case SPECIES_RATTATA_ALOLA:
        case SPECIES_CUBONE:
        case SPECIES_HAPPINY:
        case SPECIES_EEVEE:
        case SPECIES_GLIGAR:
        case SPECIES_SNEASEL:
        case SPECIES_SNEASEL_HISUI:
        case SPECIES_LINOONE_GALAR:
        case SPECIES_BUDEW:
        case SPECIES_CHINGLING:
        case SPECIES_RIOLU:
        case SPECIES_TYRUNT:
        case SPECIES_AMAURA:
        case SPECIES_YUNGOOS:
        case SPECIES_ROCKRUFF:
        case SPECIES_ROCKRUFF_OWN_TEMPO:
        case SPECIES_FOMANTIS:
        case SPECIES_COSMOEM:
        case SPECIES_SNOM:
        case SPECIES_GREAVARD:
        case SPECIES_URSARING:
            return TRUE;
        default:
            break;
        }
    }
    else if (condition == IF_REGION || condition == IF_NOT_REGION)
    {
        switch (species)
        {
        case SPECIES_PIKACHU:
        case SPECIES_EXEGGCUTE:
        case SPECIES_CUBONE:
        case SPECIES_KOFFING:
        case SPECIES_MIME_JR:
        case SPECIES_QUILAVA:
        case SPECIES_DEWOTT:
        case SPECIES_PETILIL:
        case SPECIES_RUFFLET:
        case SPECIES_GOOMY:
        case SPECIES_BERGMITE:
        case SPECIES_DARTRIX:
        case SPECIES_URSARING:
            return TRUE;
        default:
            break;
        }
    }
#endif

    return FALSE;
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

    return move != MOVE_STRUGGLE
        && move != MOVE_TERA_BLAST;
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
        if (GetMoveEffect(move) == EFFECT_PROTECT
         || GetMoveEffect(move) == EFFECT_WEATHER
         || GetMoveEffect(move) == EFFECT_WEATHER_AND_SWITCH)
            return 1;
        return 0;
    }
}

static u32 GetRandomizerStatusTierWeight(u32 moveTier, u8 level)
{
    if (level == 1)
    {
        static const u8 sInitialWeights[] = {13, 0, 0};
        return sInitialWeights[moveTier];
    }
    else if (level < 24)
    {
        static const u8 sEarlyWeights[] = {13, 8, 4};
        return sEarlyWeights[moveTier];
    }
    else if (level < 42)
    {
        static const u8 sMiddleWeights[] = {3, 32, 11};
        return sMiddleWeights[moveTier];
    }
    else
    {
        static const u8 sLateWeights[] = {1, 13, 48};
        return sLateWeights[moveTier];
    }
}

static u32 GetRandomizerStatusMoveWeight(enum Move move, u8 level)
{
    return GetRandomizerStatusTierWeight(GetRandomizerStatusMoveTier(move), level);
}

static u32 GetRandomizerDamagingMovePowerWeight(u32 power, u8 level)
{
    u32 targetPower = min(35 + level, 120);
    u32 powerDistance;
    u32 powerPenaltyBand;
    u32 powerPenalty;

    // Fixed and level-based damage moves have no listed base power.
    if (power == 0)
        power = 50;
    if (level == 1)
    {
        if (power <= RANDOMIZER_INITIAL_MOVE_POWER_PREFERRED_MAX)
            return RANDOMIZER_INITIAL_MOVE_POWER_WEIGHT_PREFERRED;
        if (power <= RANDOMIZER_INITIAL_MOVE_POWER_ALLOWED_MAX)
            return RANDOMIZER_INITIAL_MOVE_POWER_WEIGHT_ALLOWED;
        return 0;
    }
    powerDistance = (power > targetPower ? power - targetPower : targetPower - power);
    powerPenaltyBand = (level < 24) ? 7 : 10;
    powerPenalty = powerDistance / powerPenaltyBand;
    if (powerPenalty >= RANDOMIZER_MOVE_POWER_WEIGHT_MAX - 1)
        return 1;
    return RANDOMIZER_MOVE_POWER_WEIGHT_MAX - powerPenalty;
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
        return typeWeight * GetRandomizerDamagingMovePowerWeight(GetMovePower(move), level);
    }
}

static u16 FindRandomizerMoveBucket(u8 type, u16 powerOrStatusTier)
{
    for (u32 i = 0; i < sRandomizerMoveBucketCount; i++)
    {
        if (sRandomizerMoveBuckets[i].type == type
         && sRandomizerMoveBuckets[i].powerOrStatusTier == powerOrStatusTier)
            return i;
    }
    return RANDOMIZER_MOVE_BUCKET_NONE;
}

static void InitRandomizerMoveBuckets(void)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_LEARNSETS
    u16 nextMove = 0;

    if (sRandomizerMoveBucketsInitialized)
        return;
    sRandomizerMoveBucketsInitialized = TRUE;
    sRandomizerMoveBucketsUsable = FALSE;
    sRandomizerMoveBucketCount = 0;

    for (enum Move move = MOVE_NONE; move < MOVES_COUNT; move++)
        sRandomizerMoveBucketByMove[move] = RANDOMIZER_MOVE_BUCKET_NONE;

    for (enum Move move = MOVE_NONE + 1; move < MOVES_COUNT; move++)
    {
        u8 type;
        u16 powerOrStatusTier;
        u16 bucket;

        if (!IsMoveRandomizerEligible(move))
            continue;
        if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        {
            type = RANDOMIZER_MOVE_BUCKET_STATUS_TYPE;
            powerOrStatusTier = GetRandomizerStatusMoveTier(move);
        }
        else
        {
            type = GetMoveType(move);
            powerOrStatusTier = GetMovePower(move);
            if (powerOrStatusTier == 0)
                powerOrStatusTier = 50;
        }

        bucket = FindRandomizerMoveBucket(type, powerOrStatusTier);
        if (bucket == RANDOMIZER_MOVE_BUCKET_NONE)
        {
            if (sRandomizerMoveBucketCount >= RANDOMIZER_MOVE_BUCKET_COUNT_MAX)
                return;
            bucket = sRandomizerMoveBucketCount++;
            sRandomizerMoveBuckets[bucket].type = type;
            sRandomizerMoveBuckets[bucket].powerOrStatusTier = powerOrStatusTier;
        }
        sRandomizerMoveBuckets[bucket].moveCount++;
        sRandomizerMoveBucketByMove[move] = bucket;
    }

    for (u32 bucket = 0; bucket < sRandomizerMoveBucketCount; bucket++)
    {
        u16 moveCount = sRandomizerMoveBuckets[bucket].moveCount;

        sRandomizerMoveBuckets[bucket].firstMove = nextMove;
        sRandomizerMoveBuckets[bucket].moveCount = 0;
        nextMove += moveCount;
    }
    for (enum Move move = MOVE_NONE + 1; move < MOVES_COUNT; move++)
    {
        u16 bucket = sRandomizerMoveBucketByMove[move];

        if (bucket != RANDOMIZER_MOVE_BUCKET_NONE)
        {
            u16 index = sRandomizerMoveBuckets[bucket].firstMove + sRandomizerMoveBuckets[bucket].moveCount++;
            sRandomizerBucketMoves[index] = move;
        }
    }
    sRandomizerMoveBucketsUsable = TRUE;
#endif
}

static u32 GetRandomizerMoveBucketWeight(enum Species species, const struct RandomizerMoveBucket *bucket, u8 level)
{
    if (bucket->type == RANDOMIZER_MOVE_BUCKET_STATUS_TYPE)
        return RANDOMIZER_MOVE_TYPE_WEIGHT_STATUS * GetRandomizerStatusTierWeight(bucket->powerOrStatusTier, level);
    else if (bucket->type == GetSpeciesType(species, 0) || bucket->type == GetSpeciesType(species, 1))
        return RANDOMIZER_MOVE_TYPE_WEIGHT_STAB * GetRandomizerDamagingMovePowerWeight(bucket->powerOrStatusTier, level);
    else
        return RANDOMIZER_MOVE_TYPE_WEIGHT_COVERAGE * GetRandomizerDamagingMovePowerWeight(bucket->powerOrStatusTier, level);
}

static u16 GetRandomizerMoveBucketAvailableCount(u16 bucket, const u16 *excludedMoves, u8 excludedMoveCount)
{
    u16 availableCount = sRandomizerMoveBuckets[bucket].moveCount;

    for (u32 i = 0; i < excludedMoveCount; i++)
    {
        enum Move move = excludedMoves[i];
        bool32 alreadyExcluded = FALSE;

        for (u32 j = 0; j < i; j++)
        {
            if (excludedMoves[j] == move)
            {
                alreadyExcluded = TRUE;
                break;
            }
        }

        if (!alreadyExcluded
         && availableCount != 0
         && move > MOVE_NONE
         && move < MOVES_COUNT
         && sRandomizerMoveBucketByMove[move] == bucket)
            availableCount--;
    }
    return availableCount;
}

enum Move GetRandomizedLevelUpMove(enum Species species, u8 learnsetSlot, const u16 *excludedMoves, u8 excludedMoveCount, enum Move fallbackMove)
{
#if RANDOMIZER_ENABLED && RANDOMIZER_LEARNSETS
    u32 totalWeight = 0;
    u32 selection;
    u8 learnLevel = GetRandomizerLevelUpMoveLevel(learnsetSlot);

    if (species <= SPECIES_NONE || species >= NUM_SPECIES || !IsSpeciesEnabled(species))
        return fallbackMove;

    InitRandomizerMoveBuckets();
    if (!sRandomizerMoveBucketsUsable)
        return fallbackMove;

    for (u32 bucket = 0; bucket < sRandomizerMoveBucketCount; bucket++)
    {
        u32 availableCount = GetRandomizerMoveBucketAvailableCount(bucket, excludedMoves, excludedMoveCount);
        totalWeight += availableCount * GetRandomizerMoveBucketWeight(species, &sRandomizerMoveBuckets[bucket], learnLevel);
    }

    if (totalWeight == 0)
        return fallbackMove;

    selection = RandomizerHash(GetRandomizerSeed(), RANDOMIZER_CATEGORY_LEARNSET, species, learnsetSlot, 0) % totalWeight;
    for (u32 bucket = 0; bucket < sRandomizerMoveBucketCount; bucket++)
    {
        const struct RandomizerMoveBucket *moveBucket = &sRandomizerMoveBuckets[bucket];
        u32 availableCount = GetRandomizerMoveBucketAvailableCount(bucket, excludedMoves, excludedMoveCount);
        u32 weight = GetRandomizerMoveBucketWeight(species, moveBucket, learnLevel);
        u32 bucketWeight = availableCount * weight;

        if (selection < bucketWeight)
        {
            u32 selectedMove = selection / weight;

            for (u32 i = 0; i < moveBucket->moveCount; i++)
            {
                enum Move move = sRandomizerBucketMoves[moveBucket->firstMove + i];

                if (!IsExcludedRandomizerMove(move, excludedMoves, excludedMoveCount))
                {
                    if (selectedMove == 0)
                        return move;
                    selectedMove--;
                }
            }
            return fallbackMove;
        }
        selection -= bucketWeight;
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
        if (!IsAbilityRandomizerEligible(ability))
            continue;
        if (selectedIndex == 0)
        {
            sEvolutionFamilyAbilityCache[family] = ability;
            return sEvolutionFamilyAbilityCache[family];
        }
        selectedIndex--;
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
        if (IsOrdinaryEncounterCandidate(species, minBST, maxBST))
        {
            if (selectedIndex == 0)
                return species;
            selectedIndex--;
        }
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
        if (IsSpeciesRandomizerLegendaryEncounterEligible(species))
        {
            if (selectedIndex == 0)
                return species;
            selectedIndex--;
        }
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
        if (IsOrdinaryEncounterCandidate(species, minBST, maxBST))
        {
            if (selectedIndex == 0)
                return species;
            selectedIndex--;
        }
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

            if (!alreadySelected && selectedIndex == 0)
            {
                selected[choice] = species;
                break;
            }
            if (!alreadySelected)
                selectedIndex--;
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
