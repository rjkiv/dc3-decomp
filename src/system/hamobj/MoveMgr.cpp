#include "hamobj/MoveMgr.h"
#include "SongUtl.h"
#include "SuperEasyRemixer.h"
#include "char/CharClip.h"
#include "char/FileMerger.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamMove.h"
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
#include <algorithm>
#include <climits>

MoveMgr::MoveMgr() : unk40(nullptr), mPrevMoveVariant(nullptr) {
    mMovesDir = nullptr;
    for (int i = 0; i < kNumDifficultiesDC2; i++) {
        mMoveLayout[i].clear();
        mMoveKeys[i] = nullptr;
        mClipKeys[i] = nullptr;
    }
    mExpertPracticeKeys = nullptr;
    mHasRoutine = false;
    mPreferredSong = "";
    mRemixer = Hmx::Object::New<SuperEasyRemixer>();
    mRemixer->SetType("easeup_remixer");
    mMoveData = nullptr;
    unk44 = dynamic_cast<SongLayout *>(Hmx::Object::NewObject("SongLayout"));
}

MoveMgr::~MoveMgr() {
    RELEASE(mRemixer);
    mGenres.clear();
    mEras.clear();
    mSubGenres.clear();
    mSubEras.clear();
    mPrevMoveVariant = nullptr;
    mMovesDir = nullptr;
    for (int i = 0; i < kNumDifficultiesDC2; i++) {
        mMoveLayout[i].clear();
        mMoveKeys[i] = nullptr;
        mClipKeys[i] = nullptr;
    }
    mExpertPracticeKeys = nullptr;
    mMoveData = nullptr;
    RELEASE(unk44);
}

BEGIN_HANDLERS(MoveMgr)
    HANDLE_EXPR(graph_size, (int)mWholeMoveGraph.MoveParents().size())
    HANDLE_ACTION(register_song_layout, RegisterSongLayout(_msg->Obj<SongLayout>(2)))
    HANDLE_ACTION(init_song, InitSong())
    HANDLE_EXPR(get_song_layout, GetSongLayout())
    HANDLE_EXPR(get_move_difficulty, GetMoveDifficulty(_msg->ForceSym(2)))
    HANDLE_ACTION(next_moves, NextMovesToShow(_msg->Array(2), _msg->Int(3)))
    HANDLE_EXPR(get_char_clip, FindCharClip(_msg->ForceSym(2)))
    HANDLE_EXPR(get_ham_move, FindHamMove(_msg->ForceSym(2)))
    HANDLE_EXPR(move_from_move_name, FindHamMoveFromName(_msg->ForceSym(2)))
    HANDLE_EXPR(
        variant_name_from_move_name, FindVariantNameFromHamMoveName(_msg->ForceSym(2))
    )
    HANDLE_ACTION(fill_routine_from_verses, FillRoutineFromVerses(_msg->Int(2)))
    HANDLE_ACTION(fill_routine_from_replacer, FillRoutineFromReplacer(_msg->Int(2)))
    HANDLE_ACTION(prepare_next_choice_set, PrepareNextChoiceSet(_msg->Int(2)))
    HANDLE_ACTION(
        pick_random_move_set,
        PickRandomMoveSet(_msg->ForceSym(2), _msg->Int(3), _msg->Array(4), _msg->Array(5))
    )
    HANDLE_EXPR(pick_random_genre, PickRandomGenre())
    HANDLE_EXPR(get_genre_token_name, GetGenreTokenName(_msg->ForceSym(2)))
    HANDLE(find_variants, OnFindVariants)
    HANDLE_EXPR(get_remixer, mRemixer)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

Symbol MoveMgr::GetGenreTokenName(Symbol s) {
    Symbol ret = mGenres.front().tokenName;
    FOREACH (it, mGenres) {
        if (it->name == s) {
            ret = it->tokenName;
            break;
        }
    }
    return ret;
}

Difficulty MoveMgr::GetMoveDifficulty(Symbol s) {
    const MoveVariant *mv = mWholeMoveGraph.FindMoveByVariantName(s);
    if (mv) {
        return mv->GetDifficulty();
    } else {
        return DefaultDifficulty();
    }
}

