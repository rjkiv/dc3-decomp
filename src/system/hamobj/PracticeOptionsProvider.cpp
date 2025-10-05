#include "hamobj/PracticeOptionsProvider.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

PracticeOptionsProvider::PracticeOptionsProvider() : unk60(0), mDisablePrevious(0) {
    mPracticeOptions[0] = "ingame_slowmo";
    mPracticeOptions[1] = "ingame_skip";
    mPracticeOptions[2] = "ingame_previous";
    mPracticeOptions[3] = "ingame_video";
}

BEGIN_HANDLERS(PracticeOptionsProvider)
    HANDLE_ACTION(set_video_replay, SetVideoReplay(_msg->Int(2)))
    HANDLE_ACTION(set_slow, SetSlow(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamNavProvider)
END_HANDLERS

BEGIN_PROPSYNCS(PracticeOptionsProvider)
    SYNC_PROP(disable_previous, mDisablePrevious)
    SYNC_SUPERCLASS(HamNavProvider)
END_PROPSYNCS

BEGIN_SAVES(PracticeOptionsProvider)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(HamNavProvider)
END_SAVES

BEGIN_COPYS(PracticeOptionsProvider)
    COPY_SUPERCLASS(HamNavProvider)
    CREATE_COPY(PracticeOptionsProvider)
END_COPYS

BEGIN_LOADS(PracticeOptionsProvider)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(HamNavProvider)
END_LOADS

void PracticeOptionsProvider::Text(int, int idx, UIListLabel *list, UILabel *label) const {
    if (list->Matches("label")) {
        label->SetTextToken(mPracticeOptions[idx]);
    } else if (list->Matches("checkbox")) {
        if (idx == 3) {
            if (unk60) {
                label->SetIcon('b');
            } else {
                label->SetIcon('a');
            }
        } else {
            label->SetIcon('\0');
        }
    } else {
        label->SetTextToken(gNullStr);
    }
}

RndMat *PracticeOptionsProvider::Mat(int, int idx, UIListMesh *) const {
    static Symbol ingame_slowmo("ingame_slowmo");
    static Symbol ingame_speedup("ingame_speedup");
    switch (idx) {
    case 0:
        if (mPracticeOptions[0] == ingame_slowmo) {
            return mPracticeMats[0];
        } else {
            return mPracticeMats[1];
        }
    case 1:
        return mPracticeMats[2];
    case 2:
        return mPracticeMats[3];
    default:
        return nullptr;
    }
}

Symbol PracticeOptionsProvider::DataSymbol(int idx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (kNumPracticeOptions), 0x66);
    return mPracticeOptions[idx];
}

bool PracticeOptionsProvider::IsActive(int idx) const {
    static Symbol learn("learn");
    if (idx == 2) {
        return !mDisablePrevious;
    } else if (idx == 0 || idx == 3) {
        return TheHamProvider->Property("skills_mode", true)->Sym() == learn;
    } else
        return true;
}

void PracticeOptionsProvider::InitData(RndDir *dir) {
    if (dir) {
        mPracticeMats[0] = dir->Find<RndMat>("slowdown.mat", false);
        mPracticeMats[1] = dir->Find<RndMat>("speedup.mat", false);
        mPracticeMats[2] = dir->Find<RndMat>("skip.mat", false);
        mPracticeMats[3] = dir->Find<RndMat>("previous.mat", false);
    }
}

void PracticeOptionsProvider::Init() { REGISTER_OBJ_FACTORY(PracticeOptionsProvider); }

void PracticeOptionsProvider::SetSlow(bool slow) {
    static Symbol ingame_slowmo("ingame_slowmo");
    static Symbol ingame_speedup("ingame_speedup");
    if (slow) {
        mPracticeOptions[0] = ingame_speedup;
    } else {
        mPracticeOptions[0] = ingame_slowmo;
    }
}

void PracticeOptionsProvider::SetVideoReplay(bool replay) {
    static Symbol ingame_video("ingame_video");
    static Symbol ingame_video_stop("ingame_video_stop");
    if (replay) {
        mPracticeOptions[3] = ingame_video_stop;
    } else {
        mPracticeOptions[3] = ingame_video;
    }
    unk60 = replay;
}
