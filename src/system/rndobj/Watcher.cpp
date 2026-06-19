#include "rndobj/Watcher.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"

static bool sLoadingWatches = false;

void Watcher::SaveWatches() {
    if (!sLoadingWatches) {
        DataArrayPtr ptr;
        ptr->Resize(mWatches.size());
        int i = 0;
        FOREACH (it, mWatches) {
            ptr->Node(i) = it->first;
            i++;
        }
        DataWriteFile("watches.dta", ptr, 0);
    }
}

void Watcher::Update() {
    if (mWatches.size()) {
        mOverlay->SetLines(mWatches.size() * 2);
        int idx = 0;
        for (auto it = mWatches.begin(); it != mWatches.end(); ++it, ++idx) {
            DataArray *arr = it->first;
            *mOverlay << idx;
            *mOverlay << ": ";
            arr->Print(*mOverlay, kDataArray, true, 0);
            *mOverlay << "\n";
            MILO_TRY { it->second = arr->Execute(false); }
            MILO_CATCH(msg) { it->second = msg; }
            it->second.Print(*mOverlay, false, 0);
            *mOverlay << "\n";
        }
    } else
        mOverlay->Clear();
}

void Watcher::Remove(int i) {
    if (i < mWatches.size()) {
        mWatches.erase(mWatches.begin() + i);
        SaveWatches();
    }
}

DataNode Watcher::OnRemove(const DataArray *a) {
    Remove(a->Int(2));
    return 0;
}

void Watcher::Add(DataArray *a) {
    mWatches.push_back(std::pair<DataArray *, DataNode>(a, 0));
    SaveWatches();
}

DataNode Watcher::OnAdd(const DataArray *a) {
    Add(a->Array(2));
    return 0;
}

void Watcher::LoadWatches() {
    sLoadingWatches = true;
    mWatches.clear();
    DataArray *watchArr = DataReadFile("watches.dta", false);
    if (watchArr) {
        for (int i = 0; i < watchArr->Size(); i++) {
            Add(watchArr->Array(i));
        }
    }
    sLoadingWatches = false;
}

void Watcher::Init() {
    SetName("watcher", ObjectDir::Main());
    LoadWatches();
}

BEGIN_HANDLERS(Watcher)
    HANDLE(add, OnAdd)
    HANDLE(remove, OnRemove)
    HANDLE_ACTION(reload, LoadWatches())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
