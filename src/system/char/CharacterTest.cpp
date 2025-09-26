#include "char/CharacterTest.h"
#include "Character.h"
#include "rndobj/Overlay.h"

CharacterTest::CharacterTest(Character *theChar)
    : mMe(theChar), mDriver(theChar), mClip1(theChar), mClip2(theChar),
      mFilterGroup(theChar), mTeleportTo(theChar), mWalkPath(theChar), mTransition(0),
      mCycleTransition(1), mMetronome(0), mZeroTravel(0), mShowScreenSize(0),
      mShowFootExtents(0), unk94(0), unk98(0),
      mOverlay(RndOverlay::Find("char_test", true)) {
    static Symbol none("none");
    mShowDistMap = none;
}

CharacterTest::~CharacterTest() {
    mOverlay = RndOverlay::Find("char_test", false);
    if (mOverlay) {
        if (mOverlay->GetCallback() == this) {
            mOverlay->SetCallback(nullptr);
            mOverlay->SetShowing(false);
        }
    }
}

BEGIN_CUSTOM_HANDLERS(CharacterTest)
    HANDLE_ACTION(add_defaults, AddDefaults())
    HANDLE_ACTION(test_walk, Walk())
    HANDLE_ACTION(recenter, Recenter())
    HANDLE(get_filtered_clips, OnGetFilteredClips)
    HANDLE_ACTION(sync, Sync())
END_CUSTOM_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharacterTest)
    SYNC_PROP(show_screen_size, o.mShowScreenSize)
    SYNC_PROP(driver, o.mDriver)
    SYNC_PROP_SET(clips, o.Clips(), )
    SYNC_PROP(clip1, o.mClip1)
    SYNC_PROP(clip2, o.mClip2)
    SYNC_PROP(filter_group, o.mFilterGroup)
    SYNC_PROP(transition, o.mTransition)
    SYNC_PROP(cycle_transition, o.mCycleTransition)
    SYNC_PROP_SET(move_self, o.MovingSelf(), o.SetMoveSelf(_val.Int()))
    SYNC_PROP_SET(teleport_to, o.mTeleportTo.Ptr(), o.TeleportTo(_val.Obj<Waypoint>()))
    SYNC_PROP(walk_path, o.mWalkPath)
    SYNC_PROP_SET(dist_map, o.mShowDistMap, o.SetDistMap(_val.Sym()))
    SYNC_PROP(zero_travel, o.mZeroTravel)
    SYNC_PROP(metronome, o.mMetronome)
    SYNC_PROP(show_footextents, o.mShowFootExtents)
END_CUSTOM_PROPSYNC

BEGIN_SAVES(CharacterTest)
    SAVE_REVS(15, 0)
    bs << mDriver;
    bs << mClip1;
    bs << mClip2;
    bs << mTeleportTo;
    bs << mWalkPath;
    bs << mShowDistMap;
    bs << mTransition;
    bs << mCycleTransition;
    bs << unk94;
    bs << mMetronome;
    bs << mZeroTravel;
    bs << mShowScreenSize;
    bs << mShowFootExtents;
END_SAVES

void CharacterTest::TeleportTo(Waypoint *wp) {
    if (wp)
        mMe->Teleport(wp);
}
