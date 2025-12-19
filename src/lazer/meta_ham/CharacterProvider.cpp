#include "meta_ham/CharacterProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/ProfileMgr.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "ui/UIListMesh.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

CharacterProvider::CharacterProvider() : unk30(0), unk40(0) {}

bool CharacterProvider::IsCharacterAvailable(Symbol s) const {
    static Symbol character_default("character_default");
    if (s == character_default)
        return true;
    else {
        int index = unk30 == 0;
        HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
        MILO_ASSERT(pOtherPlayerData, 0x150);
        if (TheGameMode->InMode("dance_battle", true)) {
            Symbol crewForChar = GetCrewForCharacter(s);
            if (pOtherPlayerData->Crew() == crewForChar)
                return false;
        } else if (pOtherPlayerData->Char() == s
                   && 0 < TheGameData->Player(index)->GetSkeletonTrackingID()) {
            return false;
        }
    }
    return true;
}

Symbol CharacterProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mCharacters.size(), 0x138);
    return mCharacters[idx];
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

RndMat *CharacterProvider::GetMatForCharacter(Symbol s) const {
    RndMat *mat = nullptr;
    static Symbol character_default("character_default");
    if (s != character_default && unk40 != 0) {
        if (TheProfileMgr.IsContentUnlocked(s) && IsCharacterAvailable(s)) {
            String colorName = GetColorName();
            String fileName = MakeString("portrait_%s_%s.mat", colorName, s);
            mat = unk40->Find<RndMat>(fileName.c_str(), false);
        }
    }
    return mat;
}

bool CharacterProvider::CanSelect(int data) const {
    MILO_ASSERT_RANGE(data, 0, NumData(), 0x126);
    Symbol dataSym = DataSymbol(data);
    if (!TheProfileMgr.IsContentUnlocked(dataSym)) {
        return false;
    } else {
        return false + IsCharacterAvailable(dataSym);
    }
}
