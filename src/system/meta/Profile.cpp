#include "meta/Profile.h"
#include "os/PlatformMgr.h"
#include "utl/Std.h"

Profile::Profile(int pnum) : mDirty(0), mPadNum(pnum), mState(kMetaProfileUnloaded) {}

bool Profile::IsAutosaveEnabled() const { return mState == kMetaProfileLoaded; }

bool Profile::HasValidSaveData() const {
    return mState == kMetaProfileLoaded || mState == kMetaProfileError;
}

ProfileSaveState Profile::GetSaveState() const { return mState; }

bool Profile::IsUnsaved() const {
    bool b = HasCheated();
    if (b != false) {
        b = false;
    } else
        b = mDirty != 0;
    return b;
}

Profile::~Profile() { mDirty = true; }

const char *Profile::GetName() const { return ThePlatformMgr.GetName(mPadNum); }

void Profile::SetSaveState(ProfileSaveState state) {
    MILO_ASSERT(mState != kMetaProfileUnchanged, 0x78);
    if (state != kMetaProfileUnchanged)
        mState = state;
}

void Profile::SaveLoadComplete(ProfileSaveState state) { SetSaveState(state); }

BEGIN_HANDLERS(Profile)
    HANDLE_EXPR(get_pad_num, mPadNum)
    HANDLE_EXPR(get_name, GetName())
    HANDLE_EXPR(has_cheated, HasCheated())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
