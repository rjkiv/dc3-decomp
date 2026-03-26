#include "meta_ham/MultiUserGesturePanel.h"
#include "HamPanel.h"
#include "HamSongMetadata.h"
#include "HamSongMgr.h"
#include "MultiUserGesturePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/CharacterProvider.h"
#include "meta_ham/CrewProvider.h"
#include "meta_ham/DifficultyProvider.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/OutfitProvider.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SkeletonChooser.h"
#include "meta_ham/TexLoadPanel.h"
#include "meta_ham/VenueProvider.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "ui/UI.h"
#include "ui/UIComponent.h"
#include "ui/UIPicture.h"
#include "utl/FilePath.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"
#include <cstring>

MultiUserGesturePanel::MultiUserGesturePanel() {
    for (int i = 0; i < 2; i++) {
        unk54[i] = nullptr;
        unk5c[i] = nullptr;
        mCharacterProviders[i].SetPlayer(i);
        mVenueProviders[i].SetPlayer(i);
        mCrewProviders[i].SetPlayer(i);
        mOutfitProviders[i].SetPlayer(i);
        mDifficultyProviders[i].SetPlayer(i);
    }
}

void MultiUserGesturePanel::Poll() {
    if (!TheUI->InTransition()) {
        for (int i = 0; i < 2; i++)
            UpdateNavLists(i);
        UpdateProviderPlayerIndices();
    }
    TexLoadPanel::Poll();
}

void MultiUserGesturePanel::Unload() {
    TexLoadPanel::Unload();
    for (int i = 0; i < 2; i++) {
        mCharacterProviders[i].SetPanelDir(nullptr);
        mCrewProviders[i].SetPanelDir(nullptr);
    }
}

void MultiUserGesturePanel::FinishLoad() {
    TexLoadPanel::FinishLoad();
    for (int i = 0; i < 2; i++) {
        mCharacterProviders[i].SetPanelDir(LoadedDir());
        mCrewProviders[i].SetPanelDir(LoadedDir());
    }
}

bool MultiUserGesturePanel::Exiting() const { return HamPanel::Exiting(); }

void MultiUserGesturePanel::UpdateProviders() {
    for (int i = 0; i < 2; i++) {
        mCharacterProviders[i].UpdateList();
        mVenueProviders[i].UpdateList();
        mCrewProviders[i].UpdateList();
        mOutfitProviders[i].UpdateList();
    }
}

CharacterProvider const *MultiUserGesturePanel::GetCharProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0xcf);
    return &mCharacterProviders[index];
}

VenueProvider const *MultiUserGesturePanel::GetVenueProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0xd6);
    return &mVenueProviders[index];
}

CrewProvider const *MultiUserGesturePanel::GetCrewProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0xdd);
    return &mCrewProviders[index];
}

OutfitProvider const *MultiUserGesturePanel::GetOutfitProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0xe4);
    return &mOutfitProviders[index];
}

DifficultyProvider *MultiUserGesturePanel::GetDifficultyProvider(int index) {
    MILO_ASSERT_RANGE(index, 0, 2, 0xeb);
    return &mDifficultyProviders[index];
}

Symbol MultiUserGesturePanel::GetOutfit(int i1, int i2) const {
    const OutfitProvider *pProvider = GetOutfitProvider(i2);
    MILO_ASSERT(pProvider, 0xf9);
    return pProvider->DataSymbol(i1);
}

Symbol MultiUserGesturePanel::GetCharacter(int i1, int i2) {
    const CharacterProvider *pProvider = GetCharProvider(i2);
    MILO_ASSERT(pProvider, 0x2a1);
    return pProvider->DataSymbol(i1);
}

int MultiUserGesturePanel::GetCharacterIndex(int idx) const {
    const CharacterProvider *pProvider = GetCharProvider(idx);
    MILO_ASSERT(pProvider, 0x154);
    HamPlayerData *pPlayerData = TheGameData->Player(GetPlayerIndex(idx));
    return pProvider->DataIndex(pPlayerData->Char());
}

