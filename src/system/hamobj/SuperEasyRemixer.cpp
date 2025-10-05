#include "hamobj/SuperEasyRemixer.h"
#include "OriginalChoreoRemixer.h"
#include "SuperEasyRemixer.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamSupereasyData.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/MoveMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/DataPointMgr.h"
#include "world/Dir.h"

SuperEasyRemixer::SuperEasyRemixer() {}

BEGIN_HANDLERS(SuperEasyRemixer)
    HANDLE_SUPERCLASS(OriginalChoreoRemixer)
    HANDLE_EXPR(super_easy_data_error, unk124)
    HANDLE_ACTION(
        send_downgrade_datapoint,
        SendDowngradeDatapoint(
            _msg->Sym(2),
            _msg->Int(3),
            _msg->Sym(4),
            _msg->Int(5),
            _msg->Int(6),
            _msg->Int(7),
            _msg->Str(8)
        )
    )
END_HANDLERS

BEGIN_PROPSYNCS(SuperEasyRemixer)
    SYNC_SUPERCLASS(OriginalChoreoRemixer)
END_PROPSYNCS

BEGIN_SAVES(SuperEasyRemixer)
    SAVE_SUPERCLASS(OriginalChoreoRemixer)
END_SAVES

BEGIN_COPYS(SuperEasyRemixer)
    COPY_SUPERCLASS(OriginalChoreoRemixer)
    CREATE_COPY(SuperEasyRemixer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSuperEasyParents)
        COPY_MEMBER(mSuperEasyVariants)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(SuperEasyRemixer)
    LOAD_SUPERCLASS(OriginalChoreoRemixer)
END_LOADS

void SuperEasyRemixer::Reset() { OriginalChoreoRemixer::Reset(); }

void SuperEasyRemixer::Init() {
    OriginalChoreoRemixer::Init();
    SaveSuperEasyMoveParents();
    for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        unsigned int numParents = GetMoveParentsByDifficulty(d).size();
        if (numParents != mTotalMeasures) {
            MILO_NOTIFY(
                "this song has wrong number of measures in %s track (has %d, want %d)",
                DifficultyToSym(d).Str(),
                numParents,
                mTotalMeasures
            );
        }
    }
    DumpSongLayout();
}

std::vector<const MoveParent *> &SuperEasyRemixer::GetMoveParentsByDifficulty(int diff) {
    if (diff == kDifficultyBeginner) {
        return mSuperEasyParents;
    } else
        return OriginalChoreoRemixer::GetMoveParentsByDifficulty(diff);
}

std::vector<const MoveVariant *> &SuperEasyRemixer::GetMoveVariantsByDifficulty(int diff
) {
    if (diff == kDifficultyBeginner) {
        return mSuperEasyVariants;
    } else
        return OriginalChoreoRemixer::GetMoveVariantsByDifficulty(diff);
}

bool InsertVariants(std::set<const MoveVariant *> &vars, Symbol name) {
    const MoveVariant *mv = TheMoveMgr->Graph().FindMoveByVariantName(name);
    const MoveParent *mp = mv ? mv->Parent() : nullptr;
    if (!mp) {
        return false;
    } else {
        for (int i = 0; i < mp->Variants().size(); i++) {
            const MoveVariant *v = mp->Variants()[i];
            vars.insert(v);
        }
        return true;
    }
}

void SuperEasyRemixer::DumpSongLayout() {
    MILO_LOG("\tSUPEREASY\t\tEASY\t\tMEDIUM\t\tHARD\n");
    String str;
    for (int i = 0; i < mTotalMeasures; i++) {
        str = MakeString("%d", i + 1);
        for (Difficulty d = EasiestDifficulty(); d != kNumDifficulties;
             d = DifficultyOneHarder(d)) {
            str += "\t";
            // there's a string print here
            str.Print(GetMoveParentsByDifficulty(d)[i]->Name().Str());
            Difficulty next = DifficultyOneHarder(d);
            if (next != kNumDifficulties) {
                str += "\t";
                if (i - 1 >= 0) {
                    if (TheMoveMgr->HasVariantPair(
                            GetMoveParentsByDifficulty(next)[i - 1],
                            GetMoveParentsByDifficulty(d)[i]
                        )) {
                        str += "<";
                    } else {
                        str += "_";
                    }
                    if (TheMoveMgr->HasVariantPair(
                            GetMoveParentsByDifficulty(d)[i - 1],
                            GetMoveParentsByDifficulty(next)[i]
                        )) {
                        str += ">";
                    } else {
                        str += "_";
                    }
                }
            }
        }
        if (i == mFromMeasure) {
            str += "\tjump_from";
        }
        if (i == mToMeasure) {
            str += "\tjump_to";
        }
        str += "\n";
        MILO_LOG(str.c_str());
    }
}

