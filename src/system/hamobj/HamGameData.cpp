#include "hamobj/HamGameData.h"
#include "HamGameData.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/Symbol.h"

HamGameData *TheGameData;

namespace {
    DataArray *GetCrews() {
        static Symbol CREWS("CREWS");
        return DataGetMacro(CREWS);
    }

    DataArray *GetCharacters() {
        static Symbol CHARACTERS("CHARACTERS");
        return DataGetMacro(CHARACTERS);
    }
}

int GetNumCrewCharacters(Symbol crew) {
    static Symbol characters("characters");
    DataArray *pCrewArray = GetCrews();
    MILO_ASSERT(pCrewArray, 0x45);
    DataArray *pCrewData = pCrewArray->FindArray(crew);
    MILO_ASSERT(pCrewData, 0x48);
    DataArray *pCharacterArray = pCrewData->FindArray(characters);
    MILO_ASSERT(pCharacterArray, 0x4B);
    return pCharacterArray->Size() - 1;
}

Symbol GetCrewCharacter(Symbol crew, int index) {
    static Symbol characters("characters");
    DataArray *pCrewArray = GetCrews();
    MILO_ASSERT(pCrewArray, 0x53);
    DataArray *pCrewData = pCrewArray->FindArray(crew);
    MILO_ASSERT(pCrewData, 0x56);
    DataArray *pCharacterArray = pCrewData->FindArray(characters);
    MILO_ASSERT(pCharacterArray, 0x59);
    MILO_ASSERT(pCharacterArray->Size() > (index + 1), 0x5A);
    return pCharacterArray->Sym(index + 1);
}

int GetNumCharacters(bool fail) {
    DataArray *chars = GetCharacters();
    if (chars) {
        return chars->Size();
    } else if (fail) {
        MILO_FAIL("No characters defined\n");
    }
    return 0;
}

DataArray *GetCharacterEntry(int idx, bool fail) {
    DataArray *chars = GetCharacters();
    if (chars && idx < chars->Size()) {
        DataArray *entry = chars->Array(idx);
        if (entry)
            return entry;
    }
    if (fail) {
        MILO_FAIL("Character[%d] does not exist\n", idx);
    }
    return nullptr;
}

DataArray *GetCharacterEntry(Symbol theChar, bool fail) {
    DataArray *chars = GetCharacters();
    if (chars) {
        DataArray *entry = chars->FindArray(theChar, false);
        if (entry)
            return entry;
    }
    if (fail) {
        MILO_FAIL("Character does not exist: %s\n", theChar);
    }
    return nullptr;
}

const char *GetCharacterViseme(Symbol theChar, bool fail) {
    DataArray *entry = GetCharacterEntry(theChar, fail);
    if (entry) {
        static Symbol viseme("viseme");
        DataArray *visemeArr = entry->FindArray(viseme, false);
        if (visemeArr) {
            return visemeArr->Str(1);
        }
        if (fail) {
            MILO_FAIL("No viseme specified for character: %s\n", theChar);
        }
    }
    return "";
}

int GetNumCharacterOutfits(Symbol theChar, bool fail) {
    static Symbol outfits("outfits");
    DataArray *entry = GetCharacterEntry(theChar, fail);
    if (entry) {
        DataArray *outfitArr = entry->FindArray(outfits, false);
        if (outfitArr)
            return outfitArr->Size() - 1;
        if (fail) {
            MILO_FAIL("No outfits for character: %s\n", theChar.Str());
        }
    }
    return 0;
}

DataArray *GetCharacterOutfitEntry(Symbol theChar, int idx, bool fail) {
    static Symbol outfits("outfits");
    DataArray *entry = GetCharacterEntry(theChar, fail);
    if (entry) {
        DataArray *outfitArr = entry->FindArray(outfits, false);
        if (outfitArr && idx + 1 < outfitArr->Size()) {
            return outfitArr->Array(idx + 1);
        }
        if (fail) {
            MILO_FAIL("Character: %s has no outfits[%d]\n", theChar.Str(), idx);
        }
    }
    return nullptr;
}

Symbol GetOutfitRemap(Symbol outfit, bool fail) {
    static Symbol OUTFIT_REMAP("OUTFIT_REMAP");
    DataArray *outfitArr = DataGetMacro(OUTFIT_REMAP)->FindArray(outfit, false);
    if (outfitArr) {
        return outfitArr->Sym(1);
    } else if (fail) {
        MILO_FAIL("No remap entry for outfit: %s\n", outfit.Str());
    }
    return "";
}