int MultiUserGesturePanel::GetOutfitIndex(int idx) const {
    const OutfitProvider *pProvider = GetOutfitProvider(idx);
    MILO_ASSERT(pProvider, 0x15f);
    HamPlayerData *pPlayerData = TheGameData->Player(GetPlayerIndex(idx));
    return pProvider->DataIndex(pPlayerData->Outfit());
}

int MultiUserGesturePanel::GetCrewIndex(int idx) const {
    const CrewProvider *pProvider = GetCrewProvider(idx);
    MILO_ASSERT(pProvider, 0x16a);
    HamPlayerData *pPlayerData = TheGameData->Player(GetPlayerIndex(idx));
    return pProvider->DataIndex(pPlayerData->Crew());
}

int MultiUserGesturePanel::GetVenueIndex(int idx, Symbol s) const {
    const VenueProvider *pProvider = GetVenueProvider(idx);
    MILO_ASSERT(pProvider, 0x175);
    return pProvider->DataIndex(s);
}

bool MultiUserGesturePanel::IsCrewAvailable(Symbol crew, int idx) {
    const CrewProvider *pProvider = GetCrewProvider(idx);
    MILO_ASSERT(pProvider, 0x2a9);
    return pProvider->IsCrewAvailable(crew);
}

bool MultiUserGesturePanel::IsCharacterAvailable(Symbol character, int idx) {
    const CharacterProvider *pProvider = GetCharProvider(idx);
    MILO_ASSERT(pProvider, 0x2b1);
    return pProvider->IsCharacterAvailable(character);
}

void MultiUserGesturePanel::SetOutfit(Symbol outfit, int idx) {
    HamPlayerData *pPlayerData = TheGameData->Player(idx);
    MILO_ASSERT(pPlayerData, 0x101);
    pPlayerData->SetOutfit(outfit);
    if (!TheGameMode->InMode("campaign", true))
        pPlayerData->SetPreferredOutfit(outfit);
}

void MultiUserGesturePanel::SetDefaultCharacter(int idx) {
    int index = GetPlayerIndex(idx);
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0x248);
    MetaPerformer *pPerformer = MetaPerformer::Current();
    MILO_ASSERT(pPerformer, 0x24b);
    pPerformer->SetDefaultSongCharacter(index);
    if (!TheGameMode->InMode("dance_battle", true)) {
        if (!TheGameMode->InMode("campaign", true)) {
            pPlayerData->SetUnk48(gNullStr);
            pPlayerData->SetPreferredOutfit(gNullStr);
        }
    }
}

void MultiUserGesturePanel::SetRandomOutfit(int idx) {
    int index = GetPlayerIndex(idx);
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0x295);
    const OutfitProvider *pOutfitProvider = GetOutfitProvider(idx);
    MILO_ASSERT(pOutfitProvider, 0x298);
    Symbol randOutfit = pOutfitProvider->GetRandomAvailableOutfit();
    pPlayerData->SetOutfit(randOutfit);
}

void MultiUserGesturePanel::SetRandomCrew(int idx) {
    static Symbol random_crew("random_crew");
    int index = GetPlayerIndex(idx);
    int otherindex = index == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0x22e);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0x230);
    const CrewProvider *pCrewProvider = GetCrewProvider(idx);
    MILO_ASSERT(pCrewProvider, 0x233);
    Symbol symRandomCrew = pCrewProvider->GetRandomAvailableCrew();
    MILO_ASSERT(symRandomCrew != gNullStr, 0x236);
    pPlayerData->SetCrew(symRandomCrew);
    const CharacterProvider *pCharacterProvider = GetCharProvider(idx);
    MILO_ASSERT(pCharacterProvider, 0x23b);
    const_cast<CharacterProvider *>(pCharacterProvider)->UpdateList();
    SetRandomCharacter(idx);
}

void MultiUserGesturePanel::SetCrew(Symbol crew, int idx) {
    int index = GetPlayerIndex(idx);
    int otherindex = index == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0x131);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0x133);
    if (crew != Symbol("") && pOtherPlayerData->Crew() == crew) {
        pOtherPlayerData->SetCrew(pPlayerData->Crew());
        pOtherPlayerData->SetCharacter(pPlayerData->Char());
        pOtherPlayerData->SetOutfit(pPlayerData->Outfit());
        RefreshUI();
    }
    MetaPerformer *performer = MetaPerformer::Current();
    MILO_ASSERT(performer, 0x143);
    pPlayerData->SetCrew(crew);
    if (crew != Symbol("")) {
        const CharacterProvider *pCharacterProvider = GetCharProvider(idx);
        MILO_ASSERT(pCharacterProvider, 0x14a);
        const_cast<CharacterProvider *>(pCharacterProvider)->UpdateList();
        SetRandomCharacter(idx);
    }
}

