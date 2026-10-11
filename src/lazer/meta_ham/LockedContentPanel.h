#pragma once
#include "HamPanel.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/HamStarsDisplay.h"
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

private:
    AppLabel *mSongName[8]; // 0x3c
    HamStarsDisplay *mSongStars[8]; // 0x5c
    Sound *mSound; // 0x7c - not in pdb
    Timer *mTimer; // 0x80 - not in pdb
    bool mPlaying; // 0x84 - not in pdb
};

extern LockedContentPanel *TheLockedContentPanel;
