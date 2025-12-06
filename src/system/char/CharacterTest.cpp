#include "char/CharacterTest.h"
#include "Character.h"
#include "char/CharForeTwist.h"
#include "char/CharUpperTwist.h"
#include "char/CharUtl.h"
#include "char/ClipGraphGen.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Graph.h"
#include "rndobj/Overlay.h"
#include "utl/Symbol.h"
#include "rndobj/Utl.h"

Hmx::Object *gClick;

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

void CharacterTest::Walk() {
    if (!mWalkPath.empty()) {
        std::vector<Waypoint *> vec;
        for (ObjPtrList<Waypoint>::iterator it = mWalkPath.begin(); it != mWalkPath.end();
             ++it) {
            vec.push_back(*it);
        }
    }
}

void CharacterTest::Recenter() {
    Transform xfm;
    xfm.Reset();
    mMe->SetLocalXfm(xfm);
    if (mMe->BoneServo()) {
        mMe->BoneServo()->SetRegulateWaypoint(nullptr);
    }
}

void CharacterTest::SetMoveSelf(bool b) {
    if (mMe->BoneServo()) {
        mMe->BoneServo()->SetMoveSelf(b);
    }
}

DataNode CharacterTest::OnGetFilteredClips(DataArray *arr) {
    int count = 0;
    ObjectDir *clipDir = Clips();
    if (clipDir) {
        for (ObjDirItr<CharClip> it(clipDir, true); it != nullptr; ++it) {
            count++;
        }
    }
    DataArrayPtr ptr;
    ptr->Resize(count + 1);
    ptr->Node(0) = NULL_OBJ;
    if (clipDir) {
        int idx = 1;
        for (ObjDirItr<CharClip> it(clipDir, true); it != nullptr; ++it) {
            if (!mFilterGroup || mFilterGroup->FindClip(it->Name())) {
                ptr->Node(idx++) = &*it;
            }
        }
        ptr->Resize(idx);
        ptr->SortNodes(0);
    }
    return ptr;
}

float CharacterTest::UpdateOverlay(RndOverlay *o, float f) {
    if (unk98)
        unk98->Draw(40.0f, 40.0f, mDriver);
    return f;
}

void CharacterTest::AddDefaults() {
    static Symbol hand("hand");
    static Symbol twist1("twist1");
    static Symbol twist2("twist2");
    static Symbol offset("offset");
    static Symbol upper_arm("upper_arm");
    if (!mMe->Driver())
        mMe->New<CharDriver>("main.drv");
    if (!mMe->BoneServo()) {
        if (!mMe->Find<CharServoBone>("bone.servo", false)) {
            mMe->New<CharServoBone>("bone.servo");
        }
        mMe->Driver()->SetBones(mMe->Find<CharBonesObject>("bone.servo", true));
    }
    if (!mMe->Find<CharForeTwist>("foreTwist_L.ik", false)) {
        RndTransformable *lhand = CharUtlFindBoneTrans("bone_L-hand", mMe);
        if (lhand) {
            RndTransformable *ltwist2 = CharUtlFindBoneTrans("bone_L-foreTwist2", mMe);
            if (ltwist2) {
                CharForeTwist *ltwist = mMe->New<CharForeTwist>("foreTwist_L.ik");
                ltwist->SetProperty(hand, lhand);
                ltwist->SetProperty(twist2, ltwist2);
                ltwist->SetProperty(offset, 90);
            }
        }
    }
    if (!mMe->Find<CharForeTwist>("foreTwist_R.ik", false)) {
        RndTransformable *rtwist2;
        RndTransformable *rhand;
        rhand = CharUtlFindBoneTrans("bone_R-hand", mMe);
        if (rhand) {
            rtwist2 = CharUtlFindBoneTrans("bone_R-foreTwist2", mMe);
            if (rtwist2) {
                CharForeTwist *rtwist = mMe->New<CharForeTwist>("foreTwist_R.ik");
                rtwist->SetProperty(hand, rhand);
                rtwist->SetProperty(twist2, rtwist2);
                rtwist->SetProperty(offset, -90);
            }
        }
    }
    RndTransformable *utwist2;
    RndTransformable *utwist1;
    RndTransformable *uarm;
    CharUpperTwist *twist;
    if (!mMe->Find<CharUpperTwist>("upperTwist_L.ik", false)) {
        utwist1 = CharUtlFindBoneTrans("bone_L-upperTwist1", mMe);
        if (utwist1) {
            utwist2 = CharUtlFindBoneTrans("bone_L-upperTwist2", mMe);
            if (utwist2) {
                uarm = CharUtlFindBoneTrans("bone_L-upperArm", mMe);
                if (uarm) {
                    twist = mMe->New<CharUpperTwist>("upperTwist_L.ik");
                    twist->SetProperty(twist1, utwist1);
                    twist->SetProperty(twist2, utwist2);
                    twist->SetProperty(upper_arm, uarm);
                }
            }
        }
    }
    if (!mMe->Find<CharUpperTwist>("upperTwist_R.ik", false)) {
        utwist1 = CharUtlFindBoneTrans("bone_R-upperTwist1", mMe);
        if (utwist1) {
            utwist2 = CharUtlFindBoneTrans("bone_R-upperTwist2", mMe);
            if (utwist2) {
                uarm = CharUtlFindBoneTrans("bone_R-upperArm", mMe);
                if (uarm) {
                    twist = mMe->New<CharUpperTwist>("upperTwist_R.ik");
                    twist->SetProperty(twist1, utwist1);
                    twist->SetProperty(twist2, utwist2);
                    twist->SetProperty(upper_arm, uarm);
                }
            }
        }
    }
}

void CharacterTest::Draw() {
    if (mDriver && (mClip1 || mClip2))
        mDriver->Highlight();
    RndTransformable *trans = CharUtlFindBoneTrans("bone_head", mMe);
    if (!trans)
        trans = mMe;
    if (mShowScreenSize) {
        UtilDrawString(
            MakeString(
                "lod %d %.3f", mMe->LastLod(), mMe->ComputeScreenSize(RndCam::Current())
            ),
            trans->WorldXfm().v,
            Hmx::Color(1.0f, 1.0f, 1.0f)

        );
    }
}

bool CharacterTest::MovingSelf() const {
    return mMe->BoneServo() ? mMe->BoneServo()->mMoveSelf : false;
}

void CharacterTest::SetDistMap(Symbol s) {
    static Symbol none("none");
    static Symbol nodes("nodes");
    static Symbol raw("raw");
    mShowDistMap = s;
    RELEASE(unk98);
    if (s != none) {
        mOverlay->SetCallback(this);
        mOverlay->SetShowing(true);
        if (mClip1 && mClip2 && Clips()) {
            if (s == raw) {
                unk98 = new ClipDistMap(mClip1, mClip2, 1, 1, 3, nullptr);
                unk98->FindDists(0, nullptr);
            } else {
                ClipGraphGenerator gen;
                unk98 = gen.GeneratePair(mClip1, mClip2, nullptr, nullptr);
            }
        }
    }
}
