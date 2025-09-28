#pragma once
#include "MoveGraph.h"
#include "hamobj/Difficulty.h"
#include "hamobj/SongLayout.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include <map>
#include <set>

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
protected:
    MoveMgr();
    // Hmx::Object
    virtual ~MoveMgr();

public:
    virtual DataNode Handle(DataArray *, bool);

    void RegisterSongLayout(SongLayout *);
    void UnRegisterSongLayout(SongLayout *);
    Symbol PickRandomCategory();
    void
    GenerateMoveChoice(Symbol, std::vector<const MoveVariant *> &, std::vector<const MoveVariant *> &);
    const std::map<Symbol, MoveParent *> &MoveParents() const {
        return mMoveGraph.MoveParents();
    }
    const std::map<Symbol, MoveVariant *> &MoveVariants() const {
        return mMoveGraph.MoveVariants();
    }
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

    static void Init(const char *);

private:
    void LoadCategoryData(const char *);
    void LoadSubCategoryData();
    void SongInit();

protected:
    Keys<Symbol, Symbol> *mClipPropKeys[3]; // 0x2c
    int unk38; // 0x38
    Keys<Symbol, Symbol> *mPracticePropKeys; // 0x3c
    SongLayout *unk40; // 0x40
    SongLayout *unk44; // 0x44
    Keys<Symbol, Symbol> *mMovePropKeys[3]; // 0x48
    std::map<int, MoveVariant *> unk54[3];
    int mMovesDir; // 0x9c - MoveDir*
    int unka0;
    MoveGraph mMoveGraph; // 0xa4
    std::set<const MoveVariant *> unk104;
    std::vector<const MoveParent *> unk11c[2];
    std::vector<const MoveVariant *> unk134[2];
    Symbol unk14c;
    std::vector<std::pair<const MoveVariant *, const MoveVariant *> > unk150[2];
    bool unk168;
    std::vector<MoveChoiceSet> unk16c;
    std::vector<CategoryData> unk178; // 0x178 - genre data
    std::vector<CategoryData> unk184; // 0x184 - era data
    std::vector<CategoryData> unk190;
    std::vector<CategoryData> unk19c;
    ObjectDir *unk1a8; // 0x1a8
    int unk1ac; // 0x1ac - SuperEasyRemixer*
};

extern MoveMgr *TheMoveMgr;
