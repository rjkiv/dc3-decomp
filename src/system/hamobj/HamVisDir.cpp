#include "hamobj/HamVisDir.h"

#include "HamGameData.h"
#include "gesture/DepthBuffer3D.h"
#include "Pose.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

PoseOwner::PoseOwner() : pose(0), holder(0), in_pose(0) {}

PoseOwner::~PoseOwner() {
    MILO_ASSERT(!pose || pose != holder, 0x1C);
    delete pose;
    delete holder;
}

HamVisDir::HamVisDir()
    : mFreestyleFilter(0), mRunning(0), mPlayer1GestureRight(this),
      mPlayer1GestureLeft(this), mPlayer2GestureRight(this), mPlayer2GestureLeft(this),
      mMiloManualFrame(1), mGroove(0) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (!handle.HasCallback(this)) {
        handle.AddCallback(this);
    }
    for (int i = 0; i < 2; i++) {
        mPoseSquat[i].name = MakeString("pose_squat_%i", i);
        mPoseSquat[i].pose = new Pose(10, (Pose::ScoreMode)1);
        mPoseSquat[i].pose->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeRight, 0, 0.9)
        );
        mPoseSquat[i].pose->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeLeft, 0, 0.9)
        );
        mPoseSquat[i].pose->AddElement(new CamDistancePoseElement(1, 2));

        mPoseSquat[i].holder = new Pose(10, (Pose::ScoreMode)0);
        mPoseSquat[i].holder->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeRight, 0, 0.94)
        );
        mPoseSquat[i].holder->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeLeft, 0, 0.94)
        );
        mPoseSquat[i].holder->AddElement(new CamDistancePoseElement(1, 1.6f));

        mPoseY[i].name = MakeString("pose_y_%i", i);
        mPoseY[i].pose = new Pose(10, (Pose::ScoreMode)1);
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerRight, Vector3(1, 1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerLeft, Vector3(-1, 1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperRight, Vector3(1, 1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperLeft, Vector3(-1, 1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerRight, Vector3(1, -1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerLeft, Vector3(-1, -1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperRight, Vector3(1, -1, 0), 0.7853981852531433, 1
        ));
        mPoseY[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperLeft, Vector3(-1, -1, 0), 0.7853981852531433, 1
        ));

        mPoseY[i].holder = new Pose(10, (Pose::ScoreMode)0);
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerRight, Vector3(1, 1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerLeft, Vector3(-1, 1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperRight, Vector3(1, 1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperLeft, Vector3(-1, 1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerRight, Vector3(1, -1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerLeft, Vector3(-1, -1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperRight, Vector3(1, -1, 0), 1.047197580337524, 1
        ));
        mPoseY[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperLeft, Vector3(-1, -1, 0), 1.047197580337524, 1
        ));
    }
}

HamVisDir::~HamVisDir() {
    RELEASE(mFreestyleFilter);
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
}

BEGIN_HANDLERS(HamVisDir)
    HANDLE_SUPERCLASS(SkeletonDir)
END_HANDLERS

BEGIN_PROPSYNCS(HamVisDir)
    SYNC_PROP(milo_manual_frame, mMiloManualFrame)
    SYNC_PROP(player1_right, mPlayer1GestureRight)
    SYNC_PROP(player1_left, mPlayer1GestureLeft)
    SYNC_PROP(player2_right, mPlayer2GestureRight)
    SYNC_PROP(player2_left, mPlayer2GestureLeft)
    SYNC_PROP(in_pose_squat_0, mPoseSquat[0].in_pose)
    SYNC_PROP(in_pose_y_0, mPoseY[0].in_pose)
    SYNC_PROP(in_pose_squat_1, mPoseSquat[1].in_pose)
    SYNC_PROP(in_pose_y_1, mPoseY[1].in_pose)
    SYNC_SUPERCLASS(SkeletonDir)
END_PROPSYNCS

BEGIN_SAVES(HamVisDir)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(SkeletonDir)
    bs << mMiloManualFrame;
    bs << mPlayer1GestureLeft << mPlayer1GestureRight;
    bs << mPlayer2GestureLeft << mPlayer2GestureRight;
END_SAVES

BEGIN_COPYS(HamVisDir)
    COPY_SUPERCLASS(SkeletonDir)
    CREATE_COPY(HamVisDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMiloManualFrame)
        COPY_MEMBER(mPlayer1GestureLeft)
        COPY_MEMBER(mPlayer1GestureRight)
        COPY_MEMBER(mPlayer2GestureLeft)
        COPY_MEMBER(mPlayer2GestureRight)
    END_COPYING_MEMBERS
END_COPYS

void HamVisDir::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

INIT_REVS(13, 0)

void HamVisDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(13, 0);
    if (d.rev < 1) {
        PanelDir::PreLoad(bs);
    } else {
        SkeletonDir::PreLoad(bs);
    }
    PUSH_REVS(d);
}

void HamVisDir::PostLoad(BinStream &bs) {
    POP_REVS(bs);
    if (d.rev < 1) {
        PanelDir::PostLoad(bs);
    } else {
        SkeletonDir::PostLoad(bs);
    }
    if (d.rev > 1 && d.rev < 3) {
        int x;
        d >> x;
        d >> x;
    }
    if (d.rev > 2 && d.rev < 6) {
        int x;
        d >> x;
    }
    if (d.rev > 3 && d.rev < 0xD) {
        ObjPtrList<RndDrawable> list(this);
        d >> list;
        Vector3 v;
        d >> v;
    }
    if (d.rev > 4 && d.rev < 0xD) {
        ObjPtr<RndCam> cam(this);
        d >> cam;
        int x;
        d >> x;
    }
    if (d.rev > 6) {
        d >> mMiloManualFrame;
    }
    if (d.rev > 7) {
        d >> mPlayer1GestureLeft;
        d >> mPlayer1GestureRight;
        d >> mPlayer2GestureLeft;
        d >> mPlayer2GestureRight;
    }
    if (d.rev < 0xD) {
        if (d.rev > 8) {
            ObjPtr<RndDrawable> draw(this);
            d >> draw;
            d >> draw;
            d >> draw;
            d >> draw;
        }
        if (d.rev > 9) {
            ObjPtr<RndAnimatable> anim(this);
            d >> anim;
        }
        if (d.rev > 10) {
            ObjPtr<RndCam> cam(this);
            d >> cam;
        }
        if (d.rev > 11) {
            ObjPtr<RndAnimatable> anim(this);
            d >> anim;
        }
    }
}

void HamVisDir::Enter() {
    PanelDir::Enter();
    mRunning = TheLoadMgr.EditMode();
    if (!TheLoadMgr.EditMode() && TheGestureMgr) {
        mFreestyleFilter =
            ObjectDir::Main()->Find<FreestyleMotionFilter>("freestyle_filter", false);
        if (!mFreestyleFilter) {
            mFreestyleFilter = new FreestyleMotionFilter();
            mFreestyleFilter->SetName("freestyle_filter", Main());
        }
        mFreestyleFilter->Deactivate();
    }
}

void HamVisDir::PostUpdate(const SkeletonUpdateData *data) {
    if (data) {
        if (TheLoadMgr.EditMode()) {
            MILO_ASSERT(TheGameData, 0xEE);
            TheGameData->AutoAssignSkeletons(data);
        }
        if ((!mFreestyleFilter || mFreestyleFilter->IsActive()) || mRunning) {
            if (mFreestyleFilter) {
                mFreestyleFilter->UpdateFilters(*data);
            }
            if (mRunning) {
                bool b4 = false;
                for (int i = 0; i < 2; i++) {
                    HamPlayerData *player_data = TheGameData->Player(i);
                    MILO_ASSERT(player_data, 0x101);
                    if (player_data->IsPlaying()) {
                        const Skeleton *cur = data->mPlayerSkeletons[i];
                        if (cur && cur->IsTracked()) {
                            int i8 = i;
                            if (i == 0) {
                                b4 = true;
                            } else if (!b4) {
                                i8 = 0;
                            }
                            UpdateGestureFilter(*cur, i8);
                            mPoseSquat[i].pose->Update(*cur);
                            mPoseSquat[i].holder->Update(*cur);
                            mPoseY[i].pose->Update(*cur);
                            mPoseY[i].holder->Update(*cur);
                            CheckPose(i, mPoseSquat[i]);
                            CheckPose(i, mPoseY[i]);
                        }
                    }
                }
            }
        }
    }
}

void HamVisDir::SetGrooviness(float groove) {
    mGroove = (groove - 0.5f) * (2.0f / 3.0f);
    ClampEq<float>(mGroove, 0.0f, 1.0f);
    for (ObjDirItr<DepthBuffer3D> it(this, true); it != nullptr; ++it) {
        it->SetGrooviness(groove);
    }
}

void HamVisDir::CheckPose(int player, PoseOwner &poseOwner) {
    bool flag = false;
    if (poseOwner.in_pose || poseOwner.pose->CurrentScore() <= 9.0f / 10.0f) {
        if (poseOwner.in_pose && poseOwner.holder->CurrentScore() <= 9.0f / 10.0f) {
            flag = true;
            poseOwner.in_pose = false;
        }
    } else {
        flag = true;
        poseOwner.in_pose = true;
        TheGameData->HandlePoseFound(player);
    }
    if (flag) {
        static Message msg("whatever");
        msg.SetType(MakeString("%s_state_changed", poseOwner.name));
        Export(msg, true);
    }
}

void HamVisDir::Run(bool run) { mRunning = run; }

void HamVisDir::CalcArmLengths(std::vector<float> &lengths, const Skeleton &skel) {
    struct {
        SkeletonBone upper;
        SkeletonBone lower;
    } arms[2] = { { kBoneArmUpperLeft, kBoneArmLowerLeft },
                  { kBoneArmUpperRight, kBoneArmLowerRight } };
    for (unsigned int i = 0; i < 2; i++) { // it actually is unsigned here.... what
        MILO_ASSERT_RANGE(arms[i].upper, 0, kNumJoints, 0x12d);
        MILO_ASSERT_RANGE(arms[i].lower, 0, kNumJoints, 0x12e);
        lengths[i] = skel.BoneLength(arms[i].upper, kCoordCamera)
            + skel.BoneLength(arms[i].lower, kCoordCamera);
    }
}

void HamVisDir::UpdateGestureFilter(Skeleton const &skeleton, int player) {
    RndAnimatable *leftPlayer;
    RndAnimatable *rightPlayer;
    if (player == 0) {
        leftPlayer = mPlayer1GestureLeft;
        rightPlayer = mPlayer1GestureRight;
    } else {
        leftPlayer = mPlayer2GestureLeft;
        rightPlayer = mPlayer2GestureRight;
    }

    if ((leftPlayer || rightPlayer) && (!TheLoadMgr.EditMode() || !mMiloManualFrame)) {
        std::vector<float> armLengths(2, 0.0f);
        CalcArmLengths(armLengths, skeleton);
        std::vector<float> shoulders(2, 0.0f);
        shoulders[0] = skeleton.TrackedJoints()[kJointShoulderLeft].mPos[0].y;
        shoulders[1] = skeleton.TrackedJoints()[kJointShoulderRight].mPos[0].y;
        std::vector<float> wrists(2, 0.0f);
        wrists[0] = skeleton.TrackedJoints()[kJointWristLeft].mPos[0].y;
        wrists[1] = skeleton.TrackedJoints()[kJointWristRight].mPos[0].y;
        for (int i = 0; i < 2; i++) {
            RndAnimatable *animatable = (i == 0) ? leftPlayer : rightPlayer;
            float val = mGroove * 100.0f;
            if (animatable) {
                animatable->SetFrame(val, 1.0f);
            }
        }
    }
}
