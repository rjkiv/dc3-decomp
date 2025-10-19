#include "gesture/NavigationSkeletonDir.h"
#include "SkeletonDir.h"
#include "gesture/SkeletonDir.h"
#include "obj/Object.h"

NavigationSkeletonDir::NavigationSkeletonDir() : mDirectionGestureFilter(0) {}
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
