#include "hamobj/HamVisDir.h"

#include "HamGameData.h"
#include "gesture/DepthBuffer3D.h"
#include "Pose.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"

PoseOwner::PoseOwner() : pose(0), holder(0), in_pose(0) {}

PoseOwner::~PoseOwner() {
    MILO_ASSERT(!pose || pose != holder, 0x1C);
    delete pose;
    delete holder;
}

HamVisDir::HamVisDir()
    : mFilter(0), mRunning(0), unk2d8(0), unk2dc(0), mPlayer1Right(this),
      mPlayer1Left(this), mPlayer2Right(this), mPlayer2Left(this), mMiloManualFrame(1),
      unk334(0) {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (!handle.HasCallback(this)) {
        handle.AddCallback(this);
    }
    for (int i = 0; i < 2; i++) {
        mSquatPoses[i].name = MakeString("pose_squat_%i", i);
        mSquatPoses[i].pose = new Pose(10, (Pose::ScoreMode)1);
        mSquatPoses[i].pose->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeRight, 0, 0.9)
        );
        mSquatPoses[i].pose->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeLeft, 0, 0.9)
        );
        mSquatPoses[i].pose->AddElement(new CamDistancePoseElement(2, 1));

        mSquatPoses[i].holder = new Pose(10, (Pose::ScoreMode)0);
        mSquatPoses[i].holder->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeRight, 0, 0.94)
        );
        mSquatPoses[i].holder->AddElement(
            new JointDistPoseElement(kJointHead, kJointKneeLeft, 0, 0.94)
        );
        mSquatPoses[i].holder->AddElement(new CamDistancePoseElement(1.6, 1));

        mYPoses[i].name = MakeString("pose_y_%i", i);
        mYPoses[i].pose = new Pose(10, (Pose::ScoreMode)1);
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerRight, Vector3(1, 1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerLeft, Vector3(-1, 1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperRight, Vector3(1, 1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperLeft, Vector3(-1, 1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerRight, Vector3(1, -1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerLeft, Vector3(-1, -1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperRight, Vector3(1, -1, 0), 0.7853981852531433, 1
        ));
        mYPoses[i].pose->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperLeft, Vector3(-1, -1, 0), 0.7853981852531433, 1
        ));

        mYPoses[i].holder = new Pose(10, (Pose::ScoreMode)0);
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerRight, Vector3(1, 1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmLowerLeft, Vector3(-1, 1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperRight, Vector3(1, 1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneArmUpperLeft, Vector3(-1, 1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerRight, Vector3(1, -1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegLowerLeft, Vector3(-1, -1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperRight, Vector3(1, -1, 0), 1.047197580337524, 0
        ));
        mYPoses[i].holder->AddElement(new BoneAngleRangePoseElement(
            kBoneLegUpperLeft, Vector3(-1, -1, 0), 1.047197580337524, 0
        ));
    }
}

HamVisDir::~HamVisDir() {
    RELEASE(mFilter);
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
}

void HamVisDir::SetGrooviness(float groove) {
    unk334 = Clamp<float>(0.0, 1.0, (groove - 0.5f) * 2/3);
    auto itr = ObjDirItr<DepthBuffer3D>(this, true);
    while (itr != nullptr) {
        itr->SetGrooviness(groove);
        ++itr;
    }
}

void HamVisDir::CheckPose(int i1, PoseOwner &po) {
    bool flag = false;
    if (po.in_pose != false || po.pose->CurrentScore() <= 9.0f/10.0f) {
        if (po.in_pose != false && po.holder->CurrentScore() <= 9.0f/10.0f) {
            flag = true;
            po.in_pose = false;
        }
    }
    else {
        flag = true;
        po.in_pose = true;
        TheGameData->HandlePoseFound(i1);
    }
    if (flag != false) {
        static Message msg("whatever");
        msg.SetType(MakeString("%s_state_changed", po.name));
        Export(msg, true);
    }
}

void HamVisDir::Enter() {
    PanelDir::Enter();
    mRunning = TheLoadMgr.EditMode();
    if (TheLoadMgr.EditMode() == 0 && TheGestureMgr != nullptr) {
        auto freestyle_filter = ObjectDir::Main()->Find<FreestyleMotionFilter>("freestyle_filter", false);
        mFilter = freestyle_filter;
        if (freestyle_filter == nullptr) {
            freestyle_filter = new FreestyleMotionFilter();
            mFilter = freestyle_filter;
        }
        freestyle_filter->SetName("freestyle_filter", Main());
    }
    mFilter->Deactivate();
}

void HamVisDir::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
}

BEGIN_HANDLERS(HamVisDir)
    HANDLE_SUPERCLASS(SkeletonDir)
END_HANDLERS

BEGIN_PROPSYNCS(HamVisDir)
    SYNC_PROP(milo_manual_frame, mMiloManualFrame)
    SYNC_PROP(player1_right, mPlayer1Right)
    SYNC_PROP(player1_left, mPlayer1Left)
    SYNC_PROP(player2_right, mPlayer2Right)
    SYNC_PROP(player2_left, mPlayer2Left)
    SYNC_PROP(in_pose_squat_0, mSquatPoses[0].in_pose)
    SYNC_PROP(in_pose_y_0, mYPoses[0].in_pose)
    SYNC_PROP(in_pose_squat_1, mSquatPoses[1].in_pose)
    SYNC_PROP(in_pose_y_1, mYPoses[1].in_pose)
    SYNC_SUPERCLASS(SkeletonDir)
END_PROPSYNCS

BEGIN_SAVES(HamVisDir)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(SkeletonDir)
    bs << mMiloManualFrame;
    bs << mPlayer1Left << mPlayer1Right;
    bs << mPlayer2Left << mPlayer2Right;
END_SAVES

BEGIN_COPYS(HamVisDir)
    COPY_SUPERCLASS(SkeletonDir)
    CREATE_COPY(HamVisDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMiloManualFrame)
        COPY_MEMBER(mPlayer1Left)
        COPY_MEMBER(mPlayer1Right)
        COPY_MEMBER(mPlayer2Left)
        COPY_MEMBER(mPlayer2Right)
    END_COPYING_MEMBERS
END_COPYS

void HamVisDir::Run(bool run) { mRunning = run; }

void HamVisDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(13, 0);
    if (d.rev < 1) {
        PanelDir::PreLoad(bs);
    } else {
        SkeletonDir::PreLoad(bs);
    }
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void HamVisDir::PostLoad(BinStream &bs) {

}