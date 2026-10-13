#pragma once
#include "gesture/Skeleton.h"
#include "gesture/SkeletonClip.h"
#include "obj/Object.h"
#include "ui/PanelDir.h"
#include "utl/MemMgr.h"

/** "Dir with skeleton tracking/debugging functionality" */
class SkeletonDir : public PanelDir, public SkeletonCallback {
public:
    // Hmx::Object
    virtual ~SkeletonDir();
    OBJ_CLASSNAME(SkeletonDir)
    OBJ_SET_TYPE(SkeletonDir)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void RemovingObject(Hmx::Object *);
    // RndDrawable
    virtual void DrawShowing();
    // RndAnimatable
    virtual void StartAnim();
    // RndPollable
    virtual void Poll();
    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *) {}
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(SkeletonDir)

    void SetSkeletonClip(SkeletonClip *);
    SkeletonClip *TestClip() const;
    /** "Output skeleton/joint information" */
    void PrintSkeleton() const;

    // live_camera: "Return to using live camera input"

protected:
    SkeletonDir();

    virtual void MiloUpdate();

    void MiloInit();

    bool mNeedsUpdate; // 0x26c
    /** "Test clip for in-milo visualization" */
    ObjPtr<SkeletonClip> mTestClip; // 0x270
};
