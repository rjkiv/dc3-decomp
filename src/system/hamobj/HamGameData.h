#pragma once
#include "HamPlayerData.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class HamGameData : public Hmx::Object {
public:
    enum {
        kMaxPlayers = 2
    };
    // Hmx::Object
    virtual ~HamGameData();
    OBJ_CLASSNAME(HamGameData)
    OBJ_SET_TYPE(HamGameData)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    OBJ_MEM_OVERLOAD(0x3B)
    NEW_OBJ(HamGameData)
    static void Init();

    const DataNode *PlayerProp(int, Symbol) const;
    void SetPlayerProp(int, Symbol, const DataNode &);
    Symbol Venue() const;
    void Clear();
    HamPlayerData *Player(int) const;
    String GetPlayerName(int);
    void AssignSkeleton(int, int);
    void UnassignSkeletons();
    void AutoAssignSkeletons(const SkeletonUpdateData *);
    void SwapPlayerSidesByIDOnly();
    bool SidesSwapped();
    bool IsSkeletonPresent(int) const;
    int GetPlayerFromSkeleton(const Skeleton &) const;
    bool SetAssociatedPadNum(int, int);
    void SwapPlayerSides();
    void HandlePoseFound(int);
    void UpdateAssociatedPads();

    void SetPlayerSidesLocked(bool locked) {
        if (mPlayerSidesLocked != locked) {
            mPlayerSidesLocked = locked;
        }
    }
    void SetSong(Symbol song) { mSong = song; }
    Symbol GetSong() const { return mSong; }

protected:
    HamGameData();

    Symbol mVenue; // 0x2c
    Symbol mSong; // 0x30
    bool mPlayerSidesLocked; // 0x34
    bool unk35; // 0x35
    ObjVector<HamPlayerData *> mPlayers; // 0x38
    DateTime unk48; // 0x48
    int unk50; // 0x50
};

extern HamGameData *TheGameData;

int GetNumCrewCharacters(Symbol);
Symbol GetCrewCharacter(Symbol, int);
int GetNumCharacters(bool);
DataArray *GetCharacterEntry(int, bool);
DataArray *GetCharacterEntry(Symbol, bool);
const char *GetCharacterViseme(Symbol, bool);
int GetNumCharacterOutfits(Symbol, bool);
DataArray *GetCharacterOutfitEntry(Symbol, int, bool);
Symbol GetOutfitRemap(Symbol, bool);
Symbol GetCrewForCharacter(Symbol, bool);
Symbol GetAlternateCharacter(Symbol);
Symbol GetCharacterOutfit(Symbol, int, bool);
bool GetEntriesForOutfit(Symbol, DataArray **, DataArray **, bool);
DataArray *GetOutfitEntry(Symbol, bool);
int GetOutfitGender(Symbol, bool);
const char *GetOutfitModel(Symbol, bool);
Symbol GetCrewLookOutfit(Symbol, bool);
Symbol GetRivalOutfit(Symbol, bool);
Symbol GetBackupRivalOutfit(Symbol, bool);
Symbol GetOutfitCharacter(Symbol, bool);
