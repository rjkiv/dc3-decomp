#include "world/Crowd3DCharHandle.h"
#include "obj/Object.h"

WorldCrowd3DCharHandle::WorldCrowd3DCharHandle()
    : mCrowd(0), mCrowdCharItr(0), mCrowdChar3DIdx(-1) {}

BEGIN_HANDLERS(WorldCrowd3DCharHandle)
END_HANDLERS

BEGIN_SAVES(WorldCrowd3DCharHandle)
    MILO_FAIL("Attempting to save a 3D crowd character handle");
END_SAVES

BEGIN_COPYS(WorldCrowd3DCharHandle)
    MILO_FAIL("Attempting to copy a 3D crowd character handle");
END_COPYS

BEGIN_LOADS(WorldCrowd3DCharHandle)
    MILO_FAIL("Attempting to load a 3D crowd character handle");
END_LOADS

void WorldCrowd3DCharHandle::UpdatedWorldXfm() {
    if (mCrowd) {
        mCrowd->Set3DCharXfm(mCrowdCharItr, mCrowdChar3DIdx, WorldXfm());
    }
}

void WorldCrowd3DCharHandle::DrawShowing() {
    if (mCrowd) {
        Character *theChar = mCrowdCharItr->mDef.mChar;
        if (theChar) {
            mCrowd->Apply3DCharXfm(mCrowdCharItr, mCrowdChar3DIdx, RndCam::Current());
            theChar->DrawShowing();
        }
    }
}

void WorldCrowd3DCharHandle::Set3DChar(
    WorldCrowd *crowd,
    const std::list<WorldCrowd::CharData>::iterator &iter,
    int i3,
    const Transform &worldXfm
) {
    mCrowd = nullptr;
    SetWorldXfm(worldXfm);
    mCrowd = crowd;
    mCrowdCharItr = iter;
    mCrowdChar3DIdx = i3;
}
