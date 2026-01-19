#pragma once
#include "gesture/SpeechMgr.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include <vector>

class VoiceInputPanel : public HamPanel {
public:
    struct Config {
        std::vector<Symbol> mGrammars; // 0x0
        DataArray *mAltConfThresholds; // 0xc
        float mDefaultConfThreshold; // 0x10
    };

    struct VoiceContext {
        void SetActiveConfig(bool);
        Symbol GetGrammarSym(int index) const {
            MILO_ASSERT_RANGE(index, 0, mActiveConfig->mGrammars.size(), 0x4B);
            return mActiveConfig->mGrammars[index];
        }

        Symbol mName; // 0x0
        Config *mBlacklightOffConfig; // 0x4
        Config *mBlacklightOnConfig; // 0x8
        Config *mActiveConfig; // 0xc
        float mConfThreshold; // 0x10
    };

    VoiceInputPanel();
    virtual ~VoiceInputPanel();
    OBJ_CLASSNAME(VoiceInputPanel)
    OBJ_SET_TYPE(VoiceInputPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Draw() { UIPanel::Draw(); }
    virtual void Enter() { HamPanel::Enter(); }
    virtual void Exit() { UIPanel::Exit(); }
    virtual void Poll() { HamPanel::Poll(); }
    virtual void Unload() { UIPanel::Unload(); }
    virtual void FinishLoad() { UIPanel::FinishLoad(); }

    NEW_OBJ(VoiceInputPanel)

    DataNode OnMsg(const SpeechRecoMessage &);
    void CreateSongSelectGrammar(Symbol) const;
    void CreatePlaylistEditorGrammar() const;
    void ToggleBlacklight(bool);

private:
    Symbol ActiveVoiceContextSym();
    void OnConfidenceChange(float);
    void ActivateVoiceContext(Symbol);
    void DisableCurrentVoiceContext();
    void RestoreCurrentVoiceContext();
    void LoadVoiceContexts();

    bool unk3c;
    std::vector<Symbol> unk40;
    std::vector<VoiceContext *> mVoiceContexts; // 0x4c
    VoiceContext *mActiveVoiceContext; // 0x58
    VoiceContext *mDisabledVoiceContext; // 0x5c
};

extern VoiceInputPanel *TheVoiceInputPanel;
