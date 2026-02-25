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

    void SetPlayerSidesLocked(bool locked);
    bool GetPlayerSidesLocked() { return mPlayerSidesLocked; }
    void SetSong(Symbol song) { mSong = song; }
    Symbol GetSong() const { return mSong; }
    void SetVenue(Symbol venue) { mVenue = venue; }
    void SetInTimeyWimey(bool set) { unk35 = set; }

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

// For these, see the CHARACTERS and CREWS definitions in config/macros.dta.

/** How many characters are in the given crew? (I've never seen this not be 2) */
int GetNumCrewCharacters(Symbol crew);
/** Get the character associated with the given crew at the given index. */
Symbol GetCrewCharacter(Symbol crew, int index);
/** How many characters are defined in the game? */
int GetNumCharacters(bool fail = true);
/** Get the entry of the character at the supplied index. */
DataArray *GetCharacterEntry(int idx, bool fail = true);
/** Get the entry of the character based on the character's symbol. */
DataArray *GetCharacterEntry(Symbol charSym, bool fail = true);
/** Get the viseme of the character based on the character's symbol. */
const char *GetCharacterViseme(Symbol charSym, bool fail = true);
/** Get the outfit data of the character based on the character's symbol. */
int GetNumCharacterOutfits(Symbol charSym, bool fail = true);
/** Get the specific outfit data for the character
    based on the character's symbol and supplied outfit index.
    (i.e. passing in charSym="emilia" and idx=0 would return (emilia04 (...)))
*/
DataArray *GetCharacterOutfitEntry(Symbol charSym, int index, bool fail = true);
/** Given a base outfit symbol, get the remapped outfit symbol. */
Symbol GetOutfitRemap(Symbol outfit, bool fail = true);
/** Given a character, get their associated crew. */
Symbol GetCrewForCharacter(Symbol charSym, bool fail = true);
/** Given a character, get the alternate character in the crew.
    (i.e. if you pass in emilia, you should get bodie.)
*/
Symbol GetAlternateCharacter(Symbol charSym);
/** Get the specific outfit symbol for the character
    based on the character's symbol and supplied outfit index.
    (i.e. passing in charSym="emilia" and idx=0 would return emilia04)
*/
Symbol GetCharacterOutfit(Symbol charSym, int index, bool fail = true);
/** Given an outfit symbol, get the outfit's corresponding character entry and remap
 * outfit entry.
 * @param [in] outfit The outfit symbol (i.e. emilia04)
 * @param [out] charEntry The character data array associated with the outfit.
 * @param [out] outfitEntry The outfit data array associated with the outfit.
 * @param [in] fail If true, fail if the outfit was not found.
 * @returns True if we found a character and outfit entry, false if not.
 */
bool GetEntriesForOutfit(
    Symbol outfit, DataArray **charEntry, DataArray **outfitEntry, bool fail = true
);
/** Given an outfit symbol, get the corresponding outfit entry. */
DataArray *GetOutfitEntry(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the corresponding outfit's gender. */
int GetOutfitGender(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the corresponding outfit's model. */
const char *GetOutfitModel(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the character's crew outfit. */
Symbol GetCrewLookOutfit(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the rival character's outfit. */
Symbol GetRivalOutfit(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the rival character's backup outfit. */
Symbol GetBackupRivalOutfit(Symbol outfit, bool fail = true);
/** Given an outfit symbol, get the symbol of the associated character. */
Symbol GetOutfitCharacter(Symbol outfit, bool fail = true);
