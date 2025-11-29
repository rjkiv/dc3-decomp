#include "synth_xbox/Synth.h"
#include "obj/Object.h"

Synth360 *TheXboxSynth;

Synth360::Synth360()
    : unke8(0), unkec(0), unkf0(0), unkf4(0), unkf8(0), unkfc(0), unk104(true),
      unk105(false), unk138(false), unk13c(0), unk14c(false) {}

BEGIN_HANDLERS(Synth360)
    HANDLE_SUPERCLASS(Synth)
END_HANDLERS
