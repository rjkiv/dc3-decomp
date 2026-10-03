#pragma once
#include "MoveMgr.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"
#include "utl/DebugGraph.h"
#include "gesture/GestureMgr.h"
#include <vector>

// size 0xc44
/** "An object that performs rhythmic analysis on Kinect input." */
class RhythmDetector : public RndPollable, public SkeletonCallback {
public:
    struct Frame {
        float unk0; // 0x0 time created?
        std::vector<Vector3> mJointVelocities; // 0x4 - could be vec<vec3> - ghidra
                                               // doesnt make it clear
    };

    struct RecordData {
        float unk0; // 0x0 - window start
        float unk4; // 0x4 - window end
        float unk8; // 0x8
        float unkc; // 0xc
        float unk10; // 0x10
        float unk14; // 0x14
        bool unk18; // 0x18
        std::vector<Frame> frames; // 0x1c
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

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(RhythmDetector)

    /** "Start looking at Kinect data" */
    void StartRecording();
    /** "Stop looking at Kinect data" */
    void StopRecording();
    bool IsRecording() const { return mRecording; }
    float Groove() const;
    float Freshness() const;
    Vector4 Data1(int) const;
    Vector4 Data2(int) const;
    void AddDebugGraph(float, float, float, float, Hmx::Color);
    void AddFullDebugGraphs();
    void RemoveDebugGraphs();
    void ClearData();
    const RecordData &GetRecord(float, float, bool, Symbol, TextStream *);
    void SetSkeletonID(int skelID) { mSkeletonID = skelID; }

protected:
    RhythmDetector();

    // SkeletonCallback
    virtual void Clear() {}
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &) {}

    bool mTracked; // 0xc
    char mRecording; // 0xd - used in recording funcs
    int mSkeletonID; // 0x10
    std::list<Frame> unk14; // 0x14
    Frame unk1c; // 0x1c
    std::vector<Frame> unk2c; // 0x2c
    std::vector<Frame> unk38; // 0x38
    float mBeats; // 0x44
    float mGroove; // 0x48
    float mRhythmDecay; // 0x4c
    int mFold; // 0x50
    float mToleranceFactor; // 0x54
    Vector3 mDirection; // 0x58
    float unk68; // 0x68 - used in cleardata
    DebugGraph *mDebugGraphA; // 0x6c
    DebugGraph *mDebugGraphB; // 0x70
    DebugGraph *mDebugGraphC; // 0x74
    DebugGraph *mDebugGraphD; // 0x78
    DebugGraph *mDebugGraphE; // 0x7c
    int unk80; // 0x80
    Vector3 unk84[8][kNumJoints]; // 0x84
    float unka84[8]; // 0xa84
    int unkaa4; // 0xaa4 - index into the above arrays
    float unkaa8; // 0xaa8 - mLastBeatTime?
    Vector3 unkaac[kNumJoints]; // 0xaac
    RecordData mRecordData; // 0xbec

private:
    void AddFrame(BaseSkeleton const &);
    void ProcessFrames();
};

void SetupFrame(
    RhythmDetector::Frame &, float, float, Vector3 const *, Vector3 const *, float
);
