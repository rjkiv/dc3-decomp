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
    // Hmx::Object
    virtual ~VoiceControlPanel();
    OBJ_CLASSNAME(VoiceControlPanel)
    OBJ_SET_TYPE(VoiceControlPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Poll();
    virtual void Dismiss();

    virtual void ContentMounted(char const *, char const *);

    NEW_OBJ(VoiceControlPanel)

    VoiceControlPanel();
    void SetRules(bool);
    DataNode OnMsg(UITransitionCompleteMsg const &);
    DataNode OnMsg(SpeechRecoMessage const &);
    void PopUp();

protected:
    float unk44;
    bool unk48;
    float unk4c;
    float unk50;
    bool unk54;
    bool unk55;
    bool unk56;
    Symbol unk58;
    Difficulty mDifficulty; // 0x5c
    Symbol unk60;
    bool unk64;
    int unk68;
    int unk6c;

private:
    bool DifficultyLocked() const;
    void WakeUpScreenSaver() const;
    bool ReadyToStart() const;
    void DisplaySong(Symbol);
    void EnterGame();
    void CreatePlaySongGrammar() const;
    void CycleTip();
};
