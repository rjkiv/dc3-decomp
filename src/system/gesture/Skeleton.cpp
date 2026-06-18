#include "gesture/Skeleton.h"
#include "ArchiveSkeleton.h"
#include "IdentityInfo.h"
#include "gesture/GestureMgr.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/SkeletonHistory.h"
#include "gesture/JointUtl.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Std.h"
#include "xdk/NUI.h"
#include "xdk/XAPILIB.h"
#include "xdk/nui/nuiidentity.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/winerror.h"
#include <cmath>
#include <cstddef>
#include <cstring>

Vector3DESmoother SkeletonFrame::sUpVectorSmoother;

#pragma region SkeletonFrame

float SkeletonFrame::TiltAngle() const { return (PI / 2) - (float)atan2(unk8.y, unk8.z); }

void SkeletonFrame::Init() {
    static Symbol kinect("kinect");
    static Symbol up_vector_smoothing("up_vector_smoothing");
    static Symbol smoothing("smoothing");
    static Symbol trend("trend");
    DataArray *cfg = SystemConfig(kinect, up_vector_smoothing);
    sUpVectorSmoother.SetSmoothParameters(
        cfg->FindFloat(smoothing), cfg->FindFloat(trend)
    );
    sUpVectorSmoother.ForceValue(Vector3(0, 1, 0));
}

void SkeletonFrame::Create(const NUI_SKELETON_FRAME &nui_frame, int i2) {
    unk0 = nui_frame.dwFrameNumber;
    mElapsedMs = i2;
    unk8 = sUpVectorSmoother.Value();
    unk18.Set(
        nui_frame.vFloorClipPlane.x,
        nui_frame.vFloorClipPlane.y,
        nui_frame.vFloorClipPlane.z,
        nui_frame.vFloorClipPlane.w
    );
}

#pragma endregion
#pragma region Skeleton

Skeleton::Skeleton() : mTracking(kSkeletonNotTracked), mTrackingID(-1), unkac4(0) {
    Init();
}

void Skeleton::JointPos(SkeletonCoordSys cs, SkeletonJoint joint, Vector3 &pos) const {
    MILO_ASSERT((0) <= (cs) && (cs) < (kNumCoordSys), 0xDA);
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0xDB);
    pos = mTrackedJoints[joint].mJointPos[cs];
}

bool Skeleton::Displacement(
    const SkeletonHistory *history,
    SkeletonCoordSys cs,
    SkeletonJoint joint,
    int i4,
    Vector3 &disp,
    int &iref
) const {
    ArchiveSkeleton archiveSkeleton;
    if (PrevTrackedSkeleton(history, i4, iref, archiveSkeleton)) {
        Vector3 v3;
        archiveSkeleton.JointPos(cs, joint, v3);
        Subtract(mTrackedJoints[joint].mJointPos[cs], v3, disp);
        return true;
    } else {
        disp.Zero();
        return false;
    }
}

JointConfidence Skeleton::JointConf(SkeletonJoint joint) const {
    MILO_ASSERT((0) <= (joint) && (joint) < (kNumJoints), 0xE1);
    return mTrackedJoints[joint].mJointConf;
}

bool Skeleton::IsTracked() const { return mTracking == kSkeletonTracked; }
int Skeleton::QualityFlags() const { return mQualityFlags; }
int Skeleton::ElapsedMs() const { return mElapsedMs; }

void Skeleton::CameraToPlayerXfm(SkeletonCoordSys cs, Transform &playerXfm) const {
    MILO_ASSERT((kCoordLeftArm) <= (cs) && (cs) < (kNumCoordSys), 0x127);
    playerXfm = mPlayerXfms[cs - 1];
}

void Skeleton::CamJointPositions(Vector3 *positions) const {
    for (int i = 0; i < kNumJoints; i++) {
        *positions++ = mTrackedJoints[i].mJointPos[kCoordCamera];
    }
}

void Skeleton::CamBoneLengths(float *lens) const {
    memcpy(lens, mCamBoneLengths, sizeof(mCamBoneLengths));
}

float Skeleton::BoneLength(SkeletonBone bone, SkeletonCoordSys cs) const {
    if (cs == kCoordCamera) {
        MILO_ASSERT((0) <= (bone) && (bone) < (kNumBones), 0x12F);
        return mCamBoneLengths[bone];
    } else
        return BaseSkeleton::BoneLength(bone, cs);
}

bool Skeleton::IsValid() const {
    if (mSkeletonIdx >= 0) {
        return TheGestureMgr->IsSkeletonValid(mSkeletonIdx);
    } else
        return false;
}

bool Skeleton::IsSitting() const {
    if (mSkeletonIdx >= 0) {
        return TheGestureMgr->IsSkeletonSitting(mSkeletonIdx);
    } else
        return false;
}

bool Skeleton::IsSideways() const {
    if (mSkeletonIdx >= 0) {
        return TheGestureMgr->IsSkeletonSideways(mSkeletonIdx);
    } else
        return false;
}

