#include "gesture/NavigationSkeletonDir.h"
#include "SkeletonDir.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DirectionGestureFilter.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonViz.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"

NavigationSkeletonDir::NavigationSkeletonDir() : mDirectionGestureFilter(nullptr) {}
NavigationSkeletonDir::~NavigationSkeletonDir() { delete mDirectionGestureFilter; }

BEGIN_HANDLERS(NavigationSkeletonDir)
    HANDLE_ACTION(clear_filters, Clear())
    HANDLE_SUPERCLASS(SkeletonDir)
END_HANDLERS

BEGIN_PROPSYNCS(NavigationSkeletonDir)
    SYNC_SUPERCLASS(SkeletonDir)
END_PROPSYNCS

BEGIN_SAVES(NavigationSkeletonDir)
    SAVE_SUPERCLASS(SkeletonDir)
END_SAVES

BEGIN_COPYS(NavigationSkeletonDir)
    COPY_SUPERCLASS(SkeletonDir)
END_COPYS

BEGIN_LOADS(NavigationSkeletonDir)
    SkeletonDir::Load(bs);
END_LOADS

void NavigationSkeletonDir::Clear() {
    if (mDirectionGestureFilter) {
        mDirectionGestureFilter->Clear();
    }
}

void NavigationSkeletonDir::PostUpdate(const SkeletonUpdateData *data) {
    if (data) {
        RndOverlay::Find("swipe_direction")->SetCallback(mDirectionGestureFilter);
        if (mDirectionGestureFilter) {
            const Skeleton &skeleton = *data->unk4[0];
            mDirectionGestureFilter->Update(skeleton, skeleton.ElapsedMs());
        }
    }
}

void NavigationSkeletonDir::Draw(const BaseSkeleton &base, SkeletonViz &viz) {
    const Skeleton *skeleton = dynamic_cast<const Skeleton *>(&base);
    if (mDirectionGestureFilter) {
        mDirectionGestureFilter->Draw(*skeleton, viz);
    }
}

void NavigationSkeletonDir::MiloUpdate() {
    SkeletonDir::MiloUpdate();
    if (!mDirectionGestureFilter) {
        mDirectionGestureFilter = new DirectionGestureFilterSingleUser(
            kSkeletonRight, kSkeletonLeft, 1.0f, 0.1f
        );
    }
}
