#include "FxSendMeterEffect.h"
#include "FxSend.h"
#include "macros.h"

FxSendMeterEffect360::FxSendMeterEffect360() : FxSend360(this), unkb0(0) {}

FxSendMeterEffect360::~FxSendMeterEffect360() { RELEASE(unkb0); }