void SuperEasyRemixer::SaveSuperEasyMoveParents() {
    mSuperEasyVariants.clear();
    mSuperEasyParents.clear();
    mSuperEasyVariants.reserve(mTotalMeasures);
    mSuperEasyParents.reserve(mTotalMeasures);
    int i7 = 1;
    HamSupereasyData *data =
        ObjDirItr<HamSupereasyData>(TheHamDirector->GetMoveDir(), false);
    if (data) {
        if (data->mRoutine.size() != mTotalMeasures) {
            MILO_FAIL(
                "HamSuperEasyData has wrong number of measures in routine in song '%s'",
                TheGameData->GetSong()
            );
        }
        for (int i = 0; i < data->mRoutine.size(); i++) {
            HamSupereasyMeasure &curMeasure = data->mRoutine[i];
            if (curMeasure.preferred.Null() && curMeasure.first.Null()) {
                mSuperEasyVariants.push_back(nullptr);
            } else {
                const MoveVariant *mv =
                    TheMoveMgr->Graph().FindMoveByVariantName(curMeasure.preferred);
                if (!mv) {
                    MILO_FAIL(
                        "'%s' HamSupereasyData has move '%s' at index %d not found in move graph",
                        TheGameData->GetSong().Str(),
                        curMeasure.preferred,
                        i
                    );
                    i7 = 0;
                    break;
                }
                mSuperEasyVariants.push_back(mv);
            }
        }
        if (i7 != 0) {
            for (int i = 0; i < mSuperEasyVariants.size(); i++) {
                MoveParent *parent = nullptr;
                if (mSuperEasyVariants[i]) {
                    parent = mSuperEasyVariants[i]->Parent();
                }
                mSuperEasyParents.push_back(parent);
            }
            BridgeGapsInMoveParents(3);
        }
    } else {
        i7 = 0;
        MILO_NOTIFY("No HamSupereasyData found for song '%s'", TheGameData->GetSong());
    }
    if (i7 == 0) {
        MILO_NOTIFY("Supereasy will use the easy track for '%s'", TheGameData->GetSong());
        mSuperEasyParents = GetMoveParentsByDifficulty(kDifficultyEasy);
        mSuperEasyVariants = GetMoveVariantsByDifficulty(kDifficultyEasy);
    }
    unk124 = i7 == 0;
}

void SuperEasyRemixer::LoadAllVariants() {
    std::set<const MoveVariant *> vars;
    const char *song = TheGameData->GetSong().Str();
    if (TheMoveMgr->MoveParents().size() == 0) {
        MILO_FAIL("Failed to load move graph for: %s\n", song);
    }
    DataArray *layout = TheMoveMgr->Graph().Layout();
    if (!layout) {
        MILO_FAIL("couldn't load layout for: %s", song);
    }
    for (int i = 0; i < 3; i++) {
        Symbol diffSym = DifficultyToSym((Difficulty)i);
        DataArray *a = layout->FindArray(diffSym, true)->Array(1);
        if (a->Size() == 0) {
            MILO_FAIL(
                "%s's %s layout is not stored in its move graph", song, diffSym.Str()
            );
        }
        for (int j = 0; j < a->Size(); j++) {
            Symbol s = a->Sym(j);
            if (!InsertVariants(vars, s)) {
                MILO_NOTIFY(
                    "%s's %s layout, at index %d, (%s) not found in move graph",
                    song,
                    diffSym.Str(),
                    j,
                    s.Str()
                );
            }
        }
    }
    WorldDir *world = TheHamDirector->GetWorld();
    MILO_ASSERT(world, 0x12B);
    ObjectDir *hamMoves = world->Find<ObjectDir>("moves", true);
    MILO_ASSERT(hamMoves, 0x12D);
    HamSupereasyData *data = ObjDirItr<HamSupereasyData>(hamMoves, false);
    if (data) {
        for (int i = 0; i < data->mRoutine.size(); i++) {
            HamSupereasyMeasure &curMeasure = data->mRoutine[i];
            Symbol name = curMeasure.second;
            if (name.Null())
                name = curMeasure.first;
            if (!InsertVariants(vars, name)) {
                MILO_NOTIFY(
                    "%s's supereasy layout, at index %d, (%s) not found in move graph",
                    song,
                    i,
                    name.Str()
                );
            }
        }
    } else {
        MILO_NOTIFY("No HamSupereasyData found for song '%s'", TheGameData->GetSong());
    }
    TheHamDirector->LoadRoutineBuilderData(vars, true);
}

void SuperEasyRemixer::SendDowngradeDatapoint(
    Symbol s1, int i2, Symbol s3, int i4, int i5, int i6, Symbol s7
) {
    if (i6 > 0) {
        static Symbol se_song("se_song");
        static Symbol se_player("se_player");
        static Symbol se_mode("se_mode");
        static Symbol se_shortened("se_shortened");
        static Symbol se_freestyle("se_freestyle");
        static Symbol se_measure("se_measure");
        static Symbol se_move("se_move");
        SendDataPoint(
            "super_easy_downgrade",
            se_song,
            s1,
            se_player,
            i2,
            se_mode,
            s3,
            se_shortened,
            i4,
            se_freestyle,
            i5,
            se_measure,
            i6,
            se_move,
            s7
        );
    }
}