void MultiUserGesturePanel::RefreshUI() {
    static Message cRefreshUIMsg("refresh_ui");
    TheUI->Handle(cRefreshUIMsg, false);
}

int MultiUserGesturePanel::GetPlayerIndex(int idx) const {
    SkeletonChooser *pSkeletonChooser = TheHamUI.GetShellInput()->GetSkeletonChooser();
    MILO_ASSERT(pSkeletonChooser, 0x68);
    SkeletonSide playerSide = pSkeletonChooser->GetPlayerSide(0);
    const DataNode *prop = TheHamProvider->Property("is_in_party_mode", true);
    if (prop->Int() != 0) {
        return idx;
    }
    if (idx == 0) {
        return playerSide != kSkeletonRight;
    } else
        return playerSide == kSkeletonRight;
}

void MultiUserGesturePanel::SetCharacter(Symbol s, int idx) {
    int otherindex = idx == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(idx);
    MILO_ASSERT(pPlayerData, 0x110);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0x112);
    MetaPerformer *performer = MetaPerformer::Current();
    MILO_ASSERT(performer, 0x115);
    Symbol crewForChar = GetCrewForCharacter(s);
    pPlayerData->SetCharacter(s);
    pPlayerData->SetCrew(crewForChar);
    pPlayerData->SetOutfit(GetCharacterOutfit(s, false));
    if (!TheGameMode->InMode("campaign", true)) {
        pPlayerData->SetUnk48(s);
        pPlayerData->SetPreferredOutfit(GetCharacterOutfit(s, false));
    }
    if (pOtherPlayerData->Char() == s) {
        performer->SetDefaultSongCharacter(otherindex);
        RefreshUI();
    }
}

void MultiUserGesturePanel::SetRandomCharacter(int idx) {
    int index = GetPlayerIndex(idx);
    int otherindex = index == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0x25d);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0x25f);
    const CharacterProvider *pCharProvider = GetCharProvider(idx);
    MILO_ASSERT(pCharProvider, 0x262);
    Symbol symRandomCharacter = pCharProvider->GetRandomAvailableCharacter();
    MILO_ASSERT(symRandomCharacter != gNullStr, 0x265);
    Symbol crewForChar = GetCrewForCharacter(symRandomCharacter);
    pPlayerData->SetCharacter(symRandomCharacter);
    pPlayerData->SetCrew(crewForChar);
    if (!TheGameMode->InMode("dance_battle", true)) {
        if (!TheGameMode->InMode("campaign", true)) {
            pPlayerData->SetUnk48(gNullStr);
            pPlayerData->SetPreferredOutfit(gNullStr);
        }
    }

    if (TheGameMode->InMode("dance_battle", true)) {
        pPlayerData->SetOutfit(GetCharacterOutfit(symRandomCharacter, false));
    } else {
        const OutfitProvider *pOutfitProvider = GetOutfitProvider(idx);
        MILO_ASSERT(pOutfitProvider, 0x28a);
        const_cast<OutfitProvider *>(pOutfitProvider)->UpdateList();
        SetRandomOutfit(idx);
    }
}

void MultiUserGesturePanel::DropPlayerOnSide(int idx) {
    int index = GetPlayerIndex(idx);
    SkeletonChooser *pSkeletonChooser = TheHamUI.GetShellInput()->GetSkeletonChooser();
    MILO_ASSERT(pSkeletonChooser, 0x3d);
    pSkeletonChooser->ClearPlayerSkeletonID(index);
}

