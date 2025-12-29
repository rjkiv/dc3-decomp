#include "meta_ham/MultiUserGesturePanel.h"
#include "HamPanel.h"
#include "MultiUserGesturePanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/CharacterProvider.h"
#include "meta_ham/CrewProvider.h"
#include "meta_ham/DifficultyProvider.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/OutfitProvider.h"
#include "meta_ham/SkeletonChooser.h"
#include "meta_ham/TexLoadPanel.h"
#include "meta_ham/VenueProvider.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Mesh.h"
#include "ui/UI.h"
#include "ui/UIPicture.h"
#include "utl/Symbol.h"

MultiUserGesturePanel::MultiUserGesturePanel() {
    for (int i = 0; i < 2; i++) {
        mCharacterProviders[i].unk30 = 0;
        mCrewProviders[i].unk30 = i;
        mDifficultyProviders[i].unk30 = i;
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

void MultiUserGesturePanel::SetRandomCrew(int idx) { // registers are off here
    static Symbol random_crew("random_crew");
    int index = GetPlayerIndex(idx);
    HamPlayerData *pPlayerData = TheGameData->Player(index == 0);
    MILO_ASSERT(pPlayerData, 0x22e);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
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

void MultiUserGesturePanel::SetCrew(Symbol crew, int idx) { // registers are off here
    int index = GetPlayerIndex(idx);
    HamPlayerData *pPlayerData = TheGameData->Player(index == 0);
    MILO_ASSERT(pPlayerData, 0x131);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pOtherPlayerData, 0x133);
    Symbol s = "";
    if (crew != s && pOtherPlayerData->Crew() == crew) {
        pOtherPlayerData->SetCrew(pPlayerData->Crew());
        pOtherPlayerData->SetCharacter(pPlayerData->Char());
        pOtherPlayerData->SetOutfit(pPlayerData->Outfit());
        RefreshUI();
    }
    MetaPerformer *performer = MetaPerformer::Current();
    MILO_ASSERT(performer, 0x143);
    pPlayerData->SetCrew(crew);
    Symbol crewCheck = "";
    if (crew != crewCheck) {
        const CharacterProvider *pCharacterProvider = GetCharProvider(idx);
        MILO_ASSERT(pCharacterProvider, 0x14a);
        const_cast<CharacterProvider *>(pCharacterProvider)->UpdateList();
        SetRandomCharacter(idx);
    }
}

void MultiUserGesturePanel::RefreshUI() {
    static Message refresh_ui("refresh_ui");
    TheUI->Handle(refresh_ui, false);
}

int MultiUserGesturePanel::GetPlayerIndex(int idx) const {
    SkeletonChooser *pSkeletonChooser = TheHamUI.GetShellInput()->GetSkeletonChooser();
    MILO_ASSERT(pSkeletonChooser, 0x68);
    SkeletonSide playerSide = pSkeletonChooser->GetPlayerSide(0);
    const DataNode *prop = TheHamProvider->Property("is_in_party_mode", true);
    int check = prop->Int();
    if (check == 0) {
        check = playerSide - 1;
        if (idx == 0) {
            idx = check - ((playerSide - 2) + (playerSide - 1 == 0));
        } else {
            idx = playerSide == 0;
        }
    }
    return idx;
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
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS
