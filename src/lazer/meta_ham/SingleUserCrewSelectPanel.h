#pragma once
#include "meta_ham/CharacterProvider.h"
#include "meta_ham/CrewProvider.h"
#include "meta_ham/OutfitProvider.h"
#include "meta_ham/TexLoadPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "utl/Symbol.h"

class SingleUserCrewSelectPanel : public TexLoadPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(classname)
    OBJ_SET_TYPE(SingleUserCrewSelectPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    SingleUserCrewSelectPanel();
    void UpdateProviders();
    int GetPlayerIndex(int) const;
    void UpdateProviderPlayerIndices();
    CrewProvider const *GetCrewProvider(int) const;
    CharacterProvider const *GetCharProvider(int) const;
    OutfitProvider const *GetOutfitProvider(int) const;
    void SetRandomOutfit(int);
    bool IsCrewAvailable(Symbol, int);
    void SetRandomCharacter(int);
    void UpdateCrewMesh(RndMesh *, int, Symbol);
    void SetRandomCrew(int);
    void RefreshUI();
    void SetCrew(Symbol, int);

protected:
    int mActiveSide; // 0x54
    CrewProvider mCrewProviders[2]; // 0x58
    CharacterProvider mCharProviders[2]; // 0xe0
    OutfitProvider mOutfitProviders[2]; // 0x168
};
