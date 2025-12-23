#include "meta_ham/SingleUserCrewSelectPanel.h"
#include "HamPanel.h"
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
        mCrewProviders[i].unk30 = i;
        mCharProviders[i].unk30 = i;
        mOutfitProviders[i].unk30 = i;
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
        mCharProviders[i].unk30 = playerIndex;
        mOutfitProviders[i].unk30 = playerIndex;
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
    HamPlayerData *pPlayerData = TheGameData->Player(GetPlayerIndex(index));
    MILO_ASSERT(pPlayerData, 0x9e);
    HamPlayerData *pOtherPlayerData = TheGameData->Player(GetPlayerIndex(index));
    MILO_ASSERT(pPlayerData, 0xa0);
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