void MultiUserGesturePanel::UpdateCrewPic(
    UIPicture *i_pPic, int i_iSide, int i_iPlayerIndex, Symbol s
) {
    MILO_ASSERT(i_pPic, 0x1e9);
    MILO_ASSERT_RANGE(i_iPlayerIndex, 0, 2, 0x1ea);
    MILO_ASSERT_RANGE(i_iSide, 0, 2, 0x1eb);
    const CrewProvider *pProvider = GetCrewProvider(i_iSide);
    MILO_ASSERT(pProvider, 0x1ee);
    String str;
    if (!TheProfileMgr.IsContentUnlocked(s)) {
        str = MakeString("%s_char_locked_keep.png", s.Str());
    } else if (pProvider->IsCrewAvailable(s)) {
        str = MakeString("%s_char_keep.png", s.Str());
    }
    FilePath fp = FilePath("ui/image/crew/", str.c_str());
    i_pPic->SetTex(fp);
}

void MultiUserGesturePanel::UpdateNavLists(int player) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x9d);
    SkeletonChooser *skeletonChooser = TheHamUI.GetShellInput()->GetSkeletonChooser();
    MILO_ASSERT(skeletonChooser, 0xa0);
    HamPlayerData *pPlayerData = TheGameData->Player(player);
    int trackingID = pPlayerData->GetSkeletonTrackingID();
    SkeletonSide side = skeletonChooser->GetPlayerSide(player);
    int idx = side != kSkeletonRight;
    if (unk54[idx]) {
        unk54[idx]->SetTrackingID(trackingID);
        if ((trackingID <= 0 && !TheGestureMgr->InControllerMode())
            || TheHamUI.GetOverlayPanel()) {
            unk54[idx]->SetTrackingID(0);
            unk54[idx]->Disengage();
        }
    }
    if (unk5c[idx]) {
        unk5c[idx]->SetShowing(true);
        unk5c[idx]->SetTrackingID(trackingID);
        if ((trackingID <= 0 && !TheGestureMgr->InControllerMode())
            || TheHamUI.GetOverlayPanel()) {
            unk5c[idx]->SetTrackingID(0);
            unk5c[idx]->Disengage();
            if (TheHamUI.GetOverlayPanel()) {
                unk5c[idx]->SetShowing(false);
            }
        }
    }
}

bool MultiUserGesturePanel::HasNavList() const {
    for (int i = 0; i < 2; i++) {
        if (unk54[i]) {
            return true;
        }
    }
    return false;
}

void MultiUserGesturePanel::UpdateProviderPlayerIndices() {
    for (int i = 0; i < 2; i++) {
        int index = GetPlayerIndex(i);
        mCharacterProviders[i].SetPlayer(index);
        mVenueProviders[i].SetPlayer(index);
        mDifficultyProviders[i].SetPlayer(index);
        mCrewProviders[i].SetPlayer(index);
        mOutfitProviders[i].SetPlayer(index);
    }
}

Symbol MultiUserGesturePanel::GetVoiceCommandOutfitTag(int i, Symbol s) {
    HamPlayerData *pPlayerData = TheGameData->Player(i);
    Symbol charSym = pPlayerData->Char();
    Symbol retval;
    static Symbol screen_name("screen_name");
    int charOutfits = GetNumCharacterOutfits(charSym, false);
    for (int i = 0; i < charOutfits; i++) {
        DataArray *pCharacterOutfitArray = GetCharacterOutfitEntry(charSym, i);
        DataArray *pScreenArray = pCharacterOutfitArray->FindArray(screen_name, false);
        if (pScreenArray) {
            if (pScreenArray->Sym(1) == s) {
                retval = pCharacterOutfitArray->Sym(0);
                break;
            }
        }
    }
    return retval;
}

void MultiUserGesturePanel::Enter() {
    UpdateProviderPlayerIndices();
    HamPanel::Enter();
    for (int i = 0; i < 2; i++) {
        unk54[i] =
            DataDir()->Find<HamNavList>(MakeString("right_hand_p%d.hnl", i + 1), false);
        unk5c[i] =
            DataDir()->Find<HamNavList>(MakeString("left_hand_p%d.hnl", i + 1), false);
    }
    UpdateProviders();
}

