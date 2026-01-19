#pragma once
#include "gesture/SkeletonViz.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class FitnessFilter : public RndOverlay::Callback {
public:
    FitnessFilter();
    virtual ~FitnessFilter() {}
    virtual float UpdateOverlay(RndOverlay *, float f1) { return f1; }
    virtual void Clear();
    virtual void Poll();
    virtual void Draw(const BaseSkeleton &, SkeletonViz &);

    void SetPlayerIndex(int);
    bool GetFitnessData(float &, float &) const;
    bool GetFitnessDataAndReset(float &, float &);
    void SetPaused(bool);
    void StopTracking();
    void StartTracking() { unk6 = true; }

private:
    bool unk4; // 0x4 - paused?
    bool unk5;
    bool unk6;
    int mTrackingID; // 0x8
    bool unkc; // 0xc - actively tracking?
    RndOverlay *mFitnessMeterOverlay; // 0x10
    int mPlayerIndex; // 0x14
};

class FitnessFilterObj : public Hmx::Object {
public:
    OBJ_CLASSNAME(FitnessFilterObj)
    OBJ_SET_TYPE(FitnessFilterObj)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Save(BinStream &);
    virtual void Copy(Hmx::Object const *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    bool OnGetFitnessData(DataArray *) const;
    bool OnGetFitnessDataAndReset(DataArray *);

    NEW_OBJ(FitnessFilterObj);
    OBJ_MEM_OVERLOAD(0x40)

protected:
    FitnessFilterObj();

    FitnessFilter mFilter; // 0x2c
};
