#pragma once
#include "IdentityInfo.h"
#include "SkeletonRecoverer.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/IdentityInfo.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "hamobj/Ham.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Tex.h"
#include "xdk/XAPILIB.h"

DECLARE_MESSAGE(KinectHardwareStatusMsg, "kinect_status_changed")
enum KinectHardwareStatus {
    kKinectConnected = 0x0000,
    kKinectReady = 0x0001,
    kKinectError = 0x0002,
};
KinectHardwareStatusMsg(int status) : Message(Type(), status) {}
KinectHardwareStatus HardwareStatus() const {
    return (KinectHardwareStatus)mData->Int(2);
}
END_MESSAGE

DECLARE_MESSAGE(KinectGuideGestureMsg, "kinect_guide_gesture")
KinectGuideGestureMsg(int tracking_id) : Message(Type(), tracking_id) {}
int TrackingID() const { return mData->Int(2); }
END_MESSAGE

DECLARE_MESSAGE(KinectUserBindingChangedMsg, "kinect_user_binding_changed")
KinectUserBindingChangedMsg(int padNum) : Message(Type(), padNum) {}
END_MESSAGE

DECLARE_MESSAGE(SkeletonEnrollmentChangedMsg, "skeleton_enrollment_changed")
SkeletonEnrollmentChangedMsg() : Message(Type()) {}
END_MESSAGE

// size 0x4294
class GestureMgr : public Hmx::Object, public SkeletonCallback {
public:
    enum PauseOnSkeletonLossType {
        kSkelLoss_NoPauseFlashcardsDropOut = 0x0000,
        kSkelLoss_NoPauseFlashcardsDontDropOut = 0x0001,
        kSkelLoss_Pause = 0x0002,
        kSkelLoss_COUNT = 0x0003,
    };
    // Hmx::Object
    virtual ~GestureMgr();
    virtual DataNode Handle(DataArray *, bool);
    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    bool IsSkeletonValid(int idx) const;
    bool IsSkeletonSitting(int idx) const;
    bool IsSkeletonSideways(int idx) const;
    Skeleton *GetActiveSkeleton();
    IdentityInfo *GetIdentityInfo(int skeletonIndex);
    void SetIdentificationEnabled(bool enabled);
    int GetSkeletonIndexByTrackingID(int tracking_id) const;
    Skeleton *GetSkeletonByTrackingID(int tracking_id);
    Skeleton *GetSkeletonByEnrollmentIndex(int enrollmentIndex);
    Skeleton &GetSkeleton(int idx);
    const Skeleton &GetSkeleton(int idx) const;
    SkeletonQualityFilter &GetSkeletonQualityFilter(int idx);
    LiveCameraInput *GetLiveCameraInput() const;
    void SetTrackedSkeletons(int player1, int player2);
    void UpdateTrackedSkeletons();
    int GetActiveSkeletonIndex() const;
    void SetInControllerMode(bool on);
    void SetInVoiceMode(bool on);
    void SetGesturingWithVoice(bool on);
    void SetInDoubleUserMode(bool on);
    void StartTrackAllSkeletons();
    void CancelTrackAllSkeletons();
    void Poll();
    bool IsTrackingAllSkeletons() const;
    int GetPlayerSkeletonID(int playerIndex);
    void SetPlayerSkeletonID(int playerIndex, int trackingID);
    int GetPlayerFilteredSkeletonID(int playerIndex, bool filterSitting);
    int GetSecondarySkeletonIndex(bool needValid) const;
    void DrawSkeletonKinectData();
    bool IDEnabled() { return mIdentificationEnabled; }
    bool UIMode() { return mUIMode; }
    void SetUIMode(bool b) { mUIMode = b; }

    void ShowGestureGuide() {
        int id = 0;
        if (mActiveTrackingID > 0) {
            id = mActiveTrackingID;
        }
        XShowNuiGuideUI(id);
    }

    PauseOnSkeletonLossType PauseOnSkeletonLoss() const { return mPauseOnSkeletonLoss; }
    void SetPauseOnSkeletonLoss(PauseOnSkeletonLossType t) { mPauseOnSkeletonLoss = t; }

    int TogglePauseOnSkeletonLoss() {
        int newloss = ((int)mPauseOnSkeletonLoss + 1) % 3;
        mPauseOnSkeletonLoss = (PauseOnSkeletonLossType)newloss;
        return mPauseOnSkeletonLoss;
    }
    void AutoTilt() {
        if (mTiltOverlapped.InternalLow != ERROR_IO_PENDING) {
            memset(&mTiltOverlapped, 0, sizeof(XOVERLAPPED));
        }
    }
    RndTex *SnapshotTex(int idx) const {
        RndMat *mat = mLiveCamInput->GetSnapshot(idx);
        return mat ? mat->DiffuseTex() : nullptr;
    }
    bool InControllerMode() const { return mInControllerMode; }
    bool InDoubleUserMode() const { return mInDoubleUserMode; }
    bool InVoiceMode() const { return mInVoiceMode; }
    bool GesturingWithVoice() const { return mGesturingWithVoice; }
    SkeletonRecoverer &GetSkeletonRecoverer() { return mSkeletonRecoverer; }
    void SetActiveSkeletonTrackingID(int id) { mActiveTrackingID = id; }
    void SetSkeletonStatus(int idx, int val) { mSkeletonTrackingStatus[idx] = val; }
    int ActiveSkeletonTrackingId() const { return mActiveTrackingID; }
    static float GetMaxRecoveryDistance() { return sMaxRecoveryDistance; }
    static float GetMinRecoveryTime() { return sMinRecoveryTime; }
    static float GetMaxRecoveryTime() { return sMaxRecoveryTime; }

    static bool sIdentityOpInProgress;
    static void Init();
    static void DebugInit();
    static void Terminate();

private:
    GestureMgr();

    DataNode OnMsg(const KinectHardwareStatusMsg &);
    DataNode OnMsg(const KinectUserBindingChangedMsg &);

    static float sMaxRecoveryDistance;
    static float sMinRecoveryTime;
    static float sMaxRecoveryTime;
    static float sConfidenceLossThreshold;
    static float sConfidenceRegainThreshold;

    int mSkeletonTrackingStatus[NUM_SKELETONS]; // 0x30
    LiveCameraInput *mLiveCamInput; // 0x48
    Skeleton mSkeletons[NUM_SKELETONS]; // 0x4c
    IdentityInfo mIdentityInfo[NUM_SKELETONS]; // 0x4144
    SkeletonQualityFilter mSkeletonQuality[NUM_SKELETONS]; // 0x41a4
    bool mTrackAllSkeletons; // 0x424c
    SkeletonRecoverer mSkeletonRecoverer; // 0x4250
    PauseOnSkeletonLossType mPauseOnSkeletonLoss; // 0x425c
    int mActiveTrackingID; // 0x4260
    int mPlayerSkeletonID[MAX_NUM_PLAYERS]; // 0x4264
    bool mIdentificationEnabled; // 0x426c
    bool mInControllerMode; // 0x426d
    bool mInVoiceMode; // 0x426e
    bool mGesturingWithVoice; // 0x426f
    bool mInDoubleUserMode; // 0x4270
    bool mUIMode; // 0x4271
    RndDir *mSkeletonDebugDisplay; // 0x4274
    XOVERLAPPED mTiltOverlapped; // 0x4278
};

extern GestureMgr *TheGestureMgr;
