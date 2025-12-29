#pragma once

#include "gesture/SpeechMgr.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"
class VoiceInputPanel : public HamPanel {
public:
    struct VoiceContext {
    public:
        void SetActiveConfig(bool);
        Symbol GetGrammarSym(int) const;

        Symbol unk0;
    };

    virtual ~VoiceInputPanel();
    OBJ_CLASSNAME(VoiceInputPanel)
    OBJ_SET_TYPE(VoiceInputPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Draw();
    virtual void FinishLoad();
    virtual void Exit();

    NEW_OBJ(VoiceInputPanel)

    VoiceInputPanel();
    DataNode OnMsg(SpeechRecoMessage const &);
    void CreateSongSelectGrammar(Symbol) const;
    void CreatePlaylistEditorGrammar() const;
    void ToggleBlacklight(int i);
    Symbol GetActiveVoiceContextName();

protected:
    bool unk3c;
    std::vector<Symbol> unk40;
    std::vector<VoiceContext *> unk4c;
    VoiceContext *unk58;
    VoiceContext *mDisabledVoiceContext; // 0x5c

private:
    Symbol ActiveVoiceContextSym();
    void OnConfidenceChange(float);
    void ActivateVoiceContext(Symbol);
    void DisableCurrentVoiceContext();
    void RestoreCurrentVoiceContext();
    void LoadVoiceContexts();
};

extern VoiceInputPanel *TheVoiceInputPanel;
