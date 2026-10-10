#include "char/CharacterTest.h"
#include "Character.h"
#include "char/CharClip.h"
#include "char/CharClipDriver.h"
#include "char/CharForeTwist.h"
#include "char/CharUpperTwist.h"
#include "char/CharUtl.h"
#include "char/ClipGraphGen.h"
#include "math/Utl.h"
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
      mShowFootExtents(0), mInternalTransition(0), mDistMap(0),
      mOverlay(RndOverlay::Find("char_test")) {
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
    bs << mInternalTransition;
    bs << mMetronome;
    bs << mZeroTravel;
    bs << mShowScreenSize;
    bs << mShowFootExtents;
END_SAVES

INIT_REVS(15, 0)

BEGIN_LOADS(CharacterTest)
    LOAD_REVS(bs)
    if (d.rev > 15) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d",
            PathName(mMe),
            "CharacterTesting",
            d.rev,
            gRev
        );
    }
    if (d.altRev > 0) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d",
            PathName(mMe),
            "CharacterTesting",
            d.altRev,
            gAltRev
        );
    }
    if (d.rev != 0xD)
        mDriver.Load(d.stream, false, mMe);

    if (Clips()) {
        mClip1.Load(d.stream, true, Clips());
        mClip2.Load(d.stream, true, Clips());
    } else {
        Symbol s;
        d >> s;
        d >> s;
        mClip1 = nullptr;
        mClip2 = nullptr;
    }
    d >> mTeleportTo;
    mWalkPath.Load(d.stream, false, nullptr, true);
    d >> mShowDistMap;
    d >> mTransition;
    d >> mCycleTransition;
    d >> mInternalTransition;
    if (d.rev < 10) {
        int i;
        d >> i;
    }
    d >> mMetronome;
    d >> mZeroTravel;
    d >> mShowScreenSize;
    if (d.rev < 0xC) {
        Symbol ss;
        d >> ss;
    }
    d >> mShowFootExtents;
    if (d.rev < 0xF) {
        bool b;
        int i;
        d >> b;
        d >> i;
    }
    if (d.rev > 6 && d.rev < 11) {
        Symbol ss;
        int i;
        d >> ss;
        d >> i;
    }
    if (d.rev > 8 && d.rev < 11) {
        Symbol ss;
        d >> ss;
    }
    if (!mDriver) {
        mDriver = mMe->Find<CharDriver>("main.drv", false);
    }
END_LOADS

void CharacterTest::TeleportTo(Waypoint *wp) {
    if (wp)
        mMe->Teleport(wp);
}

