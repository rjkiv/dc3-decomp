#pragma once
#include "CampaignSongSelectPanel.h"
#include "TexLoadPanel.h"
#include "meta_ham/MQSongSortMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "ui/UILabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CampaignMasterQuestSongSelectPanel : public TexLoadPanel {
public:
    CampaignMasterQuestSongSelectPanel();
    virtual ~CampaignMasterQuestSongSelectPanel();
    OBJ_CLASSNAME(CampaignMasterQuestSongSelectPanel)
    OBJ_SET_TYPE(CampaignMasterQuestSongSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Enter();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    NEW_OBJ(CampaignMasterQuestSongSelectPanel)

    bool CanSelectSong(int);
    Symbol GetSong(int);
    int GetTimeSinceEnter() const;
    Symbol GetSelectedSong();
    void OnHighlightSong();
    bool CanSelectCurrentSong();
    bool IsCurrentSelectionSong();
    void Refresh();
    void OnHighlightHeader();
    void SelectSong();

private:
    // size 0xc
    class Impl {
    public:
        Impl() {}
        void Init(CampaignMasterQuestSongSelectPanel *);

        UILabel *m_ContextualTitle; // 0x0
        UILabel *m_ContextualInstructions; // 0x4
        UILabel *m_ContextualStars; // 0x8
    };

    MQSongSortMgr *m_pCampaignSongProvider; // 0x54
    DateTime m_cEnterTime; // 0x58
    bool m_bDelayFinished; // 0x5e
    Impl *m_Impl; // 0x60
};
