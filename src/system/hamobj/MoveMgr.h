#pragma once
#include "char/CharClip.h"
#include "hamobj/Difficulty.h"
#include "hamobj/MoveGraph.h"
#include "hamobj/SongLayout.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include <map>
#include <set>

class HamMove;
class MoveDir;
class SuperEasyRemixer;

class CategoryData {
public:
    Symbol name; // 0x0
    Symbol tokenName; // 0x4
};

class MoveChoiceSet {
public:
    MoveChoiceSet() {
        for (int i = 0; i < kNumDifficulties; i++) {
            mMoves[i] = nullptr;
        }
    }

    const MoveParent *mMoves[kNumDifficulties];
};

class MoveMgr : public Hmx::Object {
private:
    void LoadCategoryData(const char *);
    void LoadSubCategoryData();
    void SongInit();
    CategoryData GetCategoryByName(Symbol);

    DataNode OnFindVariants(DataArray *);

protected:
    MoveMgr();
    // Hmx::Object
    virtual ~MoveMgr();

public:
    virtual DataNode Handle(DataArray *, bool);

    void RegisterSongLayout(SongLayout *);
    void UnRegisterSongLayout(SongLayout *);
    Symbol PickRandomCategory();
    void GenerateMoveChoice(
        Symbol, std::vector<const MoveVariant *> &, std::vector<const MoveVariant *> &
    );
    void Clear();
    bool HasRoutine() const;
    void InsertMoveInSong(const MoveVariant *, int, int);
    void SaveRoutine(DataArray *) const;
    void PickRandomMoveSet(Symbol, int, DataArray *, DataArray *);
    void ImportMoveData(const char *, bool);
    void LoadMoveData(ObjectDir *);
    const MoveVariant *GetRoutinePreferredVariant(int, int) const;
    void LoadSongData();
    void ComputePotentialMoves(std::set<const MoveParent *> &, int);
    int ComputeRandomChoiceSet(int);
    void ComputeLoadedMoveSet();
    void AutoFillParents();
    void FillInRoutineAt(int, int);
    void FillRoutineFromParents(int);
    void FillRoutineFromVerses(int);
    void FillRoutineFromReplacer(int);
    void InitSong();
    void PrepareNextChoiceSet(int);
    void NextMovesToShow(DataArray *, int);
    SongLayout *GetSongLayout();
    Symbol PickRandomGenre();
    const std::pair<const MoveVariant *, const MoveVariant *> *
    GetRoutineMeasure(int, int) const;
    void ResetRemixer();
    void SaveRoutineVariants(DataArray *) const;
    void LoadRoutineVariants(const DataArray *);
    HamMove *FindHamMoveFromName(Symbol) const;
    CharClip *FindCharClip(Symbol) const;
    HamMove *FindHamMove(Symbol) const;
    Difficulty GetMoveDifficulty(Symbol);
    Symbol FindVariantNameFromHamMoveName(Symbol) const;
    Symbol GetGenreTokenName(Symbol);

    void SetPreferredSong(Symbol song) { mPreferredSong = song; }

    static void Init(const char *);

    Keys<Symbol, Symbol> *mClipKeys[kNumDifficultiesDC2]; // 0x2c
    ObjectDir *mClipsDir; // 0x38
    Keys<Symbol, Symbol> *mExpertPracticeKeys; // 0x3c
    SongLayout *unk40; // 0x40
    SongLayout *unk44; // 0x44
    Keys<Symbol, Symbol> *mMoveKeys[kNumDifficultiesDC2]; // 0x48
    std::map<int, MoveVariant *> mMoveLayout[kNumDifficultiesDC2]; // 0x54
    MoveDir *mMovesDir; // 0x9c
    MoveVariant *mPrevMoveVariant; // 0xa0
    MoveGraph mWholeMoveGraph; // 0xa4
    std::set<const MoveVariant *> mMovesNeeded; // 0x104
    // indexed by number of players
    std::vector<const MoveParent *> mRoutineParents[2]; // 0x11c
    // indexed by number of players
    std::vector<const MoveVariant *> mRoutinePreferredVariants[2]; // 0x134
    Symbol mPreferredSong; // 0x14c
    // indexed by number of players
    std::vector<std::pair<const MoveVariant *, const MoveVariant *> > mRoutine[2]; // 0x150
    bool mHasRoutine; // 0x168
    std::vector<MoveChoiceSet> mTempSongChoices; // 0x16c
    std::vector<CategoryData> mGenres; // 0x178
    std::vector<CategoryData> mEras; // 0x184
    std::vector<CategoryData> mSubGenres; // 0x190
    std::vector<CategoryData> mSubEras; // 0x19c
    ObjectDir *mMoveData; // 0x1a8
    SuperEasyRemixer *mRemixer; // 0x1ac
};

extern MoveMgr *TheMoveMgr;