void MoveMgr::Clear() {
    mPrevMoveVariant = nullptr;
    mMovesDir = 0;
    for (int i = 0; i < kNumDifficultiesDC2; i++) {
        mMoveLayout[i].clear();
        mMoveKeys[i] = nullptr;
        mClipKeys[i] = nullptr;
    }
    mExpertPracticeKeys = nullptr;
    unk40 = 0;
    mMovesNeeded.clear();
    for (int i = 0; i < 2; i++) {
        mRoutine[i].clear();
        mRoutineParents[i].clear();
        mRoutinePreferredVariants[i].clear();
    }
    mHasRoutine = false;
    mTempSongChoices.clear();
    mGenres.clear();
    mEras.clear();
    mSubGenres.clear();
    mSubEras.clear();
    mPreferredSong = "";
    mWholeMoveGraph.Clear();
    mMoveData = nullptr;
}

bool MoveVariantsWithHamMove(const MoveVariant *var, void *v) {
    int tag = var->HamMoveName();
    int *iv = (int *)v;
    return tag == *iv;
}

bool MoveMgr::HasRoutine() const {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol practice("practice");
    return mHasRoutine
        && TheHamProvider->Property(gameplay_mode, true)->Sym() != practice;
}

void MoveMgr::InsertMoveInSong(const MoveVariant *var, int measure, int player) {
    static Symbol clip("clip");
    static Symbol move("move");
    static Symbol practice("practice");
    if (var) {
        Symbol name = var->Name();
        float beat = measure * 4;
        float f4 = BeatToFrame(beat);
        float f6 = BeatToFrame(measure > 0 ? beat - 1 : 0);
        RndPropAnim *anim = TheHamDirector->SongAnim(player);
        DataArrayPtr ptr90(clip);
        DataArrayPtr ptr88(move);
        anim->SetKeyVal(TheHamDirector, ptr90, f6, name, true);
        anim->SetKeyVal(TheHamDirector, ptr88, f4, var->HamMoveName(), true);
    }
}