void MultiUserGesturePanel::UpdateVenueMesh(
    RndMesh *i_pMesh, int i_iSide, int i_iPlayerIndex, Symbol s1, Symbol s2
) {
    MILO_ASSERT(i_pMesh, 0x208);
    MILO_ASSERT_RANGE(i_iPlayerIndex, 0, 2, 0x209);
    MILO_ASSERT_RANGE(i_iSide, 0, 2, 0x20a);
    const CrewProvider *pProvider = GetCrewProvider(i_iSide);
    MILO_ASSERT(pProvider, 0x20d);
    String lockedVenueStr;
    String venueStr = MakeString("venue_p%i.mat", i_iSide + 1);
    RndMat *pMat = mDir->Find<RndMat>(venueStr.c_str(), false);
    MILO_ASSERT(pMat, 0x212);

    if (!TheProfileMgr.IsContentUnlocked(s1)) {
        lockedVenueStr = MakeString("venue_%s_locked.tex", s2.Str());
    } else {
        lockedVenueStr = MakeString("venue_%s.tex", s2.Str());
    }
    RndTex *pTex = mDir->Find<RndTex>(lockedVenueStr.c_str(), false);
    if (pTex) {
        pMat->SetDiffuseTex(pTex);
        i_pMesh->SetMat(pMat);
    }
}

void MultiUserGesturePanel::UpdateCharPic(
    UIPicture *i_pPic, int i_iSide, int i_iPlayerIndex, Symbol charSym, Symbol outfitSym
) {
    MILO_ASSERT(i_pPic, 0x18e);
    MILO_ASSERT_RANGE(i_iPlayerIndex, 0, 2, 0x18f);
    MILO_ASSERT_RANGE(i_iSide, 0, 2, 0x190);
    static Symbol character_default("character_default");
    HamPlayerData *pPlayerData = TheGameData->Player(i_iPlayerIndex);
    if (charSym == character_default) {
        MetaPerformer *pPerformer = MetaPerformer::Current();
        Symbol primaryCrew;
        Symbol primaryChar;
        Symbol primaryOutfit;
        Symbol secondaryCrew;
        Symbol secondaryChar;
        Symbol secondaryOutfit;
        int songID = TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong());
        const HamSongMetadata *pSongData = TheHamSongMgr.Data(songID);
        MILO_ASSERT(pSongData, 0x19d);

        bool check = false;
        if (TheGameMode->InMode("dance_battle") || TheGameMode->InMode("strike_a_pose")) {
            check = true;
        }
        HamPlayerData *pPrimary;
        HamPlayerData *pSecondary;
        pPerformer->CalcCharacters(
            pSongData,
            check,
            (PlayerFlag)i_iPlayerIndex,
            pPrimary,
            primaryCrew,
            primaryChar,
            primaryOutfit,
            pSecondary,
            secondaryCrew,
            secondaryChar,
            secondaryOutfit
        );

        if (pPlayerData == pPrimary) {
            charSym = primaryChar;
            outfitSym = primaryOutfit;
        } else {
            MILO_ASSERT(pPlayerData == pSecondary, 0x1a8);
            charSym = secondaryChar;
            outfitSym = secondaryOutfit;
        }
    } else if (charSym == "") {
        MILO_ASSERT(pPlayerData, 0x1af);
        charSym = pPlayerData->Char();
        outfitSym = pPlayerData->Outfit();
    }

    if (charSym == "") {
        return;
    }

    const CharacterProvider *pProvider = GetCharProvider(i_iSide);
    MILO_ASSERT(pProvider, 0x1ba);

    String str;
    bool contentLocked = !TheProfileMgr.IsContentUnlocked(charSym)
        || !TheProfileMgr.IsContentUnlocked(outfitSym);
    if (contentLocked) {
        static Symbol is_in_party_mode("is_in_party_mode");
        int propInt = TheHamProvider->Property(is_in_party_mode)->Int();
        if (propInt != 0) {
            contentLocked = contentLocked & (strstr(outfitSym.Str(), "01") != 0);
        }
    }

    if (contentLocked && !TheGameMode->InMode("campaign")) {
        str = MakeString("%s_locked_keep.png", outfitSym);
    } else if (pProvider->IsCharacterAvailable(charSym)) {
        str = MakeString("%s_keep.png", outfitSym);
    }
    FilePath fp = FilePath("ui/image/char/", str.c_str());
    i_pPic->SetTex(fp);
    i_pPic->GetMesh()->SetShowing(true);
}

