#include "FxSendReverb.h"
#include "FxSend.h"

FxSendReverb360::FxSendReverb360() : FxSend360(this) {}

FxSendReverb360::~FxSendReverb360() {}

void FxSendReverb360::Recreate(std::vector<FxSend *> &sends) { Refresh(sends); }

void FxSendReverb360::UpdateMix() { UpdateVolumes(); }

void FxSendReverb360::OnParametersChanged() { FxSend360::SyncEffectParams(); }
