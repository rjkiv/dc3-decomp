#pragma once
#include "char/CharClip.h"
#include "hamobj/Difficulty.h"
#include "hamobj/Ham.h"
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

    bool operator==(const CategoryData &rhs) const {
        return name == rhs.name && tokenName == rhs.tokenName;
    }
};

class MoveChoiceSet {
public:
    MoveChoiceSet() { Clear(); }
    void Clear() {
        for (int i = 0; i < kNumDifficulties; i++) {
            mMoves[i] = nullptr;
        }
    }

    const MoveParent *mMoves[kNumDifficulties];
};

class MoveMgr : public Hmx::Object {
private:
    CategoryData GetCategoryByName(Symbol name);
    void LoadCategoryData(const char *fileName);
    void LoadSubCategoryData();
    void SongInit();

    DataNode OnFindVariants(DataArray *);

protected:
    MoveMgr();
    // Hmx::Object
    virtual ~MoveMgr();

public:
    virtual DataNode Handle(DataArray *, bool);

    void AutoFillParents();
    void Clear();
    void ComputeLoadedMoveSet();
    void ComputePotentialMoves(std::set<const MoveParent *> &result, int iMoveIndex);
    int ComputeRandomChoiceSet(int iMoveIndex);
    void FillInRoutineAt(int player, int measure);
    void FillRoutineFromParents(int highest_measure);
    void FillRoutineFromReplacer(int highest_measure);
    void FillRoutineFromVerses(int highest_measure);
    CharClip *FindCharClip(Symbol moveVariantOrParentName) const;
    HamMove *FindHamMove(Symbol moveName) const;
    HamMove *FindHamMoveFromName(Symbol hamMoveName) const;
    Symbol FindVariantNameFromHamMoveName(Symbol hamMoveName) const;
    // public: const MoveParent* FindParentMoveByExtName(const char*) const;
    // public: void FinishGraphLoad();
    void GenerateMoveChoice(
        Symbol genre,
        std::vector<const MoveVariant *> &rightChoice,
        std::vector<const MoveVariant *> &wrongChoice
    );
    // public: Symbol GenerateMoveParentName(const char*, Symbol, int32_t, bool);
    // public: const char* GetExtName(Symbol);
    Symbol GetGenreTokenName(Symbol);
    Difficulty GetMoveDifficulty(Symbol);
    // public: void GetRandomMoveSet(int32_t,
    // std::vector<MoveVariant const *>& std::vector<MoveVariant const *>&);
    const std::pair<const MoveVariant *, const MoveVariant *> *
    GetRoutineMeasure(int aPlayer, int aMeasure) const;
    const MoveVariant *GetRoutinePreferredVariant(int aPlayer, int aMeasure) const;
    // public: Symbol GetPreferredSong() const;
    void SetPreferredSong(Symbol song) { mPreferredSong = song; }
    SongLayout *GetSongLayout();
    bool HasRoutine() const;
    void ImportMoveData(const char *movesFileName, bool clear);
    void InitSong();
    void InsertMoveInSong(const MoveVariant *pMoveVariant, int iMeasure, int iPlayer);
    // public: void InsertMoveInSong(const MoveVariant*, int32_t);
    // public: bool IsGraphLoaded() const;
    // public: void LoadRoutine(const DataArray*);
    void LoadRoutineVariants(const DataArray *src);
    void LoadSongData();
    void LoadMoveData(ObjectDir *);
    void NextMovesToShow(DataArray *result, int iMoveIndex);
    Symbol PickRandomCategory();
    // public: Symbol PickRandomEra();
    Symbol PickRandomGenre();
    void PickRandomMoveSet(
        Symbol genre, int count, DataArray *rightArray, DataArray *wrongArray
    );
    void PrepareNextChoiceSet(int iMoveIndex);
    // public: void PrintRoutine(int32_t) const;
    void RegisterSongLayout(SongLayout *layout);
    void UnRegisterSongLayout(SongLayout *layout);
    void SaveRoutine(DataArray *result) const;
    void SaveRoutineVariants(DataArray *result) const;
    // public: void StartGraphLoad(const char*, bool, bool);
    void ResetRemixer();

    static void Init(const char *movesFileName);

    Keys<Symbol, Symbol> *mClipKeys[kNumDifficultiesDC2]; // 0x2c
    ObjectDir *mClipsDir; // 0x38
    Keys<Symbol, Symbol> *mExpertPracticeKeys; // 0x3c
    SongLayout *mLayout; // 0x40
    SongLayout *mSongLayout; // 0x44 - TODO rename this maybe
    Keys<Symbol, Symbol> *mMoveKeys[kNumDifficultiesDC2]; // 0x48
    std::map<int, MoveVariant *> mMoveLayout[kNumDifficultiesDC2]; // 0x54
    MoveDir *mMovesDir; // 0x9c
    MoveVariant *mPrevMoveVariant; // 0xa0
    MoveGraph mWholeMoveGraph; // 0xa4
    std::set<const MoveVariant *> mMovesNeeded; // 0x104
    // indexed by number of players, then number of measures in the song
    std::vector<const MoveParent *> mRoutineParents[MAX_NUM_PLAYERS]; // 0x11c
    // indexed by number of players, then number of measures in the song
    std::vector<const MoveVariant *> mRoutinePreferredVariants[MAX_NUM_PLAYERS]; // 0x134
    Symbol mPreferredSong; // 0x14c
    // indexed by number of players, then number of measures in the song
    std::vector<std::pair<const MoveVariant *, const MoveVariant *> >
        mRoutine[MAX_NUM_PLAYERS]; // 0x150
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
