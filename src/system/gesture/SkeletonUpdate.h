#pragma once
#include "gesture/CameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonHistory.h"
#include "obj/Object.h"
#include "os/CritSec.h"
#include "xdk/nui/nuiskeleton.h"

class SkeletonUpdate;

class SkeletonUpdateHandle {
public:
    SkeletonUpdateHandle(SkeletonUpdate *);
    ~SkeletonUpdateHandle();

    std::vector<SkeletonCallback *> &Callbacks();
    CameraInput *GetCameraInput() const;
    void SetCameraInput(CameraInput *);
    void RemoveCallback(SkeletonCallback *);
    void AddCallback(SkeletonCallback *);
    bool HasCallback(SkeletonCallback *);
    void PostUpdate();
    const SkeletonHistory *History() const;

    SkeletonUpdate *Inst() const { return mInst; }

private:
    SkeletonUpdate *mInst; // 0x0

    static CriticalSection sCritSec;
};

class SkeletonUpdate : public SkeletonHistoryArchive,
                       public SkeletonHistory,
                       public Hmx::Object {
    friend class SkeletonUpdateHandle;

public:
    // SkeletonHistory
    virtual bool PrevSkeleton(const Skeleton &, int, ArchiveSkeleton &, int &) const;
    // Hmx::Object
    virtual ~SkeletonUpdate();

    static void Init();
    static void CreateInstance();
    static void Terminate();
    static bool HasInstance();
    static HANDLE NewSkeletonEvent();
    static HANDLE SkeletonUpdatedEvent() { return sSkeletonUpdatedEvent; }
    static SkeletonUpdateHandle InstanceHandle();

    void PublicUpdate() { Update(); }

    int GetUnk5388() const { return mNumStubSkeletons; }
    void SetUnk5388(int i) { mNumStubSkeletons = i; }
    int GetUnk538C() const { return mFakeShellSkeletonMask; }
    void SetUnk538C(int i) { mFakeShellSkeletonMask = i; }
    bool GetUnk5390() const { return mSwapFakeSkeletonSides; }
    void SetUnk5390(bool b) { mSwapFakeSkeletonSides = b; }
    int GetUnk5394() const { return mActiveFakeShellSkeleton; }
    void SetUnk5394(int i) { mActiveFakeShellSkeleton = i; }
    bool GetUnk539C() const { return mThreadedUpdate; }
    void SetUnk539C(bool b) { mThreadedUpdate = b; }

private:
    SkeletonUpdate();

    virtual bool Replace(ObjRef *, Hmx::Object *);

    void SetCameraInput(CameraInput *);
    void PostUpdate();
    void Update();
    void UpdateFakeArmPos();
    void UpdateCallbacks();

    static SkeletonUpdate *sInstance;
    static HANDLE sNewSkeletonEvent;
    static HANDLE sSkeletonUpdatedEvent;

    bool mNewFrame; // 0x78
    ObjOwnerPtr<CameraInput> mCameraInput; // 0x7c
    bool mCameraConnected; // 0x90
    bool mCameraOverride; // 0x91
    std::vector<SkeletonCallback *> mCallbacks; // 0x94
    SkeletonFrame mSkeletonFrame; // 0xa0
    Skeleton mSkeletons[6]; // 0x1268
    const Skeleton *mPlayerSkeletons[2]; // 0x5360
    const Skeleton *mAllSkeletons[6]; // 0x5368
    int mPlayerSkeletonTrackingIDs[2]; // 0x5380
    int mNumStubSkeletons; // 0x5388
    int mFakeShellSkeletonMask; // 0x538c
    bool mSwapFakeSkeletonSides; // 0x5390
    int mActiveFakeShellSkeleton; // 0x5394
    float mFakeArmHeight; // 0x5398
    bool mThreadedUpdate; // 0x539c
    HANDLE mSkeletonUpdateThread; // 0x53a0
    NUI_SKELETON_FRAME *mNuiSkeletonFrame; // 0x53a4
};
