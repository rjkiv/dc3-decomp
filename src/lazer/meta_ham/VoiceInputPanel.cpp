#include "meta_ham/VoiceInputPanel.h"
#include "gesture/SpeechMgr.h"
#include "meta_ham/HamPanel.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

VoiceInputPanel *TheVoiceInputPanel;

VoiceInputPanel::VoiceInputPanel() {
    unk3c = false; // i really gotta declare these here huh
    unk58 = 0;
    mDisabledVoiceContext = 0;
    LoadVoiceContexts();
    TheVoiceInputPanel = this;
}

VoiceInputPanel::~VoiceInputPanel() { TheVoiceInputPanel = nullptr; }

void VoiceInputPanel::FinishLoad() { UIPanel::FinishLoad(); }

void VoiceInputPanel::Exit() { UIPanel::Exit(); }

void VoiceInputPanel::Draw() { UIPanel::Draw(); }

Symbol VoiceInputPanel::ActiveVoiceContextSym() {
    if (unk58 == 0) {
        return gNullStr;
    } else
        return unk58->unk0;
}

void VoiceInputPanel::DisableCurrentVoiceContext() {
    MILO_ASSERT(mDisabledVoiceContext==NULL, 0x1f4);
    if (unk58) {
        mDisabledVoiceContext = unk58;
        ActivateVoiceContext(gNullStr);
    }
}

void VoiceInputPanel::RestoreCurrentVoiceContext() {
    if (mDisabledVoiceContext) {
        ActivateVoiceContext(mDisabledVoiceContext->unk0);
        mDisabledVoiceContext = nullptr;
    }
}

BEGIN_HANDLERS(VoiceInputPanel)
    HANDLE_ACTION(
        create_song_select_grammar,
        CreateSongSelectGrammar(GetSongTitlePronunciationLanguage())
    )
    HANDLE_ACTION(create_playlist_editor_grammar, CreatePlaylistEditorGrammar())
    // toggle_blacklight
    HANDLE_ACTION(activate_voice_context, ActivateVoiceContext(_msg->Sym(2)))
    HANDLE_ACTION(disable_current_voice_context, DisableCurrentVoiceContext())
    HANDLE_ACTION(restore_current_voice_context, RestoreCurrentVoiceContext())
    HANDLE_EXPR(active_voice_context_sym, ActiveVoiceContextSym())
    // HANDLE_EXPR(get_active_voice_context_name, GetActiveVoiceContextName())
    HANDLE_ACTION(confidence_up, OnConfidenceChange(0.05f))
    HANDLE_ACTION(confidence_down, OnConfidenceChange(-0.05f))
    HANDLE_MESSAGE(SpeechRecoMessage)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
