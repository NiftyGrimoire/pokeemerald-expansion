#include "global.h"
#include "item_ball.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "randomizer.h"

static u32 GetItemBallAmountFromTemplate(u32);
static u32 GetItemBallIdFromTemplate(u32);

static u32 GetItemBallAmountFromTemplate(u32 itemBallId)
{
    u32 amount = gMapHeader.events->objectEvents[itemBallId].movementRangeX;

    if (amount > MAX_BAG_ITEM_CAPACITY)
        return MAX_BAG_ITEM_CAPACITY;

    return (amount == 0) ? 1 : amount;
}

static u32 GetItemBallIdFromTemplate(u32 itemBallId)
{
    enum Item itemId = gMapHeader.events->objectEvents[itemBallId].trainerRange_berryTreeId;

    if (itemId >= ITEMS_COUNT)
        return ITEM_NONE + 1;

    return GetRandomizedWorldItem(itemId,
                                  gSaveBlock1Ptr->location.mapGroup,
                                  gSaveBlock1Ptr->location.mapNum,
                                  itemBallId);
}

void GetItemBallIdAndAmountFromTemplate(void)
{
    u32 itemBallId = (gSpecialVar_LastTalked - 1);
    gSpecialVar_Result = GetItemBallIdFromTemplate(itemBallId);
    gSpecialVar_0x8009 = GetItemBallAmountFromTemplate(itemBallId);
}

void RandomizeVisibleItemPickup(void)
{
    const struct ObjectEventTemplate *objectTemplate;

    objectTemplate = GetObjectEventTemplateByLocalIdAndMap(
        gSpecialVar_LastTalked,
        gSaveBlock1Ptr->location.mapNum,
        gSaveBlock1Ptr->location.mapGroup);
    if (objectTemplate == NULL || objectTemplate->graphicsId != OBJ_EVENT_GFX_ITEM_BALL)
    {
        return;
    }

    gSpecialVar_0x8000 = GetRandomizedWorldItem(
        gSpecialVar_0x8000,
        gSaveBlock1Ptr->location.mapGroup,
        gSaveBlock1Ptr->location.mapNum,
        gSpecialVar_LastTalked);
}

void RandomizeFreeItemGift(void)
{
    gSpecialVar_0x8000 = GetRandomizedFreeItem(
        gSpecialVar_0x8000,
        gSaveBlock1Ptr->location.mapGroup,
        gSaveBlock1Ptr->location.mapNum,
        gSpecialVar_LastTalked);
}
