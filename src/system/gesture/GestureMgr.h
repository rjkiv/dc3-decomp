#pragma once
#include "IdentityInfo.h"
#include "SkeletonRecoverer.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/IdentityInfo.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonQualityFilter.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Tex.h"
#include "xdk/XAPILIB.h"

DECLARE_MESSAGE(KinectHardwareStatusMsg, "kinect_status_changed")
KinectHardwareStatusMsg(int i) : Message(Type(), i) {}
END_MESSAGE

DECLARE_MESSAGE(KinectUserBindingChangedMsg, "kinect_user_binding_changed")
KinectUserBindingChangedMsg(int i) : Message(Type(), i) {}
END_MESSAGE

DECLARE_MESSAGE(SkeletonEnrollmentChangedMsg, "skeleton_enrollment_changed")
SkeletonEnrollmentChangedMsg() : Message(Type()) {}
END_MESSAGE

// size 0x4294
class GestureMgr : public Hmx::Object, public SkeletonCallback {
public:
    // Hmx::Object
    virtual ~GestureMgr();
    virtual DataNode Handle(DataArray *, bool);
    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    bool IsSkeletonValid(int) const;
    bool IsSkeletonSitting(int) const;
    bool IsSkeletonSideways(int) const;
    Skeleton *GetActiveSkeleton();
    IdentityInfo *GetIdentityInfo(int);
    void SetIdentificationEnabled(bool);
    int GetSkeletonIndexByTrackingID(int) const;
    Skeleton *GetSkeletonByTrackingID(int);
    Skeleton *GetSkeletonByEnrollmentIndex(int);
    Skeleton &GetSkeleton(int);
    const Skeleton &GetSkeleton(int) const;
    SkeletonQualityFilter &GetSkeletonQualityFilter(int);
    LiveCameraInput *GetLiveCameraInput() const;
    void SetTrackedSkeletons(int, int);
    void UpdateTrackedSkeletons();
    int GetActiveSkeletonIndex() const;
    void SetInControllerMode(bool);
    void SetInVoiceMode(bool);
    void SetGesturingWithVoice(bool);
    void SetInDoubleUserMode(bool);
    void StartTrackAllSkeletons();
    void CancelTrackAllSkeletons();
    void Poll();
    bool IsTrackingAllSkeletons() const;
    int GetPlayerSkeletonID(int);
    void SetPlayerSkeletonID(int, int);
    int GetPlayerFilteredSkeletonID(int, bool);
    bool IDEnabled() { return mIDEnabled; }
    bool GetBool4271() { return unk4271; } // change once context found
    int GetVal425C() { return unk425c; } // change once context found

    void ShowGestureGuide() {
        int id = 0;
        if (unk4260 > 0) {
            id = unk4260;
        }
        XShowNuiGuideUI(id);
    }

    int Unk425C() const { return unk425c; }

    int TogglePauseOnSkeletonLoss() {
        unk425c = (unk425c + 1) % 3;
        return unk425c;
    }
    void AutoTilt() {
        if (mOverlapped.InternalLow != 0x3E5) {
            memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
        }
    }
    RndTex *GetSnapshotTex(int idx) const {
        RndMat *mat = mLiveCamInput->GetSnapshot(idx);
        return mat ? mat->GetDiffuseTex() : nullptr;
    }
    bool InControllerMode() const { return mInControllerMode; }

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

    SkeletonCallback *mCallbacks[6]; // 0x30
    LiveCameraInput *mLiveCamInput; // 0x48
    Skeleton mSkeletons[6]; // 0x4c
    IdentityInfo mIdentityInfos[6]; // 0x4144
    SkeletonQualityFilter mFilters[6]; // 0x41a4
    bool mTrackingAllSkeletons; // 0x424c
    SkeletonRecoverer mRecoverer; // 0x4250
    int unk425c;
    int unk4260; // 0x4260 - active skeleton tracking ID
    int mPlayerSkeletonIDs[2]; // 0x4264
    bool mIDEnabled; // 0x426c
    bool mInControllerMode; // 0x426d
    bool mInVoiceMode; // 0x426e
    bool mGesturingWithVoice; // 0x426f
    bool mInDoubleUserMode; // 0x4270
    bool unk4271;
    RndDir *unk4274;
    XOVERLAPPED mOverlapped; // 0x4278
};

extern GestureMgr *TheGestureMgr;