void MoveMgr::SaveRoutine(DataArray *a) const {
    a->Resize(mRoutineParents[0].size());
    int i = 0;
    FOREACH (it, mRoutineParents[0]) {
        if (*it) {
            a->Node(i) = (*it)->Name();
        } else {
            a->Node(i) = 0;
        }
        i++;
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
    int invalidNum = 0;
    FOREACH (it, mWholeMoveGraph.MoveParents()) {
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
    std::vector<const MoveVariant *> rightMoves;
    std::vector<const MoveVariant *> wrongMoves;
    std::set<const MoveVariant *> moveSet;
    FOREACH (it, mWholeMoveGraph.MoveParents()) {
        const MoveVariant *randomVar = it->second->PickRandomVariant();
        if (randomVar->IsValidForMinigame()) {
            if (it->second->HasCategory(s1)) {
                rightMoves.push_back(randomVar);
            } else {
                wrongMoves.push_back(randomVar);
            }
        }
    }
    std::random_shuffle(rightMoves.begin(), rightMoves.end());
    std::random_shuffle(wrongMoves.begin(), wrongMoves.end());
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
    mGenres.clear();
    mEras.clear();
    static Symbol genres("genres");
    static Symbol eras("eras");
    DataArray *file = DataReadFile(filename, false);
    if (file) {
        DataArray *genreArr = file->FindArray(genres, true);
        if (genreArr) {
            for (int i = 1; i < genreArr->Size(); i++) {
                CategoryData data;
                data.name = genreArr->Array(i)->Sym(0);
                data.tokenName = genreArr->Array(i)->Sym(1);
                mGenres.push_back(data);
            }
        }
        DataArray *eraArr = file->FindArray(eras, true);
        if (eraArr) {
            for (int i = 1; i < eraArr->Size(); i++) {
                CategoryData data;
                data.name = eraArr->Array(i)->Sym(0);
                data.tokenName = eraArr->Array(i)->Sym(1);
                mEras.push_back(data);
            }
        }
        file->Release();
    }
}

void MoveMgr::ImportMoveData(const char *filename, bool clear) {
    if (clear) {
        mWholeMoveGraph.Clear();
    }
    DataArray *moveData = DataReadFile(filename, true);
    if (moveData) {
        mWholeMoveGraph.ImportMoveData(moveData);
        moveData->Release();
    }
    MILO_LOG(
        "MoveMgr::ImportMoveData filename=%s move nodes=%d\n",
        filename,
        mWholeMoveGraph.MoveParents().size()
    );
    LoadSubCategoryData();
}

void MoveMgr::LoadMoveData(ObjectDir *dir) {
    if (mMoveData != dir) {
        mMoveData = dir;
        if (dir) {
            MoveGraph *dirGraph = dir->Find<MoveGraph>("move_graph", true);
            mWholeMoveGraph.Copy(dirGraph, kCopyDeep);
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
    if (i2 < mRoutinePreferredVariants[i1].size()) {
        const MoveVariant *var = mRoutinePreferredVariants[i1][i2];
        if (var && var->Parent() != mRoutineParents[i1].at(i2)) {
            return nullptr;
        } else {
            return var;
        }
    }
    return nullptr;
}

void MoveMgr::LoadSongData() { ImportMoveData("../meta/move_data.dta", true); }

void MoveMgr::ComputePotentialMoves(std::set<const MoveParent *> &moves, int i2) {
    if (mRoutineParents[0].size() < i2 + 1) {
        mRoutineParents[0].resize(i2 + 1);
    }
    if (mRoutineParents[0][i2]) {
        moves.insert(mRoutineParents[0][i2]);
    } else {
        if (mTempSongChoices.size() < i2 + 1) {
            mTempSongChoices.resize(i2 + 1);
        }
        MoveChoiceSet &curChoice = mTempSongChoices[i2];
        if (curChoice.mMoves[0]) {
            for (int i = 0; i < kNumDifficulties; i++) {
                if (curChoice.mMoves[i]) {
                    moves.insert(curChoice.mMoves[i]);
                }
            }
        } else {
            if (i2 > 0) {
                if (mRoutineParents[0][i2 - 1]) {
                    const MoveParent *prev = mRoutineParents[0][i2 - 1];
                    FOREACH (adj, prev->NextAdjacents()) {
                        if ((*adj)->IsValidForMiniGame()) {
                            moves.insert(*adj);
                        }
                    }
                }
            }
            if (moves.size() < 1) {
                FOREACH (it, mWholeMoveGraph.MoveParents()) {
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
    for (std::vector<const MoveParent *>::iterator it = mRoutineParents[0].begin();
         it != mRoutineParents[0].end();
         ++it, ++i) {
        if (!*it) {
            int set = ComputeRandomChoiceSet(i);
            if (set != 0) {
                set = RandomInt(0, set);
                *it = mTempSongChoices[i].mMoves[set];
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
    int size = mSubGenres.size();
    return size != 0 ? mSubGenres[RandomInt(0, size)].name : genre_none;
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
    mPrevMoveVariant = nullptr;
    MILO_ASSERT(TheHamDirector, 0x123);
    mMovesDir = TheHamDirector->GetWorld()->Find<MoveDir>("moves", false);
    MILO_ASSERT(mMovesDir, 0x126);
    static Symbol move("move");
    static Symbol clip("clip");
    static Symbol practice("practice");
    for (int i = 0; i < kNumDifficultiesDC2; i++) {
        mMoveLayout[i].clear();
        PropKeys *pMovePropKeys = TheHamDirector->GetPropKeys((Difficulty)i, move);
        PropKeys *pClipPropKeys = TheHamDirector->GetPropKeys((Difficulty)i, clip);
        MILO_ASSERT(pMovePropKeys, 0x132);
        MILO_ASSERT(pClipPropKeys, 0x133);
        mMoveKeys[i] = pMovePropKeys->AsSymbolKeys();
        mClipKeys[i] = pClipPropKeys->AsSymbolKeys();
    }
    PropKeys *practiceKeys = TheHamDirector->GetPropKeys(kDifficultyExpert, practice);
    if (practiceKeys) {
        mExpertPracticeKeys = practiceKeys->AsSymbolKeys();
    } else
        mExpertPracticeKeys = nullptr;
}

void MoveMgr::NextMovesToShow(DataArray *a, int measure) {
    MILO_LOG("MoveMgr: next moves to show for measure %d\n", measure);
    if (mTempSongChoices.size() < measure + 1) {
        mTempSongChoices.resize(measure + 1);
    }
    if (!mTempSongChoices[measure].mMoves[0]) {
        MILO_LOG("MoveMgr: oh no they are not ready yet!\n");
        PrepareNextChoiceSet(measure - 1);
    }
    for (int i = 0; i < 4; i++) {
        MILO_LOG("\t%s\n", mTempSongChoices[measure].mMoves[i]->Name());
    }
    a->Resize(4);
    for (int i = 0; i < 4; i++) {
        a->Node(i) = mTempSongChoices[measure].mMoves[i]->Name();
    }
}

void MoveMgr::PrepareNextChoiceSet(int measure) {
    ComputeRandomChoiceSet(measure + 1);
    ComputeLoadedMoveSet();
    TheHamDirector->LoadRoutineBuilderData(mMovesNeeded, true);
}

void MoveMgr::FillRoutineFromParents(int x) {
    if (x < 0) {
        x = mRoutineParents[0].size() - 1;
    }
    for (int i = 0; i <= x; i++) {
        FillInRoutineAt(0, i);
    }
    for (int i = 0; i <= x; i++) {
        const MoveVariant *var = mRoutine[0][i].first;
        for (int j = 0; j < 2; j++) {
            InsertMoveInSong(var, i, j);
        }
    }
    mHasRoutine = true;
}

void MoveMgr::ResetRemixer() {
    if (mRemixer)
        mRemixer->Reset();
}

void MoveMgr::RegisterSongLayout(SongLayout *sl) { unk40 = sl; }

void MoveMgr::UnRegisterSongLayout(SongLayout *sl) {
    if (unk40 == sl) {
        unk40 = nullptr;
    }
}

const std::pair<const MoveVariant *, const MoveVariant *> *
MoveMgr::GetRoutineMeasure(int player, int measure) const {
    int idx = mRoutine[player].size() ? player : 0;
    if (mRoutine[idx].size() <= measure) {
        return 0;
    }
    return &mRoutine[idx][measure];
}

CategoryData MoveMgr::GetCategoryByName(Symbol name) {
    for (int i = 0; i < mGenres.size(); i++) {
        if (mGenres[i].name == name) {
            return mGenres[i];
        }
    }
    for (int i = 0; i < mEras.size(); i++) {
        if (mEras[i].name == name) {
            return mEras[i];
        }
    }
    CategoryData data;
    data.name = name;
    data.tokenName = "category_unknown";
    return data;
}

void MoveMgr::SaveRoutineVariants(DataArray *a) const {
    a->Resize(mRoutine[0].size());
    int idx = 0;
    FOREACH (it, mRoutine[0]) {
        if (it->first) {
            Symbol first_name = it->first->Name();
            Symbol second_name = it->second ? it->second->Name() : first_name;
            a->Node(idx) = DataArrayPtr(first_name, second_name);
        } else {
            a->Node(idx) = DataArrayPtr(0, 0);
        }
        idx++;
    }
}

void MoveMgr::LoadRoutineVariants(const DataArray *a) {
    int aSize = a->Size();
    int idx = 0;
    auto it = mRoutine[0].begin();
    for (; it != mRoutine[0].end() && idx < aSize; ++it, ++idx) {
        mRoutineParents[0][idx] = nullptr;
        it->first = nullptr;
        it->second = nullptr;
        DataArray *curArr = a->Array(idx);
        if (curArr->Type(0) == kDataSymbol && curArr->Type(1) == kDataSymbol) {
            it->first = mWholeMoveGraph.FindMoveByVariantName(curArr->Sym(0));
            it->second = mWholeMoveGraph.FindMoveByVariantName(curArr->Sym(1));
            if (it->first) {
                mRoutineParents[0][idx] = it->first->Parent();
            }
        }
    }
    for (; it != mRoutine[0].end(); ++it, ++idx) {
        mRoutineParents[0][idx] = nullptr;
        it->first = nullptr;
        it->second = nullptr;
    }
    for (int i = 0; i < mRoutine[0].size(); i++) {
        const MoveVariant *mv = mRoutine[0][i].first;
        for (int j = 0; j < 2; j++) {
            InsertMoveInSong(mv, i, j);
        }
    }
    mHasRoutine = true;
}

HamMove *MoveMgr::FindHamMoveFromName(Symbol name) const {
    if (name == Symbol("")) {
        return nullptr;
    }

    MoveDir *moveDir = TheHamDirector->GetMoveDir();

    if (!moveDir) {
        return nullptr;
    }

    HamMove *move = moveDir->Find<HamMove>(name.Str(), false);
    if (!move) {
        move = TheHamDirector->MergerDir()->Find<HamMove>(name.Str(), false);
        if (!move) {
            MILO_NOTIFY("MoveMgr::FindHamMoveFromName couldn't find a move for %s", name);
        }
    }
    return move;
}

CharClip *MoveMgr::FindCharClip(Symbol name) const {
    Symbol nameSym = name;
    FOREACH (it, mMovesNeeded) {
        if ((*it)->Parent()->Name() == nameSym) {
            nameSym = (*it)->Name();
            break;
        }
    }
    CharClip *clip = TheHamDirector->ClipDir()->Find<CharClip>(nameSym.Str(), false);
    if (!clip) {
        MILO_LOG("Error: could not find clip for %s\n", nameSym.Str());
    }
    return clip;
}

DataNode MoveMgr::OnFindVariants(DataArray *a) {
    static Symbol unknown("unknown");
    DataArrayPtr ptr(unknown, unknown);
    const MoveParent *p1 = mWholeMoveGraph.GetMoveParent(a->ForceSym(2));
    const MoveParent *p2 = mWholeMoveGraph.GetMoveParent(a->ForceSym(3));
    const MoveVariant *mv1 = nullptr;
    const MoveVariant *mv2 = nullptr;
    mWholeMoveGraph.FindVariantPair(mv1, mv2, p1, p2, nullptr, nullptr, "", false);
    if (mv1) {
        ptr->Node(0) = mv1->Name();
    } else if (p1) {
        ptr->Node(0) = p1->Variants().front()->Name();
    } else {
        ptr->Node(0) = "<unknown>";
    }
    if (mv2) {
        ptr->Node(1) = mv2->Name();
    } else if (p2) {
        ptr->Node(1) = p2->Variants().front()->Name();
    } else {
        ptr->Node(1) = "<unknown>";
    }
    return ptr;
}

HamMove *MoveMgr::FindHamMove(Symbol name) const {
    if (*name.Str() == '\0') {
        return nullptr;
    } else {
        Symbol nameSym = name;
        FOREACH (it, mMovesNeeded) {
            if ((*it)->Parent()->Name() == nameSym) {
                nameSym = (*it)->Name();
                break;
            }
        }
        const MoveVariant *mv = mWholeMoveGraph.FindMoveByVariantName(nameSym);
        int numGraphNodes = mWholeMoveGraph.MoveParents().size();
        if (!mv) {
            MILO_LOG(
                "Error: could not find a move in graph (%d nodes) called %s\n",
                numGraphNodes,
                nameSym.Str()
            );
        } else {
            nameSym = mv->HamMoveName();
        }
        HamMove *move = TheHamDirector->GetMoveDir()->Find<HamMove>(nameSym.Str(), false);
        if (!move) {
            move = TheHamDirector->MergerDir()->Find<HamMove>(nameSym.Str(), false);
        }
        return move;
    }
}

Symbol MoveMgr::FindVariantNameFromHamMoveName(Symbol name) const {
    std::vector<const MoveVariant *> moveVariants;
    mWholeMoveGraph.GatherVariants(&moveVariants, MoveVariantsWithHamMove, &name);
    if (moveVariants.size() != 0) {
        return moveVariants.front()->Name();
    } else {
        return "";
    }
}

void MoveMgr::InitSong() {
    SongInit();
    mTempSongChoices.clear();
    int i12 = INT_MAX;
    int i13 = 0;
    unk40 = GetSongLayout();
    FOREACH (it, unk40->SongSections()) {
        if (i12 >= it->mMeasureRange.start) {
            i12 = it->mMeasureRange.start;
        }
        if (i13 <= it->mMeasureRange.end) {
            i13 = it->mMeasureRange.end;
        }
    }
    mRoutineParents[0].resize(i13 + 2);
    mRoutine[0].resize(i13 + 2);
    mTempSongChoices.resize(i13 + 2);
    mMovesNeeded.clear();
    auto itEnd = mRoutineParents[0].end();
    for (auto it = mRoutineParents[0].begin(); it != itEnd; ++it) {
        *it = nullptr;
    }
    auto itEnd2 = mRoutine[0].end();
    for (auto it = mRoutine[0].begin(); it != itEnd2; ++it) {
        *it = std::make_pair(nullptr, nullptr);
    }
    auto itEnd3 = mTempSongChoices.end();
    for (auto it = mTempSongChoices.begin(); it != itEnd3; ++it) {
        it->mMoves[0] = 0;
        it->mMoves[1] = 0;
        it->mMoves[2] = 0;
        it->mMoves[3] = 0;
    }
    ComputeRandomChoiceSet(0);
    ComputeLoadedMoveSet();
    TheHamDirector->CleanOriginalMoveData();
    LoadMoveData(mMovesDir->Find<ObjectDir>("move_data", false));
}

void MoveMgr::LoadSubCategoryData() {
    mSubGenres.clear();
    mSubEras.clear();
    std::map<Symbol, int> map70;
    std::map<Symbol, int> map90;
    map70.clear();
    map90.clear();
    FOREACH (it, mWholeMoveGraph.MoveParents()) {
        MoveParent *cur = it->second;
        if (IsSuperEasyMove(cur->Name())) {
            cur->SetSuperEasy(true);
        }
        for (int i = 0; i < it->second->mGenreFlags.size(); i++) {
            map70[it->second->mGenreFlags[i]]++;
        }
        for (int i = 0; i < it->second->mEraFlags.size(); i++) {
            map90[it->second->mEraFlags[i]]++;
        }
    }
    DataArray *superEasy = SystemConfig("super_easy_moves");
    MILO_ASSERT(superEasy, 0x540);
    FOREACH (it, map70) {
        mSubGenres.push_back(GetCategoryByName(it->first));
    }
    FOREACH (it, map90) {
        mSubEras.push_back(GetCategoryByName(it->first));
    }
}

void MoveMgr::FillRoutineFromReplacer(int x) {
    FOREACH (it, unk40->MoveReplacers()) {
        if (it->unk8) {
            FOREACH (measure, it->mMeasures) {
                mRoutineParents[0][*measure] = it->unk8;
            }
        }
    }
    FillRoutineFromParents(x);
}

void MoveMgr::FillRoutineFromVerses(int x) {
    FOREACH (it, unk40->SongSections()) {
        int secLen = (it->mMeasureRange.end - it->mMeasureRange.start) + 1;
        int patLen = (it->mPatternRange.end - it->mPatternRange.start) + 1;
        MILO_ASSERT(patLen == secLen, 0x428);
        for (int i = 0; i < it->unk14->mNumMoves; i++) {
            int measureIdx = it->mMeasureRange.start + i;
            int patternIdx = (it->mPatternRange.start + i) - 1;
            if (patternIdx >= it->unk14->mElements.size() - 1) {
                patternIdx = it->unk14->mElements.size() - 1;
            }
            mRoutineParents[0][measureIdx] = it->unk14->mMoveParents[patternIdx];
        }
    }
    FillRoutineFromParents(x);
}
