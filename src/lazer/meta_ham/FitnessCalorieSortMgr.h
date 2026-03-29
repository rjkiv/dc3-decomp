#pragma once
#include "meta/SongPreview.h"
#include "meta_ham/NavListSortMgr.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class FitnessCalorieSortMgr : public NavListSortMgr {
private:
    FitnessCalorieSortMgr(SongPreview &);
    virtual ~FitnessCalorieSortMgr();

public:
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SelectionIs(Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();

    static void Terminate();
    static void Init(SongPreview &);

    std::vector<int> &GetUnk78() { return unk78; }
    int GetGroupSize() const { return mGroupSize; }

protected:
    std::vector<int> unk78;
    int mGroupSize; // 0x84
};

extern FitnessCalorieSortMgr *TheFitnessCalorieSortMgr;
