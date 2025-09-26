#include "world/Crowd3DCharHandle.h"
#include "obj/Object.h"

WorldCrowd3DCharHandle::WorldCrowd3DCharHandle() : mCrowd(0), mCharItr(0), mCharIdx(-1) {}

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
        mCrowd->Set3DCharXfm(mCharItr, mCharIdx, WorldXfm());
    }
}

void WorldCrowd3DCharHandle::DrawShowing() {
    if (mCrowd) {
        Character *theChar = mCharItr->mDef.mChar;
        if (theChar) {
            mCrowd->Apply3DCharXfm(mCharItr, mCharIdx, RndCam::Current());
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
    mCharItr = iter;
    mCharIdx = i3;
}