Symbol GetCrewForCharacter(Symbol theChar, bool fail) {
    DataArray *pCrewsArray = GetCrews();
    MILO_ASSERT(pCrewsArray, 0x24);
    for (int i = 0; i < pCrewsArray->Size(); i++) {
        DataArray *pCrewArray = pCrewsArray->Array(i);
        MILO_ASSERT(pCrewArray, 0x2A);
        Symbol crew = pCrewArray->Sym(0);
        int num = GetNumCrewCharacters(crew);
        for (int j = 0; j < num; j++) {
            Symbol crewChar = GetCrewCharacter(crew, j);
            if (crewChar == theChar) {
                return crewChar;
            }
        }
    }
    if (fail) {
        MILO_FAIL("Character: %s has no crew\n", theChar.Str());
    }
    return gNullStr;
}

Symbol GetAlternateCharacter(Symbol theChar) {
    Symbol crew = GetCrewForCharacter(theChar, true);
    Symbol char0 = GetCrewCharacter(crew, 0);
    Symbol char1 = GetCrewCharacter(crew, 1);
    if (theChar == char0) {
        return char1;
    } else
        return char0;
}

Symbol GetCharacterOutfit(Symbol theChar, int idx, bool fail) {
    DataArray *entry = GetCharacterOutfitEntry(theChar, idx, fail);
    if (entry)
        return entry->Sym(0);
    else
        return gNullStr;
}

bool GetEntriesForOutfit(
    Symbol outfit, DataArray **charEntry, DataArray **outfitEntry, bool fail
) {
    static Symbol outfits("outfits");
    DataArray *chars = GetCharacters();
    if (chars) {
        for (int i = 0; i < chars->Size(); i++) {
            DataArray *arr = chars->Array(i);
            if (arr) {
                DataArray *arr2 = arr->FindArray(outfits, false);
                if (arr2) {
                    static Symbol OUTFIT_REMAP("OUTFIT_REMAP");
                    Symbol remap = GetOutfitRemap(outfit, fail);
                    if (!remap.Null()) {
                        DataArray *remapArr = arr2->FindArray(remap, false);
                        if (remapArr) {
                            if (charEntry)
                                *charEntry = arr;
                            if (outfitEntry)
                                *outfitEntry = remapArr;
                            return true;
                        }
                    }
                }
            }
        }
    }
    if (fail) {
        MILO_FAIL("Could not find outfit: %s", outfit);
    }
    if (charEntry)
        *charEntry = nullptr;
    if (outfitEntry)
        *outfitEntry = nullptr;
    return false;
}

DataArray *GetOutfitEntry(Symbol outfit, bool fail) {
    DataArray *entry;
    GetEntriesForOutfit(outfit, nullptr, &entry, fail);
    return entry;
}

int GetOutfitGender(Symbol outfit, bool fail) {
    static Symbol gender("gender");
    DataArray *array;
    GetEntriesForOutfit(outfit, &array, nullptr, fail);
    if (array) {
        DataArray *genderArr = array->FindArray(gender, false);
        if (genderArr)
            return genderArr->Int(1);
    }
    return 1;
}

const char *GetOutfitModel(Symbol outfit, bool fail) {
    DataArray *entry;
    GetEntriesForOutfit(outfit, nullptr, &entry, fail);
    if (entry) {
        static Symbol model("model");
        DataArray *modelArr = entry->FindArray(model, false);
        if (modelArr) {
            return modelArr->Str(1);
        }
        if (fail) {
            MILO_FAIL("No model specified for outfit: %s\n", outfit);
        }
    }
    return "";
}

Symbol GetCrewLookOutfit(Symbol outfit, bool fail) {
    String outfitStr(GetCharacterOutfit(outfit, 0, fail));
    outfitStr.replace(outfitStr.length() - 2, 2, "01");
    return GetOutfitRemap(outfitStr.c_str(), fail);
}

Symbol GetRivalOutfit(Symbol outfit, bool fail) {
    DataArray *pCharacterEntry;
    DataArray *pOutfitEntry;
    GetEntriesForOutfit(outfit, &pCharacterEntry, &pOutfitEntry, fail);
    MILO_ASSERT(pCharacterEntry, 0x1C9);
    MILO_ASSERT(pOutfitEntry, 0x1CA);
    static Symbol rival_outfit("rival_outfit");
    Symbol symRivalOutfit;
    pOutfitEntry->FindData(rival_outfit, symRivalOutfit, false);
    MILO_ASSERT(!symRivalOutfit.Null(), 0x1D0);
    return symRivalOutfit;
}

