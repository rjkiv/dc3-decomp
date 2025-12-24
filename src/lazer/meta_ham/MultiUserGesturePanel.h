#pragma once
#include "meta_ham/CharacterProvider.h"
#include "meta_ham/CrewProvider.h"
#include "meta_ham/DifficultyProvider.h"
#include "meta_ham/OutfitProvider.h"
#include "meta_ham/TexLoadPanel.h"
#include "meta_ham/VenueProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "ui/UIPicture.h"
#include "utl/Symbol.h"

class MultiUserGesturePanel : public TexLoadPanel {
public:
    OBJ_CLASSNAME(MultiUserGesturePanel)
    OBJ_SET_TYPE(MultiUserGesturePanel)
    virtual DataNode Handle(DataArray *, bool);

    virtual void Enter();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();
    virtual bool HasNavList() const;
    virtual bool Exiting() const;

    MultiUserGesturePanel();
    void UpdateProviders();
    int GetPlayerIndex(int) const;
    void UpdateProviderPlayerIndices();
    CharacterProvider const *GetCharProvider(int) const;
    VenueProvider const *GetVenueProvider(int) const;
    CrewProvider const *GetCrewProvider(int) const;
    OutfitProvider const *GetOutfitProvider(int) const;
    DifficultyProvider *GetDifficultyProvider(int);
    Symbol GetOutfit(int, int) const;
    void SetOutfit(Symbol, int);
    int GetCharacterIndex(int) const;
    int GetOutfitIndex(int) const;
    int GetCrewIndex(int) const;
    int GetVenueIndex(int, Symbol) const;
    void UpdateCharPic(UIPicture *, int, int, Symbol, Symbol);
    void UpdateCrewPic(UIPicture *, int, int, class Symbol);
    void SetDefaultCharacter(int);
    void SetRandomOutfit(int);
    Symbol GetCharacter(int, int);
    bool IsCrewAvailable(Symbol, int);
    bool IsCharacterAvailable(Symbol, int);
    Symbol GetVoiceCommandOutfitTag(int, Symbol);
    void DropPlayerOnSide(int);
    void SetRandomCharacter(int);
    DataNode OnMsg(ButtonDownMsg const &);
    void UpdateVenueMesh(RndMesh *, int, int, Symbol, Symbol);
    void SetRandomCrew(int);
    void RefreshUI();
    void SetCharacter(Symbol, int);
    void SetCrew(Symbol, int);

protected:
    u32 unk54;
    u32 unk58;
    u32 unk5c;
    u32 unk60;
    CharacterProvider mCharacterProviders[2]; // 0x64
    CrewProvider mCrewProviders[2]; // 0xec
    DifficultyProvider mDifficultyProviders[2]; // 0x174
    OutfitProvider mOutfitProviders[2]; // 0x1f4
    VenueProvider mVenueProviders[2]; // 0x274

private:
    void UpdateNavLists(int);
};
