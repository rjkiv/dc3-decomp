#include "FxSend.h"
#include "Synth.h"
#include "os/Debug.h"
#include "synth/FxSend.h"

FxSend360::FxSend360(FxSend *fx) : unk4(0), mThis(fx), unk30(true) {
    TheXboxSynth->AddFxSend(this);
    MILO_ASSERT(mThis, 0x19);
}

FxSend360::~FxSend360() {
    if (TheXboxSynth)
        TheXboxSynth->RemoveFxSend(this);
    CleanChain();
}

void FxSend360::AddOwnerVoice(Voice *v) { unk34.push_back(v); }
