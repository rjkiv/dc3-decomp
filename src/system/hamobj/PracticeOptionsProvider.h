#pragma once
#include "hamobj/HamNavProvider.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

enum PracticeOptions {
    kPracticeSlowmo = 0,
    kPracticeSkip = 1,
    kPracticePrevious = 2,
    kPracticeVideo = 3,
    kNumPracticeOptions = 4
};

class PracticeOptionsProvider : public HamNavProvider {
public:
    // Hmx::Object
    virtual ~PracticeOptionsProvider() {}
    OBJ_CLASSNAME(PracticeOptionsProvider);
    OBJ_SET_TYPE(PracticeOptionsProvider);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return kNumPracticeOptions; }
    virtual bool IsActive(int) const;
    virtual void InitData(RndDir *);

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(PracticeOptionsProvider)
    static void Init();

    void SetVideoReplay(bool);
    void SetSlow(bool);

protected:
    PracticeOptionsProvider();

    RndMat *mPracticeMats[kNumPracticeOptions]; // 0x40
    Symbol mPracticeOptions[kNumPracticeOptions]; // 0x50
    bool unk60;
    bool mDisablePrevious; // 0x61
};