const TrackedJoint &Skeleton::HandJoint(SkeletonSide side) const {
    return mTrackedJoints[side == kSkeletonLeft ? kJointHandLeft : kJointHandRight];
}

const TrackedJoint &Skeleton::ElbowJoint(SkeletonSide side) const {
    return mTrackedJoints[side == kSkeletonLeft ? kJointElbowLeft : kJointElbowRight];
}

const TrackedJoint &Skeleton::ShoulderJoint(SkeletonSide side) const {
    return mTrackedJoints[side == kSkeletonLeft ? kJointShoulderLeft : kJointShoulderRight];
}

const TrackedJoint &Skeleton::HipJoint(SkeletonSide side) const {
    return mTrackedJoints[side == kSkeletonLeft ? kJointHipLeft : kJointHipRight];
}

const TrackedJoint &Skeleton::KneeJoint(SkeletonSide side) const {
    return mTrackedJoints[side == kSkeletonLeft ? kJointKneeLeft : kJointKneeRight];
}

void Skeleton::ScreenPos(SkeletonJoint joint, Vector2 &pos) const {
    if (mTracking == kSkeletonTracked) {
        JointScreenPos(mTrackedJoints[joint], pos);
    } else
        pos.Zero();
}

bool Skeleton::PrevTrackedSkeleton(
    const SkeletonHistory *history, int i2, int &iref, ArchiveSkeleton &archiveSkeleton
) const {
    MILO_ASSERT(history, 0x169);
    if (mTracking == kSkeletonTracked
        && history->PrevSkeleton(*this, i2, archiveSkeleton, iref)) {
        return archiveSkeleton.IsTracked();
    } else
        return false;
}

bool Skeleton::Velocity(
    const SkeletonHistory &history,
    SkeletonCoordSys cs,
    SkeletonJoint joint,
    int i4,
    Vector3 &velocity,
    int &iref
) const {
    if (Displacement(&history, cs, joint, i4, velocity, iref)) {
        velocity *= (1.0f / (iref * 0.001f));
        return true;
    } else {
        velocity.Zero();
        return false;
    }
}

void Skeleton::Init() {
    mTracking = kSkeletonNotTracked;
    mSkeletonIdx = -1;
    mQualityFlags = 0;
    unkab0.Zero();
    for (int i = 0; i < 5; i++) {
        mPlayerXfms[i].Reset();
    }
    for (int i = 0; i < kNumJoints; i++) {
        for (int j = 0; j < kNumCoordSys; j++) {
            mTrackedJoints[i].mJointPos[j].Zero();
        }
        mTrackedJoints[i].mJointConf = kConfidenceNotTracked;
        mTrackedJoints[i].unk60.Zero();
    }
    memset(mCamBoneLengths, 0, sizeof(mCamBoneLengths));
    mCamDisplacements.clear();
}

bool Skeleton::ProfileMatched() const {
    IdentityInfo *info = TheGestureMgr->GetIdentityInfo(mSkeletonIdx);
    return info ? info->ProfileMatched() : false;
}

int Skeleton::GetEnrollmentIndex() const {
    IdentityInfo *info = TheGestureMgr->GetIdentityInfo(mSkeletonIdx);
    return info ? info->EnrollmentIndex() : -1;
}

bool Skeleton::NeedIdentify() const {
    return GetEnrollmentIndex() == -1 || GetEnrollmentIndex() == -5;
}

void Skeleton::PostUpdate() {}

bool Skeleton::RequestIdentity() {
    MILO_ASSERT(!GestureMgr::sIdentityOpInProgress, 0x2A9);
    IdentityInfo *info = TheGestureMgr->GetIdentityInfo(mSkeletonIdx);
    if (info) {
        HRESULT hr = NuiIdentityIdentify(mTrackingID, 0, IdentityCallback, info);
        MILO_ASSERT(hr != E_INVALIDARG, 0x2B1);
        if (hr < 0) {
            if (hr != (HRESULT)0x8000000A) {
                return false;
            }
        }

        if (hr == 0) {
            info->SetUnk0(true);
        } else {
            GestureMgr::sIdentityOpInProgress = true;
        }
        return true;
    } else {
        return false;
    }
}

bool Skeleton::EnrollIdentity(int i) {
    MILO_ASSERT(!GestureMgr::sIdentityOpInProgress, 0x25d);
    IdentityInfo *info = TheGestureMgr->GetIdentityInfo(mSkeletonIdx);
    if (!info) {
        return false;
    }

    DWORD flags = 1;
    if (i != -1) {
        flags = 0x21;
    }
    if (i == -3) {
        i = -2;
    }

    HRESULT hr = NuiIdentityEnroll(mTrackingID, i, flags, IdentityCallback, info);
    MILO_ASSERT(hr != E_INVALIDARG, 0x26e);

    if (hr < 0) {
        if (hr != (HRESULT)0x8000000A) {
            return false;
        }
    }

    if (hr == 0) {
        info->SetUnk0(true);
    } else {
        GestureMgr::sIdentityOpInProgress = true;
    }
    return true;
}