DataNode MultiUserGesturePanel::OnMsg(const ButtonDownMsg &msg) {
    static Symbol side("side");
    if (msg.GetButton() == kPad_LStickLeft || msg.GetButton() == kPad_DLeft) {
        if (TheUI->FocusPanel() && TheUI->FocusComponent() == unk54[0]) {
            TheUI->FocusPanel()->SetFocusComponent(unk54[1]);
            PropertyEventProvider *multiUserProvider =
                DataDir()->Find<PropertyEventProvider>("multiuser.ep", false);
            if (multiUserProvider) {
                multiUserProvider->SetProperty(side, true);
            }
        }
    }

    if (msg.GetButton() == kPad_LStickRight || msg.GetButton() == kPad_DRight) {
        if (TheUI->FocusPanel() && TheUI->FocusComponent() == unk54[1]) {
            TheUI->FocusPanel()->SetFocusComponent(unk54[0]);
            PropertyEventProvider *multiUserProvider =
                DataDir()->Find<PropertyEventProvider>("multiuser.ep", false);
            if (multiUserProvider) {
                multiUserProvider->SetProperty(side, false);
            }
        }
    }
    return DATA_UNHANDLED;
}

BEGIN_HANDLERS(MultiUserGesturePanel)
    HANDLE_EXPR(
        get_char_provider, const_cast<CharacterProvider *>(GetCharProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(
        get_crew_provider, const_cast<CrewProvider *>(GetCrewProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(
        get_outfit_provider, const_cast<OutfitProvider *>(GetOutfitProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(
        get_difficulty_provider,
        dynamic_cast<DifficultyProvider *>(GetDifficultyProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(
        get_venue_provider, const_cast<VenueProvider *>(GetVenueProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(
        is_skeleton_present, TheGameData->IsSkeletonPresent(GetPlayerIndex(_msg->Int(2)))
    )
    HANDLE_EXPR(is_character_available, IsCharacterAvailable(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(is_crew_available, IsCrewAvailable(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(get_character, GetCharacter(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(set_character, SetCharacter(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(get_outfit, GetOutfit(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(set_outfit, SetOutfit(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(set_crew, SetCrew(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(set_default_character, SetDefaultCharacter(_msg->Int(2)))
    HANDLE_ACTION(set_random_crew, SetRandomCrew(_msg->Int(2)))
    HANDLE_ACTION(
        update_char_pic,
        UpdateCharPic(
            _msg->Obj<UIPicture>(2), _msg->Int(3), _msg->Int(4), _msg->Sym(5), _msg->Sym(6)
        )
    )
    HANDLE_ACTION(
        update_crew_pic,
        UpdateCrewPic(_msg->Obj<UIPicture>(2), _msg->Int(3), _msg->Int(4), _msg->Sym(5))
    )
    HANDLE_ACTION(
        update_venue_mesh,
        UpdateVenueMesh(
            _msg->Obj<RndMesh>(2), _msg->Int(3), _msg->Int(4), _msg->Sym(5), _msg->Sym(6)
        )
    )
    HANDLE_EXPR(get_character_index, GetCharacterIndex(_msg->Int(2)))
    HANDLE_EXPR(get_outfit_index, GetOutfitIndex(_msg->Int(2)))
    HANDLE_EXPR(get_venue_index, GetVenueIndex(_msg->Int(2), _msg->Sym(3)))
    HANDLE_EXPR(get_crew_index, GetCrewIndex(_msg->Int(2)))
    HANDLE_EXPR(get_player_index, GetPlayerIndex(_msg->Int(2)))
    HANDLE_ACTION(update_provider_player_indices, UpdateProviderPlayerIndices())
    HANDLE_ACTION(drop_side, DropPlayerOnSide(_msg->Int(2)))
    HANDLE_EXPR(
        get_voice_command_outfit_tag, GetVoiceCommandOutfitTag(_msg->Int(2), _msg->Sym(3))
    )
    HANDLE_ACTION(update_providers, UpdateProviders())
    HANDLE_MESSAGE(ButtonDownMsg) HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS
