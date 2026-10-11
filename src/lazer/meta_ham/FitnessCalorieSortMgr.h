#pragma once
#include "meta/SongPreview.h"
#include "meta_ham/NavListSortMgr.h"
#include "obj/Data.h"
#include "utl/Symbol.h"

// class not in pdb :(

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

    std::vector<int> &GetCalories() { return mCalories; }
    int GetGroupSize() const { return mGroupSize; }

private:
    std::vector<int> mCalories; // 0x78
    int mGroupSize; // 0x84
};

extern FitnessCalorieSortMgr *TheFitnessCalorieSortMgr;
