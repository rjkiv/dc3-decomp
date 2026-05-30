#include "FxSendDelay.h"
#include "FxSend.h"
#include "dsp/DelayEffect.h"
#include "dsp/StandardEffect.h"

FxSendDelay360::FxSendDelay360() : FxSend360(this) {}

FxSendDelay360::~FxSendDelay360() {}

void FxSendDelay360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendDelay360::UpdateMix() { UpdateVolumes(); }

void FxSendDelay360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

IUnknown *FxSendDelay360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<DelayEffect>());
}
