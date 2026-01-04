#pragma once
#include "MoveMgr.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"
#include "utl/DebugGraph.h"
#include "gesture/GestureMgr.h"

class RhythmDetector : public RndPollable, public SkeletonCallback {
public:
    struct Frame {
        float unk0; // 0x0 time created?
        std::vector<MoveChoiceSet> mJointVelocities; // 0x4
    };

    struct RecordData {
    public:
        ~RecordData();
        float unkbec; // 0xbec
        float unkbf0; // 0xbf0
        float unkbf4; // 0xbf4
        float unkbf8; // 0xbf8
        float unkbfc; // 0xbfc
        float unkc00; // 0xc00
        bool unkc04; // 0xc04
        std::vector<Frame> frames; // 0xc08
    };

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
    virtual void Enter();
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
    const RecordData &GetRecord(float, float, bool, Symbol, TextStream *);

protected:
    RhythmDetector();

    bool unkc; // 0xc
    char mRecording; // 0xd
    int unk10; // 0x10 skeletonID?
    std::list<Frame> unk14; // 0x14
    int unk18; // 0x18
    std::vector<Vector3> unk20; // 0x20
    std::vector<Frame> unk2c; // 0x2C
    std::vector<Frame> unk38; // 0x38
    float unk44; // 0x44 - mBeats?
    float unk48; // 0x48 mGroove?
    float unk4c; // 0x4c mFreshness?
    int unk50; // 0x50 - mFold?
    float unk54; // 0x54
    //float unk58; // 0x58 - mDirX?
    //float unk5c; // 0x5c - mDirY?
    //float unk60; // 0x60 - mDirZ?
    Vector3 unk58;
    float unk64; // 0x64
    //float unk68; // 0x68
    //float unksomething;
    DebugGraph *mDebugGraphA; // 0x6c
    DebugGraph *mDebugGraphB; // 0x70
    DebugGraph *mDebugGraphC; // 0x74
    DebugGraph *mDebugGraphD; // 0x78
    DebugGraph *mDebugGraphE; // 0x7c
    float unk80[721]; // 0x80 some big ass buffer maybe who knows
    //float unk80[256];
    std::vector<float> unka80; // 0xa80
    //double *unka80; // 0xa80
    //float unkaa4; // 0xaa4
    float unkaa8; // 0xaa8
    //std::vector<Vector3> unkaac; // 0xaac - im unsure of these 3 vectors
    //std::vector<Vector3*> unkab0; // 0xab0
    //std::vector<Vector3*> unkab4; // 0xab4
    float unkaac; // 0xaac
    float unkab0; // 0xab0
    //float unkab4; // 0xab4
    RecordData mRecordData; // 0xbec
    //float unkbec; // 0xbec
    //float unkbf0; // 0xbf0
    //float unkbf4; // 0xbf4
    //float unkbf8; // 0xbf8
    //float unkbfc; // 0xbfc
    //float unkc00; // 0xc00
    bool unkc04; // 0xc04
    std::vector<Frame> unkc08; // 0xc08

private:
    void AddFrame(BaseSkeleton const &);
    void ProcessFrames();
};

void SetupFrame(
    RhythmDetector::Frame &, float, float, Vector3 const *, Vector3 const *, float
);
