#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HamProfile;

struct DeferredPoints {
    int unk0; // score?
    Symbol unk4;
};

class MetagameRank : public Hmx::Object, public FixedSizeSaveable {
public:
    MetagameRank(HamProfile *);
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    void Clear();
    int GetRankInTier() const;
    int GetTier() const;
    int GetXPOfRank(int) const;
    bool HasNewRank() const;
    void AwardPoints(int, Symbol);
    void AwardPointsForTask(Symbol);
    void UpdateScore(int, const class HamPlayerData *, const class SongStatusMgr *, int, int);
    Symbol GetRankTitle() const;

    DataNode GetNextDeferredPoints(DataArray *);

    static int SaveSize(int);
    static void Preinit();
    static void Init();

    int RankNumber() { return mRankNumber; }

private:
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    bool GetOneTimeTask(Symbol, DataArray **, int *);
    int ComputeRankNumber(bool);

protected:
    int mScore; // 0x34
    bool unk38;
    char unk39[0x40];
    char unk79[0x40];
    HamProfile *mProfile; // 0xbc
    int mRankNumber; // 0xc0 - current level?
    float mPctToNextRank; // 0xc4
    bool mAtMaxRank; // 0xc8
    bool unkc9;
    bool unkca;
    std::list<DeferredPoints> mDeferredPoints; // 0xcc
};
