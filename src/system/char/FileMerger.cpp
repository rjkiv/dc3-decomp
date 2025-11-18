#include "char/FileMerger.h"

FileMerger *FileMerger::sFmDeleting;

FileMerger::FileMerger()
    : mMergers(this), mAsyncLoad(0), mLoadingLoad(0), mCurLoader(0), mFilter(0),
      mHeap(GetCurrentHeapNum()), mOrganizer(this) {
    MILO_ASSERT(MemNumHeaps() == 0 || (mHeap != kNoHeap && mHeap != kSystemHeap), 0x86);
}

FileMerger::~FileMerger() {
    FileMerger *old = sFmDeleting;
    sFmDeleting = this;
    Clear();
    sFmDeleting = old;
}

void FileMerger::Clear() {
    for (int i = 0; i < mMergers.size(); i++) {
        mMergers[i].Clear(false);
    }
    if (mCurLoader) {
        Merger *merger = mFilesPending.front();
        mFilesPending.clear();
        mFilesPending.push_front(merger);
        DeleteCurLoader();
    }
}

bool FileMerger::StartLoad(bool b) { return StartLoadInternal(b, false); }
