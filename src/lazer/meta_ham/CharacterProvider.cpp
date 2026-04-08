#include "meta_ham/CharacterProvider.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Mat.h"
#include "ui/PanelDir.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"
#include <cstdio>

CharacterProvider::CharacterProvider() : mPlayer(0), mPanelDir(0) {}

void CharacterProvider::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT_RANGE(data, 0, mCharacters.size(), 0x93);
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x96);
    Symbol charSym = mCharacters[data];
    if (slot->Matches("character")) {
        static Symbol player_present("player_present");
        int otherPlayer = !mPlayer;
        if (TheGameData->Player(mPlayer)->Provider()->Property(player_present)->Int() != 1
            && TheGameData->Player(otherPlayer)->Provider()->Property(player_present)->Int()
                != 0) {
            char buffer[32];
            buffer[0] = 0;
            memset(&buffer[1], 0, 31);
            sprintf(buffer, "%s%s", charSym.Str(), "_title");
            label->SetTextToken(buffer);
        } else {
            label->SetTextToken(charSym);
        }
    } else if (slot->Matches("new")) {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            app_label->SetNew(profile->IsContentNew(charSym));
        } else {
            app_label->SetTextToken(gNullStr);
        }
    } else if (slot->Matches("lock")) {
        if (!TheProfileMgr.IsContentUnlocked(charSym)) {
            app_label->SetIcon('B');
        } else {
            app_label->SetTextToken(gNullStr);
        }
    } else if (slot->Matches("taken")) {
        if (!IsCharacterAvailable(charSym)) {
            label->SetIcon('i');
        } else {
            app_label->SetTextToken(gNullStr);
        }
    }
}

RndMat *CharacterProvider::Mat(int, int data, UIListMesh *uiMesh) const {
    MILO_ASSERT_RANGE(data, 0, NumData(), 0xd1);
    Symbol dataSym = DataSymbol(data);
    if (uiMesh->Matches("icon")) {
        return GetMatForCharacter(dataSym);
    } else {
        return uiMesh->DefaultMat();
    }
}

Symbol CharacterProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mCharacters.size(), 0x138);
    return mCharacters[idx];
}

bool CharacterProvider::CanSelect(int data) const {
    MILO_ASSERT_RANGE(data, 0, NumData(), 0x126);
    Symbol dataSym = DataSymbol(data);
    if (!TheProfileMgr.IsContentUnlocked(dataSym)) {
        return false;
    } else {
        return IsCharacterAvailable(dataSym) != false;
    }
}

int CharacterProvider::DataIndex(Symbol s) const {
    for (int i = 0; i < mCharacters.size(); i++) {
        if (mCharacters[i] == s) {
            return i;
        }
    }
    return -1;
}

bool CharacterProvider::IsCharacterAvailable(Symbol charSym) const {
    static Symbol character_default("character_default");
    if (charSym == character_default)
        return true;
    else {
        int index = !mPlayer;
        HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
        MILO_ASSERT(pOtherPlayerData, 0x150);
        if (TheGameMode->InMode("dance_battle", true)) {
            Symbol crewForChar = GetCrewForCharacter(charSym);
            if (pOtherPlayerData->Crew() == crewForChar)
                return false;
        } else if (pOtherPlayerData->Char() == charSym
                   && 0 < TheGameData->Player(index)->GetSkeletonTrackingID()) {
            return false;
        }
    }
    return true;
}

RndMat *CharacterProvider::GetMatForCharacter(Symbol charSym) const {
    RndMat *mat = nullptr;
    static Symbol character_default("character_default");
    if (charSym != character_default && mPanelDir != 0) {
        if (TheProfileMgr.IsContentUnlocked(charSym) && IsCharacterAvailable(charSym)) {
            String colorName = GetColorName();
            String fileName =
                MakeString("portrait_%s_%s.mat", colorName.c_str(), charSym.Str());
            mat = mPanelDir->Find<RndMat>(fileName.c_str(), false);
        }
    }
    return mat;
}

String CharacterProvider::GetColorName() const {
    HamPlayerData *pPlayerData = TheGameData->Player(mPlayer);
    MILO_ASSERT(pPlayerData, 0xE5);
    Hmx::Object *pPlayerProvider = pPlayerData->Provider();
    MILO_ASSERT(pPlayerProvider, 0xE8);
    static Symbol player_present("player_present");
    const DataNode *pPlayerPresent = pPlayerProvider->Property(player_present);
    MILO_ASSERT(pPlayerPresent, 0xEC);
    if (pPlayerPresent->Int() != 0) {
        if (mPlayer == 0) {
            return "pink";
        } else {
            return "blue";
        }
    } else {
        return "purple";
    }
}

Symbol CharacterProvider::GetRandomAvailableCharacter() const {
    static Symbol character_default("character_default");
    std::vector<Symbol> vCharacters;
    int num = NumData();
    for (int i = 0; i < num; i++) {
        Symbol dataSym = DataSymbol(i);
        if (CanSelect(i) && dataSym != character_default) {
            vCharacters.push_back(dataSym);
        }
    }
    MILO_ASSERT(vCharacters.size(), 0x36);
    return vCharacters[RandomInt(0, vCharacters.size())];
}

void CharacterProvider::UpdateList() {
    HamPlayerData *pPlayerData = TheGameData->Player(mPlayer);
    MILO_ASSERT(pPlayerData, 0x44);
    Symbol crew = pPlayerData->Crew();
    std::set<Symbol> symSet;
    DataArray *charCfg = SystemConfig()->FindArray("characters", false);
    if (charCfg) {
        for (int i = 1; i < charCfg->Size(); i++) {
            DataArray *pCharacterEntry = charCfg->Array(i);
            MILO_ASSERT(pCharacterEntry, 0x4F);
            static Symbol hidden("hidden");
            bool isHidden = false;
            pCharacterEntry->FindData(hidden, isHidden, false);
            if (isHidden) {
                Symbol sym = pCharacterEntry->Sym(0);
                symSet.insert(sym);
            }
        }
    }
    mCharacters.clear();
    if (!TheGameMode->InMode("dance_battle")) {
        static Symbol character_default("character_default");
        mCharacters.push_back(character_default);
    }
    DataArray *crewCfg = SystemConfig()->FindArray("selectable_crews", false);
    if (crewCfg) {
        for (int i = 1; i < crewCfg->Size(); i++) {
            Symbol curCrew = crewCfg->Sym(i);
            if (TheGameMode->InMode("dance_battle")) {
            crewCmp:
                if (curCrew == crew)
                    goto loop;
            } else {
                if (TheHamProvider->Property("is_in_party_mode")->Int()) {
                    goto crewCmp;
                }
            loop:
                int numCrewChars = GetNumCrewCharacters(curCrew);
                for (int j = 0; j < numCrewChars; j++) {
                    Symbol curCrewChar = GetCrewCharacter(curCrew, j);
                    if (TheProfileMgr.IsContentUnlocked(curCrewChar)) {
                        mCharacters.push_back(curCrewChar);
                    } else if (symSet.find(curCrewChar) == symSet.end()) {
                        mCharacters.push_back(curCrewChar);
                    }
                }
            }
        }
    }
}

void CharacterProvider::SetPanelDir(PanelDir *dir) { mPanelDir = dir; }
