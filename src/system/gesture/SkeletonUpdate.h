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

private:
    SkeletonUpdate();

    virtual bool Replace(ObjRef *, Hmx::Object *);

    void SetCameraInput(CameraInput *);
    void PostUpdate();
    void Update();

    static SkeletonUpdate *sInstance;
    static HANDLE sNewSkeletonEvent;
    static HANDLE sSkeletonUpdatedEvent;

    bool unk78; // 0x78
    ObjOwnerPtr<CameraInput> mCameraInput; // 0x7c
    bool unk90;
    bool unk91;
    std::vector<SkeletonCallback *> mCallbacks; // 0x94
    SkeletonFrame mSkeletonFrame; // 0xa0
    Skeleton mSkeletons[6]; // 0x1268
    const Skeleton *(*unk5360[2])[6]; // 0x5360
    const Skeleton *(*unk5368[2])[6]; // 0x5368
    // Skeleton *unk5360[2]; // 0x5360
    // Skeleton *unk5368[2]; // 0x5368
    int unk5370;
    int unk5374;
    int unk5378;
    int unk537c;
    int unk5380[2]; // 0x5380
    int unk5388; // 0x5388
    int unk538c; // 0x538c
    bool unk5390; // 0x5390 - sides swapped?
    int unk5394;
    float unk5398;
    bool unk539c; // 0x539c - update thread?
    HANDLE unk53a0;
    NUI_SKELETON_FRAME *mNUISkeletonFrame; // 0x53a4
};
