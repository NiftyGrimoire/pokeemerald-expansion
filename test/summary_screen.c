#include "global.h"
#include "config/summary_screen.h"
#include "test/test.h"

TEST("Skills page toggles between stats and numeric stored IVs")
{
    EXPECT(P_SUMMARY_SCREEN_IV_EV_INFO);
    EXPECT(P_SUMMARY_SCREEN_IV_ONLY);
    EXPECT(P_SUMMARY_SCREEN_IV_EV_VALUES);
    EXPECT(P_SUMMARY_SCREEN_IV_EV_TILESET);
    EXPECT(!P_SUMMARY_SCREEN_IV_HYPERTRAIN);
    EXPECT(!P_SUMMARY_SCREEN_EV_ONLY);
}
