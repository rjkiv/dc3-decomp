#pragma once
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"

class RhythmDetector : public RndPollable, public SkeletonCallback {
public:
    struct Frame {};

    struct RecordData {};

    // Hmx::Object
    virtual ~RhythmDetector();
    OBJ_CLASSNAME(RhythmDetector);
    OBJ_SET_TYPE(RhythmDetector);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter() { RndPollable::Enter(); }
    virtual void Exit() { RndPollable::Exit(); }
    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(RhythmDetector)

    void StopRecording();
    void StartRecording();
    float Groove() const;
    float Freshness() const;
    Vector4 Data1(int) const;
    Vector4 Data2(int) const;
    void AddDebugGraph(float, float, float, float, Hmx::Color);
    void AddFullDebugGraphs();
    void RemoveDebugGraphs();
    void ClearData();

protected:
    RhythmDetector();

private:
    void AddFrame(BaseSkeleton const &);
    void ProcessFrames();
};

void SetupFrame(
    RhythmDetector::Frame &, float, float, Vector3 const *, Vector3 const *, float
);
