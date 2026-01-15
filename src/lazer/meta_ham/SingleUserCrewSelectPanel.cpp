#include "meta_ham/SingleUserCrewSelectPanel.h"
#include "HamPanel.h"
#include "HamUI.h"
#include "SkeletonChooser.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "gesture/BaseSkeleton.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/CharacterProvider.h"
#include "meta_ham/CrewProvider.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/OutfitProvider.h"
#include "meta_ham/TexLoadPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

SingleUserCrewSelectPanel::SingleUserCrewSelectPanel() {
    for (int i = 0; i < 2; i++) {
        mCrewProviders[i].SetPlayer(i);
        mCharProviders[i].SetPlayer(i);
        mOutfitProviders[i].SetPlayer(i);
    }
}

void SingleUserCrewSelectPanel::Enter() {
    UpdateProviderPlayerIndices();
    HamPanel::Enter();
    UpdateProviders();
}

void SingleUserCrewSelectPanel::Poll() {
    if (!TheUI->InTransition())
        UpdateProviderPlayerIndices();
    TexLoadPanel::Poll();
}

void SingleUserCrewSelectPanel::Unload() {
    TexLoadPanel::Unload();
    for (int i = 0; i < 2; i++) {
        mCrewProviders[i].SetPanelDir(nullptr);
        mCharProviders[i].SetPanelDir(nullptr);
    }
}

void SingleUserCrewSelectPanel::FinishLoad() {
    TexLoadPanel::FinishLoad();
    for (int i = 0; i < 2; i++) {
        mCrewProviders[i].SetPanelDir(LoadedDir());
        mCharProviders[i].SetPanelDir(LoadedDir());
    }
}

void SingleUserCrewSelectPanel::UpdateProviders() {
    for (int i = 0; i < 2; i++) {
        mCrewProviders[i].UpdateList();
        mCharProviders[i].UpdateList();
        mOutfitProviders[i].UpdateList();
    }
}

void SingleUserCrewSelectPanel::UpdateProviderPlayerIndices() {
    for (int i = 0; i < 2; i++) {
        int playerIndex = GetPlayerIndex(i);
        mCharProviders[i].SetPlayer(playerIndex);
        mOutfitProviders[i].SetPlayer(playerIndex);
    }
}

CrewProvider const *SingleUserCrewSelectPanel::GetCrewProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0x85);
    return &mCrewProviders[index];
}

CharacterProvider const *SingleUserCrewSelectPanel::GetCharProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0x8c);
    return &mCharProviders[index];
}

OutfitProvider const *SingleUserCrewSelectPanel::GetOutfitProvider(int index) const {
    MILO_ASSERT_RANGE(index, 0, 2, 0x93);
    return &mOutfitProviders[index];
}

void SingleUserCrewSelectPanel::SetRandomOutfit(int index) {
    HamPlayerData *pPlayerData = TheGameData->Player(GetPlayerIndex(index));
    MILO_ASSERT(pPlayerData, 0x134);
    const OutfitProvider *pOutfitProvider = GetOutfitProvider(index);
    MILO_ASSERT(pOutfitProvider, 0x137);
    Symbol randomAvailOutfit = pOutfitProvider->GetRandomAvailableOutfit();
    pPlayerData->SetOutfit(randomAvailOutfit);
}

void SingleUserCrewSelectPanel::SetCrew(Symbol crew, int index) {
    int idx = GetPlayerIndex(index);
    int otherindex = idx == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(idx);
    MILO_ASSERT(pPlayerData, 0x9e);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0xa0);
    if (pOtherPlayerData->Crew() == crew) {
        pOtherPlayerData->SetCrew(pPlayerData->Crew());
        pOtherPlayerData->SetCharacter(pPlayerData->Char());
        pOtherPlayerData->SetOutfit(pPlayerData->Outfit());
        RefreshUI();
    }
    MetaPerformer *performer = MetaPerformer::Current();
    MILO_ASSERT(performer, 0xad);
    pPlayerData->SetCrew(crew);
    const CharacterProvider *pCharacterProvider = GetCharProvider(index);
    MILO_ASSERT(pCharacterProvider, 0xb2);
    const_cast<CharacterProvider *>(pCharacterProvider)->UpdateList();
    SetRandomCharacter(index);
}