Symbol GetBackupRivalOutfit(Symbol outfit, bool fail) {
    DataArray *pCharacterEntry;
    DataArray *pOutfitEntry;
    GetEntriesForOutfit(outfit, &pCharacterEntry, &pOutfitEntry, fail);
    MILO_ASSERT(pCharacterEntry, 0x1DB);
    MILO_ASSERT(pOutfitEntry, 0x1DC);
    static Symbol backup_rival_outfit("backup_rival_outfit");
    Symbol symBackupRivalOutfit;
    pOutfitEntry->FindData(backup_rival_outfit, symBackupRivalOutfit, false);
    MILO_ASSERT(!symBackupRivalOutfit.Null(), 0x1E2);
    return symBackupRivalOutfit;
}

Symbol GetOutfitCharacter(Symbol outfit, bool fail) {
    DataArray *charEntry;
    GetEntriesForOutfit(outfit, &charEntry, nullptr, fail);
    if (charEntry)
        return charEntry->Sym(0);
    else
        return gNullStr;
}

HamGameData::HamGameData() : mPlayerSidesLocked(0), unk35(0), mPlayers(this), unk50(0) {
    GetDateAndTime(unk48);
    for (int i = 0; i < kMaxPlayers; i++) {
        mPlayers.push_back(new HamPlayerData(i));
    }
}

HamGameData::~HamGameData() { DeleteAll(mPlayers); }

BEGIN_HANDLERS(HamGameData)
    HANDLE_EXPR(getp, *PlayerProp(_msg->Int(2), _msg->Sym(3)))
    HANDLE_ACTION(setp, SetPlayerProp(_msg->Int(2), _msg->Sym(3), _msg->Node(4)))
    HANDLE_EXPR(max_players, kMaxPlayers)
    HANDLE_EXPR(player, mPlayers[_msg->Int(2)])
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamGameData)
    SYNC_PROP_SET(venue, Venue(), mVenue = _val.Sym())
    SYNC_PROP(song, mSong)
    SYNC_PROP_SET(
        player_sides_locked, mPlayerSidesLocked, SetPlayerSidesLocked(_val.Int())
    )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

DataNode OnGetCrewForCharacter(DataArray *a) {
    return GetCrewForCharacter(a->Sym(1), false);
}

DataNode OnGetOutfitGender(DataArray *a) { return GetOutfitGender(a->Sym(1), false); }

DataNode OnGetNumCharacterOutfits(DataArray *a) {
    return GetNumCharacterOutfits(a->Sym(1), true);
}

DataNode OnGetCharacterOutfit(DataArray *a) {
    return GetCharacterOutfit(a->Sym(1), a->Int(2), false);
}

DataNode GetPlayableOutfits(DataArray *a) {
    static Symbol outfits("outfits");
    static Symbol playable("playable");
    bool i2 = a->Int(1);
    DataArrayPtr ptr(new DataArray(0));
    DataArray *pCharacterArray = GetCharacters();
    MILO_ASSERT(pCharacterArray, 0x1A9);
    for (int i = 0; i < pCharacterArray->Size(); i++) {
        DataArray *pCharacterData = pCharacterArray->Array(i);
        MILO_ASSERT(pCharacterData, 0x1AE);
        bool isPlayable = true;
        pCharacterData->FindData(playable, isPlayable, false);
        if (isPlayable == i2) {
            DataArray *pOutfitArray = pCharacterData->FindArray(outfits, false);
            if (pOutfitArray) {
                for (int j = 1; j < pOutfitArray->Size(); j++) {
                    ptr->Insert(ptr->Size(), pOutfitArray->Array(j)->Sym(0));
                }
            }
        }
    }
    return ptr;
}

void HamGameData::Init() {
    REGISTER_OBJ_FACTORY(HamGameData);
    RELEASE(TheGameData);
    TheGameData = new HamGameData();
    TheGameData->SetName("gamedata", ObjectDir::Main());
    DataRegisterFunc("get_crew_for_character", OnGetCrewForCharacter);
    DataRegisterFunc("outfit_gender", OnGetOutfitGender);
    DataRegisterFunc("num_character_outfits", OnGetNumCharacterOutfits);
    DataRegisterFunc("character_outfit", OnGetCharacterOutfit);
    DataRegisterFunc("playable_outfits", GetPlayableOutfits);
}

void HamGameData::Clear() {
    mSong = gNullStr;
    mVenue = gNullStr;
}

Symbol HamGameData::Venue() const {
    static Symbol bid("bid");
    return unk35 ? bid : mVenue;
}

const DataNode *HamGameData::PlayerProp(int index, Symbol s2) const {
    MILO_ASSERT((0) <= (index) && (index) < (2), 0x224);
    return mPlayers[index]->Property(s2, true);
}