int Skeleton::IdentityCallback(void *pvContext, NUI_IDENTITY_MESSAGE *pMessage) {
    IdentityInfo *info = static_cast<IdentityInfo *>(pvContext);
    MILO_ASSERT(pvContext != NULL, 0x280);
    MILO_ASSERT(pMessage != NULL, 0x281);
    switch (pMessage->MessageId) {
    case NUI_IDENTITY_MESSAGE_ID_FRAME_PROCESSED:
        break;
    case NUI_IDENTITY_MESSAGE_ID_COMPLETE:
        info->SetUnk0(true);
        info->SetProfileMatched(pMessage->Data.Complete.bProfileMatched != 0);
        if (info->EnrollmentIndex() != pMessage->Data.Complete.dwEnrollmentIndex) {
            info->SetEnrollmentIndex(pMessage->Data.Complete.dwEnrollmentIndex);
            info->SetUnk9(true);
        }
        break;
    default:
        MILO_ASSERT(false, 0x297);
        break;
    }

    if (!TheGestureMgr->IDEnabled()) {
        MILO_LOG("An identification operation that was in progress was canceled.\n");
        GestureMgr::sIdentityOpInProgress = false;
        return 0;
    }
    return 1;
}

void Skeleton::Poll(int skel_idx, SkeletonFrame const &skeletonFrame) {
    MILO_ASSERT_RANGE(skel_idx, 0, 6, 0x1f8);
    if (mSkeletonIdx != skel_idx && TheGestureMgr) {
        IdentityInfo *identityInfo = TheGestureMgr->GetIdentityInfo(skel_idx);
        MILO_ASSERT(identityInfo, 0x1fc);
        identityInfo->SetProfileMatched(false);
        identityInfo->SetEnrollmentIndex(-1);
    }

    mSkeletonIdx = skel_idx;
    mElapsedMs = skeletonFrame.mElapsedMs;
    const SkeletonData &data = skeletonFrame.mSkeletonDatas[skel_idx];
    mTrackingID = data.mTrackingID;
    unkab0 = data.unk2e0;
    mTracking = data.mTracking;
    if (mTracking != kSkeletonNotTracked) {
        if (mTracking == kSkeletonTracked) {
            mQualityFlags = data.mQualityFlags;
            if (TheGestureMgr) {
                IdentityInfo *identityInfo = TheGestureMgr->GetIdentityInfo(skel_idx);
                MILO_ASSERT(identityInfo, 0x211);
                if (identityInfo->EnrollmentIndex() != data.unk2dc) {
                    identityInfo->SetEnrollmentIndex(data.unk2dc);
                    identityInfo->SetUnk9(true);
                }
            }

            for (int i = 1; i < 6; i++) {
                BaseSkeleton::MakeCameraToPlayerXfm(
                    (SkeletonCoordSys)i,
                    mPlayerXfms[i - 1],
                    data.unk144,
                    skeletonFrame.unk8
                );
            }

            // somethin wrong in here but im fixing this later
            for (int i = 0; i < 0x910; i++) {
                TrackedJoint &joint = mTrackedJoints[i];
                for (int j = 0; j < kNumCoordSys; j++) {
                    if (j == 0) {
                        joint.mJointPos[0] = data.unk144[i];
                    } else {
                        MultiplyTranspose(
                            data.unk144[i], mPlayerXfms[j - 1], joint.mJointPos[j]
                        );
                    }
                }
                joint.mJointConf = (JointConfidence)data.unk284[i];
                joint.unk60 = data.unk144[i];
            }

            for (int i = 0; i < kNumJoints; i++) {
                mCamBoneLengths[i] =
                    BaseSkeleton::BoneLength((SkeletonBone)i, kCoordCamera);
            }

            mCamDisplacements.clear();

            Vector4 vec4 = skeletonFrame.unk18;
            unkac4 = (mTrackedJoints[kJointHipRight].mJointPos[kCoordCamera].y
                      + mTrackedJoints[kJointHipLeft].mJointPos[kCoordCamera].y)
                    * 0.5f
                + vec4.w;
        }

    } else {
        Init();
    }
}

bool Skeleton::Displacements(
    const SkeletonHistory *history, SkeletonCoordSys sys, int i1, Vector3 *vec, int &i2
) const {
    FOREACH (it, mCamDisplacements) {
        if (it->unk0 == i1) {
            memcpy(vec, it->unk8, 0x140);
            i2 = it->unk4;
            return it->unk4 + 1 != 0;
        }
    }

    CameraDisplacement displacement;
    displacement.unk0 = i1;
    ArchiveSkeleton skeleton;
    bool check = PrevTrackedSkeleton(history, i1, i2, skeleton);
    if (check) {
        for (int i = 0; i < kNumJoints; i++) {
            Vector3 v;
            skeleton.JointPos(sys, (SkeletonJoint)i, v);
            Subtract(mTrackedJoints[i].mJointPos[sys], v, vec[i]);
        }
    } else {
        memset(vec, 0, 0x140);
    }

    displacement.unk4 = i2;
    memcpy(displacement.unk8, vec, 0x140);
    mCamDisplacements.push_back(displacement);
    return check;
}
