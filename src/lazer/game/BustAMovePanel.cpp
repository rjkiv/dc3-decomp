#include "BustAMovePanel.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamDirector.h"
#include "lazer/game/GameMode.h"
#include "lazer/meta_ham/HamPanel.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

BustAMovePanel::BustAMovePanel() {}

BustAMovePanel::~BustAMovePanel() {}

bool BustAMovePanel::InBustAMove() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol bustamove("bustamove");
    return TheGameMode->Property(gameplay_mode, true)->Sym() == bustamove;
}

void BustAMovePanel::Exit() {
    UIPanel::Exit();
    // TheMaster->RemoveSink(this);
    TheHamDirector->SetPlayerSpotlightsEnabled(true);
}

BEGIN_PROPSYNCS(BustAMovePanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void BustAMovePanel::QueueMovePromptVO() { float vo_length = GetMovePromptVOLength(); }

void BustAMovePanel::PlayMovePromptVO() {
    int i = unk_0xA0;
    Symbol vo;

    DataArray *movedata = GetMoveNameData(unk_0x84);

    vo = movedata->Sym(i + 2);
    PlayVO(vo);
}

void BustAMovePanel::PlayIntroVO() {}

void BustAMovePanel::Poll() {
    if (InBustAMove()) {
        HamPanel::Poll();
    }
}

void BustAMovePanel::OnBeat() {
    if (InBustAMove() /* && TheGamePanel->unk_0x80 != 3 */) {
        int beat = TheHamProvider->Property("beat", true)->Int();
        if (beat != -1) {
            if (beat == 4) {
                for (int i = 2; i != 0; i--) {
                    mDirs[2 - i]
                        ->Find<RndPropAnim>("advance.anim", false)
                        ->Animate(0.0f, 0, 0.0f, nullptr, kEaseLinear, 0.0f, false);
                }
                if (unk_0x3C == 1) {
                    static Message endMessage("bustamove_end_create");
                }
            }
        }
    }
}

BEGIN_HANDLERS(BustAMovePanel)
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_ACTION(cache_objects, CacheObjects())
    HANDLE_ACTION(set_up_song_structure, SetUpSongStructure(_msg->Sym(2)))
    HANDLE_ACTION(on_stream_jump, 0)
    HANDLE_ACTION(play_intro_vo, PlayIntroVO())
    HANDLE_SUPERCLASS(HamPanel)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