void HamGameData::SetPlayerProp(int index, Symbol s2, const DataNode &n) {
    MILO_ASSERT((0) <= (index) && (index) < (2), 0x22A);
    mPlayers[index]->SetProperty(s2, n);
}

HamPlayerData *HamGameData::Player(int index) const {
    MILO_ASSERT((0) <= (index) && (index) < (2), 0x230);
    return mPlayers[index];
}

String HamGameData::GetPlayerName(int player) {
    MILO_ASSERT((0) <= (player) && (player) < (2), 0x2B7);
    HamPlayerData *pPlayer = mPlayers[player];
    MILO_ASSERT(pPlayer, 0x2BA);
    return pPlayer->GetPlayerName();
}

void HamGameData::AssignSkeleton(int player, int id) {
    MILO_ASSERT((0) <= (player) && (player) < (2), 0x247);
    if (id != -1) {
        for (int i = 0; i < kMaxPlayers; i++) {
            if (i != player && id == mPlayers[i]->mSkeletonTrackingID) {
                MILO_NOTIFY(
                    "AssignSkeleton called for player %i with tracking_id %i, butplayer % i is already using it",
                    player,
                    id,
                    i
                );
            }
        }
    }
    mPlayers[player]->SetSkeletonTrackingID(id);
}

void HamGameData::UnassignSkeletons() {
    mPlayers[0]->SetSkeletonTrackingID(-1);
    mPlayers[1]->SetSkeletonTrackingID(-1);
}

void HamGameData::SwapPlayerSidesByIDOnly() {
    HamPlayerData *pPlayer0 = mPlayers[0];
    MILO_ASSERT(pPlayer0, 0x27D);
    HamPlayerData *pPlayer1 = mPlayers[1];
    MILO_ASSERT(pPlayer1, 0x27F);
    int id0 = pPlayer0->mSkeletonTrackingID;
    pPlayer0->SetSkeletonTrackingID(pPlayer1->mSkeletonTrackingID);
    pPlayer1->SetSkeletonTrackingID(id0);
}

bool HamGameData::SidesSwapped() {
    static Symbol side("side");
    int side0 = mPlayers[0]->mProvider->Property(side, true)->Int();
    return side0 != 1;
}

bool HamGameData::IsSkeletonPresent(int index) const {
    return TheGestureMgr->InControllerMode() || Player(index)->mSkeletonTrackingID > 0;
}

int HamGameData::GetPlayerFromSkeleton(const Skeleton &skeleton) const {
    for (int i = 0; i < kMaxPlayers; i++) {
        HamPlayerData *player = TheGameData->Player(i);
        if (player->mSkeletonTrackingID == skeleton.TrackingID())
            return i;
    }
    return -1;
}

bool HamGameData::SetAssociatedPadNum(int player, int padnum) {
    MILO_ASSERT((0) <= (player) && (player) < (2), 0x2CD);
    HamPlayerData *pPlayer = mPlayers[player];
    MILO_ASSERT(pPlayer, 0x2D0);
    if (padnum >= 0 && ThePlatformMgr.IsSignedIn(padnum)) {
        HamPlayerData *playerData = mPlayers[player];
        if (playerData->mPadNum == padnum) {
            playerData->SetAssociatedPadNum(-1, gNullStr);
        }
        return playerData->SetAssociatedPadNum(padnum, ThePlatformMgr.GetName(padnum));
    } else {
        return pPlayer->SetAssociatedPadNum(-1, gNullStr);
    }
}

void HamGameData::SwapPlayerSides() {
    static Symbol side("side");
    int side0 = mPlayers[0]->mProvider->Property(side, true)->Int();
    int side1 = mPlayers[1]->mProvider->Property(side, true)->Int();
    mPlayers[0]->mProvider->SetProperty(side, side1);
    mPlayers[1]->mProvider->SetProperty(side, side0);
    for (int i = 0; i < kMaxPlayers; i++) {
        if (0.5f < Player(i)->TrackingAgeSeconds()) {
            static Message side_moved("side_moved");
            Player(i)->mProvider->Export(side_moved, true);
        }
    }
}

void HamGameData::HandlePoseFound(int i1) {
    static Message poseFound("pose_found", 0);
    poseFound[0] = i1;
    Export(poseFound, true);
}

void HamGameData::UpdateAssociatedPads() {
    for (int i = 0; i < kMaxPlayers; i++) {
        HamPlayerData *pPlayer = mPlayers[i];
        MILO_ASSERT(pPlayer, 0x2C4);
        SetAssociatedPadNum(i, pPlayer->mPadNum);
    }
}
