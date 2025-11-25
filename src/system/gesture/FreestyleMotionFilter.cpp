#include "gesture/FreestyleMotionFilter.h"
#include "FreestyleMotionFilter.h"
#include "Skeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Object.h"
#include "os/Debug.h"

FreestyleMotionFilter::FreestyleMotionFilter() : mIsActive(false) { Clear(); }

FreestyleMotionFilter::~FreestyleMotionFilter() {}

BEGIN_PROPSYNCS(FreestyleMotionFilter)
    SYNC_PROP(velocity_threshold, unk2c)
    SYNC_PROP(move_time, unk30)
    SYNC_PROP(movement_amount, unk34)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void FreestyleMotionFilter::Clear() {
    unk2c = 0.0f;
    unk30 = 0.0f;
    unk34 = 0.0f;
}

void FreestyleMotionFilter::Activate() {
    mIsActive = true;
    unk30 = 0.0f;
    unk34 = 0.0f;
}

void FreestyleMotionFilter::Deactivate() { mIsActive = false; }

bool FreestyleMotionFilter::IsActive() const { return mIsActive; }

bool FreestyleMotionFilter::Detected() { return 0 < unk30; }

void FreestyleMotionFilter::UpdateFilters(SkeletonUpdateData const &skeletonData) {
    HamPlayerData *player = TheGameData->Player(0);
    MILO_ASSERT(player, 0x44);
    if (player->IsPlaying() && skeletonData.unk0) {
        unk30 = 0.0f;
        for (int i = 0; i < 20; i++) {
            // do something
        }
    }
}

BEGIN_HANDLERS(FreestyleMotionFilter)
    HANDLE_ACTION(activate, Activate())
    HANDLE_ACTION(deactivate, Deactivate())
    HANDLE_EXPR(detected, Detected())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
