#pragma once
#include "meta/SongPreview.h"
#include "meta_ham/NavListSortMgr.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class FitnessCalorieSortMgr : public NavListSortMgr {
private:
    virtual ~FitnessCalorieSortMgr();
    FitnessCalorieSortMgr(SongPreview &);

public:
    virtual DataNode Handle(DataArray *, bool) { return 0; }
    virtual bool SelectionIs(Symbol);
    virtual Symbol MoveOn();
    virtual void OnEnter();

    static void Terminate();
    static void Init(SongPreview &);

protected:
    std::vector<int> unk78;
    int mGroupSize; // 0x84
};

extern FitnessCalorieSortMgr *TheFitnessCalorieSortMgr;
