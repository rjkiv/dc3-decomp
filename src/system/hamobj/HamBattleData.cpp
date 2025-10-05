#include "hamobj/HamBattleData.h"
#include "hamobj/HamCamShot.h"
#include "obj/Object.h"

HamBattleData::HamBattleData() {}
HamBattleData::~HamBattleData() {}

BEGIN_HANDLERS(HamBattleData)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(BattleStep)
    SYNC_PROP_SET(players, o.mPlayers, o.mPlayers = (HamPlayerFlags)_val.Int())
    SYNC_PROP(music_range, o.mMusicRange)
    SYNC_PROP(play_range, o.mPlayRange)
    SYNC_PROP(cam, o.mCam)
    SYNC_PROP(nonplay_action, o.mNonplayAction)
    SYNC_PROP(state, o.mState)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamBattleData)
    SYNC_PROP(steps, mSteps)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const BattleStep &step) {
    bs << step.mPlayers;
    bs << step.mMusicRange.start;
    bs << step.mMusicRange.end;
    bs << step.mPlayRange.start;
    bs << step.mPlayRange.end;
    bs << step.mCam;
    bs << step.mNonplayAction;
    bs << step.mState;
    return bs;
}

BEGIN_SAVES(HamBattleData)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mSteps;
END_SAVES

BEGIN_COPYS(HamBattleData)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamBattleData)
    BEGIN_COPYING_MEMBERS
        mSteps.clear();
        mSteps.insert(mSteps.begin(), c->mSteps.begin(), c->mSteps.end());
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, BattleStep &step) {
    int flags;
    d >> flags;
    step.mPlayers = (HamPlayerFlags)flags;
    d >> step.mMusicRange.start;
    d >> step.mMusicRange.end;
    d >> step.mPlayRange.start;
    d >> step.mPlayRange.end;
    if (d.rev > 1) {
        d >> step.mCam;
    }
    if (d.rev > 2) {
        d >> step.mNonplayAction;
    }
    if (d.rev > 3) {
        d >> step.mState;
    }
    return d;
}

BEGIN_LOADS(HamBattleData)
    LOAD_REVS(bs)
    ASSERT_REVS(4, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bsrev >> mSteps;
END_LOADS
