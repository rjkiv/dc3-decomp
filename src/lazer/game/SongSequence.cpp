#include "game/SongSequence.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

SongSequence::SongSequence() {}

SongSequence::~SongSequence() {}

void SongSequence::Init() {
    unk2c = nullptr;
    SetName("songseq", ObjectDir::Main());
    Clear();
}

bool SongSequence::Done() const { return unk8.empty() || mCurrentIndex >= unk8.size(); }

void SongSequence::Add(DataArray const *d) {
    static Symbol babygotback("babygotback");
    static Symbol perform("perform");
    static Symbol holla_back_config_default("holla_back_config_default");
    if (d) {
    }
}

BEGIN_HANDLERS(SongSequence)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(add, Add(_msg))
    HANDLE_EXPR(do_next, DoNext(false, _msg->Int(3) != 0))
    HANDLE_EXPR(done, Done())
    HANDLE_ACTION(on_rhythm_battle_combo_full, DoNext(false, false))
    HANDLE_ACTION(load_next_song_audio, LoadNextSongAudio())
    HANDLE_EXPR(get_intro_cam_shot, GetIntroCamShot())
    HANDLE_EXPR(get_outro_cam_shot, GetOutroCamShot())
    HANDLE_EXPR(loop_start, unk18) // fix this
    HANDLE_EXPR(empty, unk8.empty())
    HANDLE_EXPR(current_index, mCurrentIndex)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
