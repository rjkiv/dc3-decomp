#pragma once
#include "HamPanel.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/Sound.h"
#include "utl/Symbol.h"

class LockedContentPanel : public HamPanel {
public:
    // Hmx::Object
    virtual ~LockedContentPanel();
    OBJ_CLASSNAME(LockedContentPanel)
    OBJ_SET_TYPE(LockedContentPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();

    NEW_OBJ(LockedContentPanel)

    LockedContentPanel();
    void SetUpCampaignSong(Symbol);
    void SetUpCampaignMasterQuestHeader(Symbol);
    void SetUp(Symbol);
    void SetUpNoFlashcards(Symbol, Difficulty);
    void SetUpDifficultyLocked(Symbol, Symbol);
    void SetVoiceOver(Sound *, bool);

protected:
    virtual void FinishLoad();

    void TriggerTeaserText();

    u32 filler[16]; // something to do with HamStarsDisplay, just not sure yet
    Sound *mSound; // 0x7c
    Timer *mTimer; // 0x80
    bool unk84;
};

extern LockedContentPanel *TheLockedContentPanel;
