#include "gesture/FreestyleMotionFilter.h"
#include "FreestyleMotionFilter.h"
#include "Skeleton.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Vec.inl"
#include "obj/Object.h"
#include "os/Debug.h"

FreestyleMotionFilter::FreestyleMotionFilter() : mActive(false) { Clear(); }

FreestyleMotionFilter::~FreestyleMotionFilter() {}

BEGIN_PROPSYNCS(FreestyleMotionFilter)
    SYNC_PROP(velocity_threshold, mVelocityThreshold)
    SYNC_PROP(move_time, mFrameMoveMs)
    SYNC_PROP(movement_amount, mMovement)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void FreestyleMotionFilter::Clear() {
    mVelocityThreshold = 0.0f;
    mFrameMoveMs = 0.0f;
    mMovement = 0.0f;
}

void FreestyleMotionFilter::Activate() {
    mActive = true;
    mFrameMoveMs = 0.0f;
    mMovement = 0.0f;
}

void FreestyleMotionFilter::Deactivate() { mActive = false; }

bool FreestyleMotionFilter::IsActive() const { return mActive; }

bool FreestyleMotionFilter::Detected() { return 0 < mFrameMoveMs; }

void FreestyleMotionFilter::UpdateFilters(SkeletonUpdateData const &data) {
    HamPlayerData *player = TheGameData->Player(0);
    MILO_ASSERT(player, 0x44);
    if (player->IsPlaying()) {
        const Skeleton *playerSkel = data.mPlayerSkeletons[0];
        if (playerSkel) {
            mFrameMoveMs = 0.0f;
            for (int i = 0; i < kNumJoints; i++) {
                SkeletonJoint j = (SkeletonJoint)i;
                Vector3 velocity;
                int vel_ms;
                if (playerSkel->Velocity(
                        data.mHistory,
                        kCoordCamera,
                        j,
                        playerSkel->ElapsedMs(),
                        velocity,
                        vel_ms
                    )
                    && playerSkel->JointConf(j) == kConfidenceTracked) {
                    float lensq = LengthSquared(velocity);
                    if (lensq > mVelocityThreshold) {
                        mFrameMoveMs = playerSkel->ElapsedMs();
                        mMovement = sqrtf(lensq);
                        return;
                    }
                }
            }
        }
    }
}

BEGIN_HANDLERS(FreestyleMotionFilter)
    HANDLE_ACTION(activate, Activate())
    HANDLE_ACTION(deactivate, Deactivate())
    HANDLE_EXPR(detected, Detected())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
