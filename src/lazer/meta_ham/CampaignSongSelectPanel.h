#pragma once
#include "TexLoadPanel.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamNavProvider.h"
#include "meta_ham/CampaignEra.h"
#include "meta_ham/CampaignProgress.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class CampaignSongProvider : public HamNavProvider {
public:
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void Custom(int, int, UIListCustom *, Hmx::Object *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const;

    CampaignSongProvider();
    bool IsSongAvailable(Symbol) const;
    int SymbolIndex(Symbol) const;
    bool IsSongPlayed(Symbol) const;
    bool IsCrazeSong(Symbol) const;
    void UpdateList();

    void SetPanelDir(PanelDir *p) { unk40 = p; }

protected:
    PanelDir *unk40;
    std::vector<Symbol> unk44;
};

class CampaignSongSelectPanel : TexLoadPanel {
public:
    virtual ~CampaignSongSelectPanel();
    OBJ_CLASSNAME(CampaignSongSelectPanel)
    OBJ_SET_TYPE(CampaignSongSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Load();
    virtual void Enter();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    CampaignSongSelectPanel();
    bool CanSelectSong(int);
    Symbol GetSong(int);
    int GetTimeSinceEnter() const;
    int GetStarsRequiredForEraSong() const;
    int GetEraStars() const;
    int GetSongIndex(Symbol);
    int GetEraSongStars() const;
    Symbol GetSelectedSong();
    void SelectSong();
    bool IsWaitingForEraSongUnlock();
    bool CanSelectCurrentSong();
    void Refresh();
    void CheatWinEraSong(Symbol, int);
    void CheatTransitionPending();

protected:
    CampaignSongProvider *m_pCampaignSongProvider; // 0x54
    DateTime unk58;
    bool mPreviewDelayFinished; // 0x5e
    Symbol unk60;
    CampaignEra *m_pCurCampaignEra; // 0x64
    Difficulty mDifficulty; // 0x68
    CampaignProgress *m_pCurCampaignProgress; // 0x6c
};
