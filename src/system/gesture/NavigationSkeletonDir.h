#pragma once
#include "gesture/DirectionGestureFilter.h"
#include "gesture/SkeletonDir.h"

/** "Dir to view shell navigation gestures in milo" */
class NavigationSkeletonDir : public SkeletonDir {
public:
    // Hmx::Object
    virtual ~NavigationSkeletonDir();
    OBJ_CLASSNAME(NavigationSkeletonDir)
    OBJ_SET_TYPE(NavigationSkeletonDir)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // SkeletonCallback
    virtual void Clear();
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &);

    OBJ_MEM_OVERLOAD(0x12)
    NEW_OBJ(NavigationSkeletonDir)
    REGISTER_OBJ(NavigationSkeletonDir)

protected:
    NavigationSkeletonDir();

    virtual void MiloUpdate();

    DirectionGestureFilterSingleUser *mDirectionGestureFilter; // 0x284
};
