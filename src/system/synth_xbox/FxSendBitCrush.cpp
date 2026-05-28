#include "FxSendBitCrush.h"
#include "FxSend.h"
#include "dsp/BitCrushEffect.h"
#include "dsp/StandardEffect.h"
#include "xdk/XAUDIO2.h"

FxSendBitCrush360::FxSendBitCrush360() : FxSend360(this) {}

FxSendBitCrush360::~FxSendBitCrush360() {}

IUnknown *FxSendBitCrush360::CreateFx() {
    return static_cast<CXAPOBase *>(new StandardEffect<BitCrushEffect>());
}
