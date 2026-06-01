#include "FxSendChorus.h"
#include "FxSend.h"
#include "dsp/FlangerEffect.h"
#include "dsp/StandardEffect.h"

FxSendChorus360::FxSendChorus360() : FxSend360(this) {}

FxSendChorus360::~FxSendChorus360() {}

IUnknown *FxSendChorus360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<FlangerEffect>());
}
