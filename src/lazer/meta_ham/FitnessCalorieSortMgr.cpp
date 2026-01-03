#include "meta_ham/FitnessCalorieSortMgr.h"
#include "FitnessCalorieSortMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/NavListSortMgr.h"
#include "obj/Dir.h"

FitnessCalorieSortMgr *TheFitnessCalorieSortMgr;

FitnessCalorieSortMgr::FitnessCalorieSortMgr(SongPreview &sp) : NavListSortMgr(sp) {
    SetName("calorie_provider", ObjectDir::Main());
    // mSorts.push_back(new FitnessSortByCalorie());
    mGroupSize = 1;
}
