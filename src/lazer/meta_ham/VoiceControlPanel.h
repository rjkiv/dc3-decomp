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

    float unk44;
    bool unk48;
    float unk4c;
    float unk50;
    bool unk54;
    bool unk55;
    bool unk56;
    Symbol unk58; // 0x58 - song shortname?
    Difficulty mDifficulty; // 0x5c
    Symbol unk60; // 0x60 - game mode
    bool unk64;
    int unk68;
    int unk6c;
};
