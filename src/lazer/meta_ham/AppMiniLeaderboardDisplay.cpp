#include "meta_ham/AppMiniLeaderboardDisplay.h"
#include "net_ham/RockCentral.h"

AppMiniLeaderboardDisplay::AppMiniLeaderboardDisplay()
    : unk60(0), unk64(0), unk68(0), unk6c(0) {}

AppMiniLeaderboardDisplay::~AppMiniLeaderboardDisplay() {
    TheRockCentral.CancelOutstandingCalls(this);
}