bool SingleUserCrewSelectPanel::IsCrewAvailable(Symbol crew, int i) {
    const CrewProvider *pProvider = GetCrewProvider(i);
    MILO_ASSERT(pProvider, 0x140);
    return pProvider->IsCrewAvailable(crew);
}

void SingleUserCrewSelectPanel::RefreshUI() {
    static Message refresh_ui("refresh_ui");
    TheUI->Handle(refresh_ui, false);
}

void SingleUserCrewSelectPanel::SetRandomCharacter(int idx) {
    int index = GetPlayerIndex(idx);
    int otherindex = index == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0xfc);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0xfe);
    const CharacterProvider *pCharProvider = GetCharProvider(idx);
    MILO_ASSERT(pCharProvider, 0x101);
    Symbol symRandomCharacter = pCharProvider->GetRandomAvailableCharacter();
    MILO_ASSERT(symRandomCharacter != gNullStr, 0x104);
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
        pPlayerData->SetOutfit(GetCharacterOutfit(symRandomCharacter, 0));
    } else {
        const OutfitProvider *pOutfitProvider = GetOutfitProvider(idx);
        MILO_ASSERT(pOutfitProvider, 0x129);
        const_cast<OutfitProvider *>(pOutfitProvider)->UpdateList();
        SetRandomOutfit(idx);
    }
}

void SingleUserCrewSelectPanel::SetRandomCrew(int idx) {
    static Symbol random_crew("random_crew");
    int index = GetPlayerIndex(idx);
    int otherindex = index == 0;
    HamPlayerData *pPlayerData = TheGameData->Player(index);
    MILO_ASSERT(pPlayerData, 0xe3);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(otherindex);
    MILO_ASSERT(pOtherPlayerData, 0xe5);
    const CrewProvider *pCrewProvider = GetCrewProvider(idx);
    MILO_ASSERT(pCrewProvider, 0xe8);
    Symbol symRandomCrew = pCrewProvider->GetRandomAvailableCrew();
    MILO_ASSERT(symRandomCrew != gNullStr, 0xeb);
    pPlayerData->SetCrew(symRandomCrew);
    const CharacterProvider *pCharacterProvider = GetCharProvider(idx);
    MILO_ASSERT(pCharacterProvider, 0xf0);
    const_cast<CharacterProvider *>(pCharacterProvider)->UpdateList();
    SetRandomCharacter(idx);
}

int SingleUserCrewSelectPanel::GetPlayerIndex(int i) const {
    SkeletonChooser *pSkeletonChooser = TheHamUI.GetShellInput()->mSkelChooser;
    MILO_ASSERT(pSkeletonChooser, 0x52);
    SkeletonSide skelSide = pSkeletonChooser->GetPlayerSide(0);
    Symbol is_in_party_mode("is_in_party_mode");
    const DataNode *node = TheHamProvider->Property(is_in_party_mode, true);
    if (node->Int() == 0) {
    }
    return i == 0;
}

BEGIN_HANDLERS(SingleUserCrewSelectPanel)
    HANDLE_EXPR(
        get_crew_provider, const_cast<CrewProvider *>(GetCrewProvider(_msg->Int(2)))
    )
    HANDLE_EXPR(is_crew_available, IsCrewAvailable(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(set_crew, SetCrew(_msg->Sym(2), _msg->Int(3)))
    HANDLE_ACTION(set_random_crew, SetRandomCrew(_msg->Int(2)))
    HANDLE_ACTION(
        update_crew_mesh,
        UpdateCrewMesh(_msg->Obj<RndMesh>(2), _msg->Int(3), _msg->Sym(4))
    )
    HANDLE_ACTION(update_provider_player_indices, UpdateProviderPlayerIndices())
    HANDLE_ACTION(update_providers, UpdateProviders())
    HANDLE_ACTION(set_active_side, mActiveSide = _msg->Int(2))
    HANDLE_SUPERCLASS(TexLoadPanel)
END_HANDLERS
