#include "hamobj/MoveMgr.h"
#include "SongUtl.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/SongLayout.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "stl/_algo.h"

MoveMgr::MoveMgr() : unk40(0), unka0(0) {
    mMovesDir = 0;
    for (int i = 0; i < 3; i++) {
        unk54[i].clear();
        mMovePropKeys[i] = nullptr;
        mClipPropKeys[i] = nullptr;
    }
    mPracticePropKeys = nullptr;
    unk168 = false;
    unk14c = "";
    //   pSVar1 = Hmx::Object::New<>();
    //   *(this + 0x1ac) = pSVar1;
    //   puVar2 = Symbol::Symbol(local_5c,"easeup_remixer");
    //   (**(**(this + 0x1ac) + 0x10))(*(this + 0x1ac),*puVar2);
    unk1a8 = nullptr;
    unk44 = dynamic_cast<SongLayout *>(Hmx::Object::NewObject("SongLayout"));
}

MoveMgr::~MoveMgr() {}

void MoveMgr::Clear() {
    unka0 = 0;
    mMovesDir = 0;
    for (int i = 0; i < 3; i++) {
        unk54[i].clear();
        mMovePropKeys[i] = nullptr;
        mClipPropKeys[i] = nullptr;
    }
    mPracticePropKeys = nullptr;
    unk40 = 0;
    unk104.clear();
    for (int i = 0; i < 2; i++) {
        unk150[i].clear();
        unk11c[i].clear();
        unk134[i].clear();
    }
    unk168 = false;
    unk16c.clear();
    unk178.clear();
    unk184.clear();
    unk190.clear();
    unk19c.clear();
    unk14c = "";
    mMoveGraph.Clear();
    unk1a8 = nullptr;
}

bool MoveVariantsWithHamMove(const MoveVariant *var, void *v) {
    int tag = var->HamMoveName();
    int *iv = (int *)v;
    return tag == *iv;
}

bool MoveMgr::HasRoutine() const {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol practice("practice");
    if (unk168) {
        if (TheHamProvider->Property(gameplay_mode, true)->Sym() != practice) {
            return true;
        }
    }
    return false;
}

void MoveMgr::InsertMoveInSong(const MoveVariant *var, int i2, int i3) {
    static Symbol clip("clip");
    static Symbol move("move");
    static Symbol practice("practice");
    if (var) {
        Symbol name = var->Name();
        float beat = i2 * 4;
        float f4 = BeatToFrame(beat);
        float f6 = BeatToFrame(i2 > 0 ? beat - 1 : 0);
        RndPropAnim *anim = TheHamDirector->SongAnim(i3);
        DataArrayPtr ptr90(clip);
        DataArrayPtr ptr88(move);
        anim->SetKeyVal(TheHamDirector, ptr90, f6, name, true);
        anim->SetKeyVal(TheHamDirector, ptr88, f4, var->HamMoveName(), true);
    }
}

void MoveMgr::SaveRoutine(DataArray *a) const {
    a->Resize(unk11c[0].size());
    int i = 0;
    for (std::vector<const MoveParent *>::const_iterator it = unk11c[0].begin();
         it != unk11c[0].end();
         ++it, ++i) {
        if (*it) {
            // a->Node(i) = (*it)->unk4;
        } else {
            a->Node(i) = 0;
        }
    }
}

void MoveMgr::GenerateMoveChoice(
    Symbol s1,
    std::vector<const MoveVariant *> &vec1,
    std::vector<const MoveVariant *> &vec2
) {
    vec1.clear();
    vec2.clear();
    MILO_LOG("MoveMgr::GenerateMoveChoice %s\n", s1.Str());
    const std::map<Symbol, MoveParent *> &parents = MoveParents();
    int invalidNum = 0;
    for (std::map<Symbol, MoveParent *>::const_iterator it = parents.begin();
         it != parents.end();
         ++it) {
        MoveParent *curParent = it->second;
        const MoveVariant *randomVar = curParent->PickRandomVariant();
        MILO_LOG("move=%s genre=%s", randomVar->Name(), randomVar->Genre().Str());
        if (!randomVar->IsValidForMinigame()) {
            MILO_LOG(" not valid for mini games\n");
            invalidNum++;
        } else {
            if (curParent->HasCategory(s1)) {
                MILO_LOG(" is %s\n", s1.Str());
                vec1.push_back(randomVar);
            } else {
                MILO_LOG(" NOT %s\n", s1.Str());
                vec2.push_back(randomVar);
            }
        }
    }
    MILO_LOG(
        "invalid=%d, wrong genre=%d right genre=%d\n", invalidNum, vec2.size(), vec1.size()
    );
    std::random_shuffle(vec1.begin(), vec1.end());
    std::random_shuffle(vec2.begin(), vec2.end());
}

void MoveMgr::PickRandomMoveSet(Symbol s1, int count, DataArray *a3, DataArray *a4) {
    MILO_ASSERT(count > 1, 0x4BB);
    a3->Resize(1);
    a4->Resize(count - 1);
    std::vector<const MoveVariant *> wrongMoves;
    std::vector<const MoveVariant *> rightMoves;
    std::set<const MoveVariant *> moveSet;
    for (std::map<Symbol, MoveParent *>::const_iterator it = MoveParents().begin();
         it != MoveParents().end();
         ++it) {
        const MoveVariant *randomVar = it->second->PickRandomVariant();
        if (randomVar->IsValidForMinigame()) {
            if (it->second->HasCategory(s1)) {
                wrongMoves.push_back(randomVar);
            } else {
                rightMoves.push_back(randomVar);
            }
        }
    }
    std::random_shuffle(wrongMoves.begin(), wrongMoves.end());
    std::random_shuffle(rightMoves.begin(), rightMoves.end());
    MILO_ASSERT(rightMoves.size() > 0, 0x4D9);
    MILO_ASSERT(wrongMoves.size() > count - 1, 0x4DA);
    const MoveVariant *firstRightMove = rightMoves[0];
    moveSet.insert(firstRightMove);
    a3->Node(0) = firstRightMove->Name();
    for (int i = 0; i < count - 1; i++) {
        const MoveVariant *curWrongMove = wrongMoves[i];
        moveSet.insert(curWrongMove);
        a4->Node(i) = curWrongMove->Name();
    }
    TheHamDirector->LoadRoutineBuilderData(moveSet, true);
}

void MoveMgr::LoadCategoryData(const char *filename) {
    unk178.clear();
    unk184.clear();
    static Symbol genres("genres");
    static Symbol eras("eras");
    DataArray *file = DataReadFile(filename, false);
    if (file) {
        DataArray *genreArr = file->FindArray(genres, true);
        if (genreArr) {
            for (int i = 1; i < genreArr->Size(); i++) {
                CategoryData data;
                data.unk0 = genreArr->Array(i)->Sym(0);
                data.unk4 = genreArr->Array(i)->Sym(1);
                unk178.push_back(data);
            }
        }
        DataArray *eraArr = file->FindArray(eras, true);
        if (eraArr) {
            for (int i = 1; i < eraArr->Size(); i++) {
                CategoryData data;
                data.unk0 = eraArr->Array(i)->Sym(0);
                data.unk4 = eraArr->Array(i)->Sym(1);
                unk184.push_back(data);
            }
        }
        file->Release();
    }
}

void MoveMgr::ImportMoveData(const char *filename, bool clear) {
    if (clear) {
        mMoveGraph.Clear();
    }
    DataArray *moveData = DataReadFile(filename, true);
    if (moveData) {
        mMoveGraph.ImportMoveData(moveData);
        moveData->Release();
    }
    MILO_LOG(
        "MoveMgr::ImportMoveData filename=%s move nodes=%d\n",
        filename,
        MoveParents().size()
    );
    LoadSubCategoryData();
}

void MoveMgr::LoadMoveData(ObjectDir *dir) {
    if (unk1a8 != dir) {
        unk1a8 = dir;
        if (dir) {
            MoveGraph *dirGraph = dir->Find<MoveGraph>("move_graph", true);
            mMoveGraph.Copy(dirGraph, kCopyDeep);
            LoadSubCategoryData();
        } else {
            MILO_LOG("MoveMgr::LoadMoveData - NULL dir passed in. Doing nothing...\n");
        }
    } else {
        MILO_LOG("MoveMgr::LoadMoveData - Dir is already loaded. Doing nothing...\n");
    }
}

void MoveMgr::Init(const char *filename) {
    TheMoveMgr = new MoveMgr();
    if (ObjectDir::Main()) {
        TheMoveMgr->SetName("movemgr", ObjectDir::Main());
    }
    TheMoveMgr->LoadCategoryData("modular_song_data/category.dta");
    if (filename) {
        TheMoveMgr->ImportMoveData(filename, false);
    }
}

const MoveVariant *MoveMgr::GetRoutinePreferredVariant(int i1, int i2) const {
    if (i2 < unk134[i1].size()) {
        const MoveVariant *var = unk134[i1][i2];
        if (var && var->Parent() == unk11c[i1].at(i2)) {
            return var;
        }
    }
    return nullptr;
}

void MoveMgr::LoadSongData() { ImportMoveData("../meta/move_data.dta", true); }

void MoveMgr::ComputePotentialMoves(std::set<const MoveParent *> &moves, int i2) {
    unk11c[0].resize(i2 + 1);
    if (unk11c[0][i2]) {
        moves.insert(unk11c[0][i2]);
    } else {
        if (unk16c.size() < i2 + 1) {
            unk16c.resize(i2 + 1);
        }
        MoveChoiceSet &curChoice = unk16c[i2];
        if (curChoice.unk0[0]) {
            for (int i = 0; i < kNumDifficulties; i++) {
                if (curChoice.unk0[i]) {
                    moves.insert(curChoice.unk0[i]);
                }
            }
        } else {
            // there's another loop here
            if (moves.size() < 1) {
                for (std::map<Symbol, MoveParent *>::const_iterator it =
                         MoveParents().begin();
                     it != MoveParents().end();
                     ++it) {
                    MoveParent *cur = it->second;
                    if (cur->IsValidForMiniGame()) {
                        moves.insert(cur);
                    }
                }
            }
        }
    }
}

void MoveMgr::AutoFillParents() {
    int i = 0;
    for (std::vector<const MoveParent *>::iterator it = unk11c[0].begin();
         it != unk11c[0].end();
         ++it, ++i) {
        if (!*it) {
            int set = ComputeRandomChoiceSet(i);
            if (set != 0) {
                set = RandomInt(0, set);
                *it = unk16c[i].unk0[set];
            }
        }
    }
}

SongLayout *MoveMgr::GetSongLayout() {
    if (!unk40) {
        unk40 = unk44;
        unk44->SetDefaultPattern(0x40);
    }
    if (unk40->NumReplacers() == 0) {
        unk40->SetDefaultReplacer();
    }
    return unk40;
}

Symbol MoveMgr::PickRandomGenre() {
    static Symbol genre_none("genre_none");
    int size = unk190.size();
    if (size != 0) {
        return unk190[RandomInt(0, size)].unk0;
    } else
        return genre_none;
}

Symbol MoveMgr::PickRandomCategory() { return PickRandomGenre(); }

bool IsSuperEasyMove(Symbol move) {
    DataArray *superEasy = SystemConfig("super_easy_moves");
    MILO_ASSERT(superEasy, 0x514);
    for (int i = 0; i < superEasy->Size(); i++) {
        if (superEasy->Sym(i) == move) {
            return true;
        }
    }
    return false;
}

void MoveMgr::SongInit() {
    unka0 = 0;
    MILO_ASSERT(TheHamDirector, 0x123);
    // mMovesDir gets assigned here
    static Symbol move("move");
    static Symbol clip("clip");
    static Symbol practice("practice");
    for (int i = 0; i < 3; i++) {
        unk54[i].clear();
        PropKeys *pMovePropKeys = TheHamDirector->GetPropKeys((Difficulty)i, move);
        PropKeys *pClipPropKeys = TheHamDirector->GetPropKeys((Difficulty)i, clip);
        MILO_ASSERT(pMovePropKeys, 0x132);
        MILO_ASSERT(pClipPropKeys, 0x133);
        mMovePropKeys[i] = pMovePropKeys->AsSymbolKeys();
        mClipPropKeys[i] = pClipPropKeys->AsSymbolKeys();
    }
    PropKeys *practiceKeys = TheHamDirector->GetPropKeys(kDifficultyExpert, practice);
    if (practiceKeys) {
        mPracticePropKeys = practiceKeys->AsSymbolKeys();
    } else
        mPracticePropKeys = nullptr;
}

void MoveMgr::NextMovesToShow(DataArray *a, int measure) {
    MILO_LOG("MoveMgr: next moves to show for measure %d\n", measure);
    if (unk16c.size() < measure + 1) {
        unk16c.resize(measure + 1);
    }
    if (!unk16c[measure].unk0[0]) {
        MILO_LOG("MoveMgr: oh no they are not ready yet!\n");
        PrepareNextChoiceSet(measure - 1);
    }
    for (int i = 0; i < 4; i++) {
        // MILO_LOG("\t%s\n", unk16c[measure].unk0[i]->unk4);
    }
    a->Resize(4);
}

void MoveMgr::PrepareNextChoiceSet(int measure) {
    ComputeRandomChoiceSet(measure + 1);
    ComputeLoadedMoveSet();
    TheHamDirector->LoadRoutineBuilderData(unk104, true);
}

void MoveMgr::FillRoutineFromParents(int x) {
    if (x < 0) {
        x = unk11c[0].size() - 1;
    }
    for (int i = 0; i <= x; i++) {
        FillInRoutineAt(0, i);
    }
    for (int i = 0; i <= x; i++) {
        const MoveVariant *var = unk150[0][i].first;
        for (int j = 0; j < 2; j++) {
            InsertMoveInSong(var, i, j);
        }
    }
    unk168 = true;
}
