#include "FxSendChorus.h"
#include "FxSend.h"
#include "dsp/FlangerEffect.h"
#include "dsp/StandardEffect.h"

FxSendChorus360::FxSendChorus360() : FxSend360(this) {}

FxSendChorus360::~FxSendChorus360() {}

void FxSendChorus360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendChorus360::UpdateMix() { UpdateVolumes(); }

void FxSendChorus360::OnParametersChanged() { FxSend360::SyncEffectParams(); }

IUnknown *FxSendChorus360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<FlangerEffect>());
}
