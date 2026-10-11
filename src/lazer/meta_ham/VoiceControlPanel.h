#pragma once
#include "gesture/SpeechMgr.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/OverlayPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

class VoiceControlPanel : public OverlayPanel, public ContentMgr::Callback {
public:
    VoiceControlPanel();
    // Hmx::Object
    virtual ~VoiceControlPanel();
    OBJ_CLASSNAME(VoiceControlPanel)
    OBJ_SET_TYPE(VoiceControlPanel)
    virtual DataNode Handle(DataArray *, bool);
    // UIPanel
    virtual void Poll();
    virtual void Dismiss();
    // ContentMgr::Callback
    virtual void ContentMounted(char const *, char const *);

    NEW_OBJ(VoiceControlPanel)

    void SetRules(bool);
    void PopUp();

    DataNode OnMsg(const UITransitionCompleteMsg &);
    DataNode OnMsg(const SpeechRecoMessage &);

private:
    bool DifficultyLocked() const;
    void WakeUpScreenSaver() const;
    bool ReadyToStart() const;
    void DisplaySong(Symbol);
    void EnterGame();
    void CreatePlaySongGrammar() const;
    void CycleTip();

    float mEnterTime; // 0x44
    bool mEntered; // 0x48
    float mPickRandomSongTimer; // 0x4c
    float mCycleTipTimer; // 0x50
    bool mHasSelectedDifficulty; // 0x54
    bool mHasSelectedMode; // 0x55
    bool mMustTurnMusicBackOn; // 0x56
    Symbol mSong; // 0x58
    Difficulty mDiff; // 0x5c
    Symbol mMode; // 0x60
    bool mDismissOnTransitionComplete; // ox64
    int mNumShellCmds; // 0x68
    int mNumBidCmds; // 0x6c
};
