#pragma once
#include "Accomplishment.h"
#include "HamProfile.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_map.h"
#include "stl/_pair.h"
#include "stl/_set.h"
#include "stl/_vector.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbase.h"
#include <list>

enum GamerAwardType {
    type0,
    type1
};

class GamerAwardStatus : public FixedSizeSaveable {
public:
    virtual ~GamerAwardStatus();
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    static int SaveSize(int);

    GamerAwardStatus();
    GamerAwardStatus(int, GamerAwardType);

    int unk8;
    GamerAwardType unkc;
    bool unk10;
    u32 unk14;
    u32 unk18;
    XOVERLAPPED unk1c;
};

class AccomplishmentProgress : public Hmx::Object, public FixedSizeSaveable {
public:
    // Hmx::Object
    virtual ~AccomplishmentProgress();
    virtual DataNode Handle(DataArray *, bool);
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int) const;

    static int SaveSize(int);

    AccomplishmentProgress(HamProfile *);
    int GetNiceMoveCount() const;
    void IncrementDanceBattleCount();
    void ClearAllPerfectMoves();
    void ClearPerfectStreak();
    bool HasNewAwards() const;
    void NotifyPlayerOfAccomplishment(Symbol, char const *);
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

    std::map<Symbol, int> unk34;
    HamProfile *mParentProfile; // 0x4c
    std::list<GamerAwardStatus *> unk50;
    std::set<Symbol> unk58;
    std::set<Symbol> unk70;
    std::vector<Symbol> unk88;
    std::set<Symbol> unk94;
    std::list<stlpmtx_std::pair<Symbol, Symbol> > unkac;
    int mTotalSongsPlayed; // 0xb4
    int mTotalCampaignSongsPlayed; // 0xb8
    std::map<Symbol, int> unkbc;
    int mDanceBattleCount; // 0xd4
    int mFreestylePhotoCount; // 0xd8
    bool mPerfectMovesCleared; // 0xdc
    int unke0;
    int unke4; // 0xe4
    int unke8; // 0xe8
    std::map<Symbol, int> unkec;
    int unk104;
    int mNiceMoveCount; // 0x108
    Symbol unk10c;
    int unk110;
    int unk114;
    int unk118;
    int unk11c;
    int unk120;

private:
    void GiveGamerpic(Accomplishment *);
    void GiveAvatarAsset(Accomplishment *);
};
