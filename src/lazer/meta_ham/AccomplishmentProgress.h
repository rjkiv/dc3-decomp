#pragma once
#include "Accomplishment.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"
#include <list>
#include <utility>
#include <map>
#include <set>
#include <vector>

class HamProfile;

enum GamerAwardType {
    type0,
    type1
};

class GamerAwardStatus : public FixedSizeSaveable {
public:
    GamerAwardStatus();
    GamerAwardStatus(int, GamerAwardType);
    virtual ~GamerAwardStatus();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    static int SaveSize(int);

    int unk8;
    GamerAwardType mType; // 0xc
    bool unk10;
    XUSER_AVATARASSET mAsset; // 0x14
    XOVERLAPPED mOverlapped; // 0x1c
};

class AccomplishmentProgress : public Hmx::Object, public FixedSizeSaveable {
public:
    // Hmx::Object
    virtual ~AccomplishmentProgress();
    virtual DataNode Handle(DataArray *, bool);
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    static int SaveSize(int);

    AccomplishmentProgress(HamProfile *);
    int GetNiceMoveCount() const;
    void IncrementDanceBattleCount();
    void ClearAllPerfectMoves();
    void ClearPerfectStreak();
    bool HasNewAwards() const;
    void NotifyPlayerOfAccomplishment(Symbol, const char *);
    void SetTotalSongsPlayed(int);
    void SetTotalCampaignSongsPlayed(int);
    void MovePassed(Symbol, int);
    Symbol GetFirstNewAward() const;
    Symbol GetFirstNewAwardReason() const;
    void Poll();
    bool IsAccomplished(Symbol) const;
    void ClearFirstNewAward();
    int GetNumCompletedInCategory(Symbol) const;
    int GetNumCompletedInGroup(Symbol) const;
    int GetCharacterUseCount(Symbol) const;
    int GetCount(Symbol) const;
    bool AddAward(Symbol, Symbol);
    bool AddAccomplishment(Symbol);
    void Clear();
    void IncrementCharacterUseCount(Symbol);
    void IncrementCount(Symbol, int);
    int GetTotalSongsPlayed() const;
    int GetTotalCampaignSongsPlayed() const;
    int GetNumCompleted() const;
    int GetFlawlessMoveCount() const;
    bool HasAward(Symbol s) const { return unk94.find(s) != unk94.end(); }
    int NumDays() const { return unk114; }
    void SetNumDays(int i) { unk114 = i; }
    int NumWeekends() const { return unk11c; }
    int GetUnk118() const { return unk118; }
    void SetUnk118(int i) { unk118 = i; }
    int GetUnk120() const { return unk120; }
    void SetWeekends(int i) { unk11c = i; }
    void SetUnk120(int i) { unk120 = i; }

private:
    void GiveGamerpic(Accomplishment *);
    void GiveAvatarAsset(Accomplishment *);

    std::map<Symbol, int> unk34;
    HamProfile *mParentProfile; // 0x4c
    std::list<GamerAwardStatus *> unk50;
    std::set<Symbol> unk58;
    std::set<Symbol> unk70;
    std::vector<Symbol> unk88;
    std::set<Symbol> unk94;
    // award, reason
    std::list<std::pair<Symbol, Symbol> > mNewAwards; // 0xac
    int mTotalSongsPlayed; // 0xb4
    int mTotalCampaignSongsPlayed; // 0xb8
    std::map<Symbol, int> unkbc; // 0xbc
    int mDanceBattleCount; // 0xd4
    int mFreestylePhotoCount; // 0xd8
    bool mPerfectMovesCleared; // 0xdc - completely flawless?
    int unke0;
    int unke4; // 0xe4
    int unke8; // 0xe8
    // symbol = char, int = use count
    std::map<Symbol, int> mCharacterUseCounts; // 0xec
    int mFlawlessMoveCount; // 0x104
    int mNiceMoveCount; // 0x108
    Symbol unk10c;
    int unk110;
    int unk114; // 0x114
    int unk118;
    int unk11c;
    int unk120;
};
