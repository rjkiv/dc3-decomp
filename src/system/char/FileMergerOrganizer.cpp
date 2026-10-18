#include "char/FileMergerOrganizer.h"
#include "FileMerger.h"
#include "FileMergerOrganizer.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"

struct CatData {
    int priority; // 0x0
    bool unk4; // 0x4 - true if gender_order array contains category_order key
};

FileMergerOrganizer *TheFileMergerOrganizer;
std::map<Symbol, CatData> gCatPriority;
int gNextCatPriority = 1;
bool gOrganizing;
bool gGenderChirality;

void FileMergerOrganizerLoader::PollLoading() { TheFileMergerOrganizer->StartLoad(); }

FileMergerOrganizer::FileMergerOrganizer() : mActiveOrg(nullptr), mStartOrg(nullptr) {
    MILO_ASSERT(TheFileMergerOrganizer == NULL, 0x1BE);
    TheFileMergerOrganizer = this;
}

BEGIN_HANDLERS(FileMergerOrganizer)
    HANDLE_ACTION(add_file_merger, AddFileMerger(_msg->Obj<FileMerger>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(FileMergerOrganizer)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(FileMergerOrganizer)
    MILO_FAIL("Can't save TheFileMergerOrganizer");
END_SAVES

BEGIN_COPYS(FileMergerOrganizer)
    MILO_FAIL("Can't copy TheFileMergerOrganizer");
END_COPYS

BEGIN_LOADS(FileMergerOrganizer)
    MILO_FAIL("Can't load TheFileMergerOrganizer");
END_LOADS

FileMerger::Merger *FileMergerOrganizer::FrontInactiveMerger(OrganizedFileMerger *ofm) {
    std::list<FileMerger::Merger *>::iterator it = ofm->merger->mFilesPending.begin();
    if (ofm->merger->mCurLoader)
        it++;
    if (it == ofm->merger->mFilesPending.end())
        return nullptr;
    else
        return *it;
}

void FileMergerOrganizer::FinishLoading(Loader *l) {
    OrganizedFileMerger *org = mActiveOrg;
    MILO_ASSERT(org->merger->mCurLoader == l, 0x144);
    MILO_ASSERT(org->merger->mFilesPending.front()->loading == l->LoaderFile(), 0x145);
    MILO_ASSERT(org->state == kPendingLoad, 0x146);
    org->state = kFinishLoad;
    mActiveOrg = nullptr;
    if (l->LoaderFile().empty()) {
        if (FrontInactiveMerger(org)) {
            Dispatch(org);
        } else
            RemoveFileMerger(org);
    } else
        CheckDone();
}

void FileMergerOrganizer::FailedLoading(Loader *l) {
    OrganizedFileMerger *org = nullptr;
    FOREACH (it, mFileMergers) {
        org = &*it;
        if (org->merger->mCurLoader && org->merger->mCurLoader == l)
            break;
    }
    MILO_ASSERT(org, 0x173);
    MILO_ASSERT(org->merger->mFilesPending.front()->loading == l->LoaderFile(), 0x176);
    MILO_ASSERT(org->merger->mCurLoader == l, 0x177);
    MILO_ASSERT(org->state == kPendingLoad || (org->state == kFinishLoad && mActiveOrg != org), 0x179);
    org->state = kFailedLoad;
    OrganizedFileMerger *oldOrg = mActiveOrg;
    mActiveOrg = nullptr;
    Dispatch(org);
    MILO_ASSERT(org->merger->mCurLoader == NULL, 0x184);
    org->state = (OrganizedState)0;
    if (oldOrg == org)
        oldOrg = nullptr;
    mActiveOrg = oldOrg;
    if (!mActiveOrg && !mStartOrg) {
        FOREACH (it, mFileMergers) {
            OrganizedFileMerger *curr = &*it;
            MILO_ASSERT(curr->state != kPendingLoad, 0x193);
        }
        mStartOrg = new FileMergerOrganizerLoader();
    }
}

void FileMergerOrganizer::Init() {
    MILO_ASSERT(gNextCatPriority == 1, 0x19E);
    REGISTER_OBJ_FACTORY(FileMergerOrganizer)
    DataArray *cfg = SystemConfig("file_merger_organizer");
    DataArray *catArr = cfg->FindArray("category_order", false);
    DataArray *genderArr = cfg->FindArray("gender_order", false);
    if (catArr) {
        for (; gNextCatPriority < catArr->Size(); gNextCatPriority++) {
            DataArray *curCatArr = catArr->Array(gNextCatPriority);
            for (int i = 0; i < curCatArr->Size(); i++) {
                Symbol key = curCatArr->Sym(i);
                CatData &val = gCatPriority[key];
                val.priority = gNextCatPriority;
                val.unk4 = genderArr && genderArr->Contains(key);
            }
            gOrganizing = true;
        }
    }
    FileMergerOrganizer::NewObject(); // ???
}

void FileMergerOrganizer::StartLoad() {
    gGenderChirality = RandomInt() & 1;
    RELEASE(mStartOrg);
    CheckDone();
}

void FileMergerOrganizer::Dispatch(FileMergerOrganizer::OrganizedFileMerger *ofm) {
    MILO_ASSERT(mActiveOrg == NULL, 0xC4);
    mActiveOrg = ofm;
    if (mActiveOrg->state != kFailedLoad) {
        FOREACH (it, mFileMergers) {
            OrganizedFileMerger *curr = &*it;
            MILO_ASSERT(curr->state != kPendingLoad, 0xCD);
        }
    }
    if (ofm->state == kFinishLoad) {
        ofm->merger->FinishLoading(ofm->merger->mCurLoader);
    } else if (ofm->state == kFailedLoad) {
        ofm->merger->FailedLoading(ofm->merger->mCurLoader);
    } else if (ofm->state == 0) {
        ofm->merger->LaunchNextLoader();
    } else {
        MILO_FAIL("Unknown dispatch state %d\n", ofm->state);
    }
    ofm->state = kPendingLoad;
}

void FileMergerOrganizer::RemoveFileMerger(FileMergerOrganizer::OrganizedFileMerger *ofm) {
    MILO_ASSERT(!mActiveOrg, 0x116);
    if (ofm->merger->mCurLoader) {
        Dispatch(ofm);
    }
    FileMerger *merger = ofm->merger;
    if (!merger->mCurLoader) {
        merger->mOrganizer = merger;
        FOREACH (it, mFileMergers) {
            if (&*it == ofm) {
                mFileMergers.erase(it);
                break;
            }
        }
        mActiveOrg = nullptr;
        CheckDone();
    }
}

void FileMergerOrganizer::CheckDone() {
    MILO_ASSERT(mActiveOrg == NULL, 0x97);
    MILO_ASSERT(mStartOrg == NULL, 0x98);
    OrganizedFileMerger *o5 = nullptr;
    FileMerger::Merger *m4 = nullptr;
    FOREACH (it, mFileMergers) {
        OrganizedFileMerger *curr = &*it;
        MILO_ASSERT(curr->state != kPendingLoad, 0xA3);
        FileMerger::Merger *front = FrontInactiveMerger(curr);
        if (!front) {
            RemoveFileMerger(curr);
            return;
        }
        FileMergerSort sort;
        if (!o5 || !sort(m4, front)) {
            o5 = curr;
            m4 = front;
        }
    }
    if (o5)
        Dispatch(o5);
}

void FileMergerOrganizer::AddFileMerger(FileMerger *fm) {
    if (!gOrganizing)
        fm->LaunchNextLoader();
    else {
        MILO_ASSERT(fm->mOrganizer == fm, 0xFD);
        MILO_ASSERT(fm->mCurLoader == NULL, 0xFE);
        FOREACH (it, mFileMergers) {
            OrganizedFileMerger *curr = &*it;
            MILO_ASSERT(curr->merger != fm, 0x102);
        }
        fm->mOrganizer = this;
        OrganizedFileMerger ofm;
        ofm.merger = fm;
        ofm.state = (OrganizedState)0;
        mFileMergers.push_back(ofm);
        if (mActiveOrg == 0 && !mStartOrg) {
            mStartOrg = new FileMergerOrganizerLoader();
        }
    }
}

bool FileMergerSort::operator()(
    const FileMerger::Merger *aMerger, const FileMerger::Merger *bMerger
) {
    CatData &aRef = gCatPriority[aMerger->name];
    CatData &bRef = gCatPriority[bMerger->name];
    if (bRef.priority == 0) {
        if (gOrganizing) {
            MILO_NOTIFY("unknown file merger organizer category %s", bMerger->name);
        }
        bRef.priority = gNextCatPriority++;
        bRef.unk4 = false;
    }
    if (aRef.priority == 0) {
        if (gOrganizing) {
            MILO_NOTIFY("unknown file merger organizer category %s", aMerger->name);
        }
        aRef.priority = gNextCatPriority++;
        aRef.unk4 = false;
    }

    CatData aData = aRef;
    CatData bData = bRef;

    if (aMerger->loading.empty()) {
        aData.priority -= gNextCatPriority;
        MILO_ASSERT(aData.priority < 0, 0x5F);
    }
    if (bMerger->loading.empty()) {
        bData.priority -= gNextCatPriority;
        MILO_ASSERT(bData.priority < 0, 0x64);
    }

    if (aData.unk4 && bData.unk4) {
        bool females = strstr(aMerger->loading.c_str(), "female")
            && strstr(bMerger->loading.c_str(), "female");
        if (females)
            return gGenderChirality != females;
    }
    if (aData.priority == bData.priority) {
        return strcmp(aMerger->loading.c_str(), bMerger->loading.c_str()) < 0;
    } else
        return aData.priority < bData.priority;
}