void CharacterTest::Walk() {
    if (!mWalkPath.empty()) {
        std::vector<Waypoint *> vec;
        FOREACH (it, mWalkPath) {
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
    if (mDistMap)
        mDistMap->Draw(40.0f, 40.0f, mDriver);
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
        mMe->Driver()->SetBones(mMe->Find<CharBonesObject>("bone.servo"));
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
    RELEASE(mDistMap);
    if (s != none) {
        mOverlay->SetCallback(this);
        mOverlay->SetShowing(true);
        if (mClip1 && mClip2 && Clips()) {
            if (s == raw) {
                mDistMap = new ClipDistMap(mClip1, mClip2, 1, 1, 3, nullptr);
                mDistMap->FindDists(0, nullptr);
            } else {
                ClipGraphGenerator gen;
                mDistMap = gen.GeneratePair(mClip1, mClip2, nullptr, nullptr);
            }
        }
    }
}

void CharacterTest::Poll() {
    if (Clips() && mClip1) {
        if (!gClick) {
            ObjectDir *clickdir = DirLoader::LoadObjects(
                MakeString("%s/char/chartest.milo", FileSystemRoot()), nullptr, nullptr
            );
            gClick = clickdir->Find<Hmx::Object>("click_hi.cue");
        }
        float beat = TheTaskMgr.Beat();
        float deltabeat = TheTaskMgr.DeltaBeat();
        if (mMetronome) {
            if (floorf(beat - deltabeat) + 1.0f == floorf(beat)) {
                static Message playMsg("play");
                gClick->Handle(playMsg, true);
            }
        }
        CharClipDriver *drivs = mDriver->First();
        if (!drivs) {
            PlayNew();
        } else {
            if (!mClip2) {
                if (drivs->GetClip() != mClip1) {
                    PlayNew();
                }
            } else if (drivs->GetClip() != mClip1 && drivs->GetClip() != mClip2
                       || (drivs->GetClip() == mClip2 && drivs->mBeat > mPopFrame)) {
                PlayNew();
            }
        }
        if (mZeroTravel) {
            Transform xfm = mMe->LocalXfm();
            xfm.v.Zero();
            mMe->SetLocalXfm(xfm);
            if (mMe->BoneServo()) {
                mMe->BoneServo()->SetRegulateWaypoint(nullptr);
            }
            Recenter();
        }
    }
}

void CharacterTest::Sync() {
    mInternalTransition = 0;
    if (!mDriver || (mClip1 && mClip1->Dir() != Clips())) {
        mClip1 = nullptr;
    }
    if (!mClip1 || !mDriver || (mClip2 && mClip2->Dir() != Clips())) {
        mClip2 = nullptr;
    }
    if (!mDriver || (mFilterGroup && mFilterGroup->Dir() != Clips())) {
        mFilterGroup = nullptr;
    }
    if (mMe->BoneServo()) {
        mMe->BoneServo()->SetRegulateWaypoint(nullptr);
    }
    if (mClip1 || mClip2) {
        mOverlay->SetCallback(this);
        mOverlay->SetShowing(true);
    } else {
        mOverlay->SetCallback(nullptr);
        mOverlay->SetShowing(false);
    }
    static Symbol none("none");
    // this is absolutely positively 100% terrible, probably a fakematch
    // but seeing as what's being passed in is basically a UUID...i guess it's fine
    RndGraph::Get(reinterpret_cast<void *>((char *)this + 0x13F920))->Reset();
    if (!mClip2) {
        mTransition = 0;
    }
    if (mDistMap && (mDistMap->ClipA() != mClip1 || mDistMap->ClipB() != mClip2)) {
        SetDistMap(mShowDistMap);
    }
    if (mClip1) {
        float bps = mClip1->AverageBeatsPerSecond();
        if (mClip2) {
            CharClip::NodeVector *nodes = mClip1->GetTransitions().FindNodes(mClip2);
            if (nodes) {
                mTransition = Clamp(0, nodes->size - 1, mTransition);
            } else {
                mTransition = 0;
            }
            MinEq(bps, mClip2->AverageBeatsPerSecond());
        }
    }
    if (mClip1 || mClip2) {
        mDriver->Enter();
    }
    mInternalTransition = 0;
}

void CharacterTest::PlayNew() {
    mPopFrame = kHugeFloat;
    if (mClip1) {
        CharClipDriver *ccd = mDriver->Play(mClip1, 2, -1, kHugeFloat, 0);
        if (mClip2) {
            mPopFrame = mClip2->EndBeat();
            CharClip::NodeVector *nodes = mClip1->GetTransitions().FindNodes(mClip2);
            if (nodes) {
                ccd->mPlayFlags = ccd->mPlayFlags & 0xFFFF0FFF;
                mInternalTransition = mInternalTransition % nodes->size;
                int idx;
                if (mCycleTransition) {
                    idx = mInternalTransition++;
                } else {
                    idx = mTransition;
                }
                CharGraphNode *graphNode = &nodes->nodes[idx];
                MaxEq(ccd->mBeat, graphNode->curBeat - 4.0f);
                mDriver->Play(
                    mClip2, 1, -1, graphNode->nextBeat, graphNode->curBeat - ccd->mBeat
                );
                MinEq(mPopFrame, graphNode->nextBeat + 4.0f);
            } else {
                mDriver->Play(mClip2, 4, -1, kHugeFloat, 0);
            }
        } else {
            ccd->mPlayFlags = ccd->mPlayFlags & 0xFFFF0F0F | 0x20;
        }
    }
}
