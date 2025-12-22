#pragma once
#include "CampaignSongSelectPanel.h"
#include "TexLoadPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "utl/Symbol.h"

class CampaignMasterQuestSongSelectPanel : public TexLoadPanel {
public:
    virtual ~CampaignMasterQuestSongSelectPanel();
    OBJ_CLASSNAME(CampaignMasterQuestSongSelectPanel)
    OBJ_SET_TYPE(CampaignMasterQuestSongSelectPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Enter();
    virtual void Poll();
    virtual void Unload();
    virtual void FinishLoad();

    CampaignMasterQuestSongSelectPanel();
    bool CanSelectSong(int);
    Symbol GetSong(int);
    int GetTimeSinceEnter() const;
    Symbol GetSelectedSong();
    void OnHighlightSong();
    bool CanSelectCurrentSong();
    bool IsCurrentSelectionSong();
    void Refresh();
    void OnHighlightHeader();

protected:
    CampaignSongProvider *m_pCampaignSongProvider; // 0x54
    DateTime mDateTime; // 0x58
    bool unk5e;
    int unk60;
};
