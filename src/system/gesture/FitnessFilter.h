#pragma once

#include "gesture/SkeletonViz.h"
#include "obj/Data.h"
#include "rndobj/Overlay.h"
#include "utl/BinStream.h"
class FitnessFilter {
public:
    virtual ~FitnessFilter();
    virtual void Clear();
    virtual void Poll();
    virtual void Draw(BaseSkeleton const &, SkeletonViz &);

    FitnessFilter();
    void SetPlayerIndex(int);
    bool GetFitnessData(float &, float &) const;
    bool GetFitnessDataAndReset(float &, float &);
    void SetPaused(bool);
    void StopTracking();

    bool unk4;
    bool unk5;
    bool unk6;
    int unk8;
    bool unkc;
    RndOverlay *unk10;
    int unk14;
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

    FitnessFilter *unk2c;

protected:
    FitnessFilterObj();
};
