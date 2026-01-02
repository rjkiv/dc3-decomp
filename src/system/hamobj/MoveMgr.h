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
    Symbol unk0;
    Symbol unk4;
};

class MoveChoiceSet {
public:
    const MoveParent *unk0[kNumDifficulties];
};

class MoveMgr : public Hmx::Object {
    friend class DanceRemixer;

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
    const std::map<Symbol, MoveParent *> &MoveParents() const {
        return mMoveGraph.MoveParents();
    }
    const std::map<Symbol, MoveVariant *> &MoveVariants() const {
        return mMoveGraph.MoveVariants();
    }
    const DataArrayPtr &Layout() const { return mMoveGraph.Layout(); }
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

    std::vector<const MoveParent *> &CurParents(int i) { return mMoveParents[i]; }
    bool HasVariantPair(const MoveParent *p1, const MoveParent *p2) const {
        return mMoveGraph.HasVariantPair(p1, p2);
    }
    MoveGraph &Graph() { return mMoveGraph; }
    ObjectDir *MoveDataDir() const { return mMoveDataDir; }
    void SetSong(Symbol song) { unk14c = song; }

    static void Init(const char *);

private:
    void LoadCategoryData(const char *);
    void LoadSubCategoryData();
    void SongInit();
    CategoryData GetCategoryByName(Symbol);

    DataNode OnFindVariants(DataArray *);

protected:
    Keys<Symbol, Symbol> *mClipPropKeys[kNumDifficultiesDC2]; // 0x2c
    int unk38; // 0x38
    Keys<Symbol, Symbol> *mPracticePropKeys; // 0x3c
    SongLayout *unk40; // 0x40
    SongLayout *unk44; // 0x44
    Keys<Symbol, Symbol> *mMovePropKeys[kNumDifficultiesDC2]; // 0x48
    std::map<int, MoveVariant *> unk54[kNumDifficultiesDC2]; // 0x54
    MoveDir *mMovesDir; // 0x9c
    int unka0; // 0xa0
    MoveGraph mMoveGraph; // 0xa4
    std::set<const MoveVariant *> unk104; // 0x104
    // indexed by number of players
    std::vector<const MoveParent *> mMoveParents[2]; // 0x11c
    // indexed by number of players
    std::vector<const MoveVariant *> unk134[2]; // 0x134
    Symbol unk14c; // 0x14c
    // indexed by number of players
    std::vector<std::pair<const MoveVariant *, const MoveVariant *> > unk150[2]; // 0x150
    bool unk168; // 0x168
    std::vector<MoveChoiceSet> unk16c; // 0x16c
    std::vector<CategoryData> unk178; // 0x178 - genre data
    std::vector<CategoryData> unk184; // 0x184 - era data
    std::vector<CategoryData> unk190; // 0x190
    std::vector<CategoryData> unk19c; // 0x19c
    ObjectDir *mMoveDataDir; // 0x1a8
    SuperEasyRemixer *mSuperEasyRemixer; // 0x1ac
};

extern MoveMgr *TheMoveMgr;
