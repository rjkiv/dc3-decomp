#include "lazer/game/Shuttle.h"
#include "math/Utl.h"
#include "os/Joypad.h"
#include "types.h"
#include <cmath>

Shuttle::Shuttle() : mMs(0), mEndMs(0), mActive(0), mController(0) {}
Shuttle::~Shuttle() {}

void Shuttle::SetActive(bool b) { mActive = b; }

void Shuttle::Poll() {
    if (mActive) {
        JoypadData *data = JoypadGetPadData(mController);
        if (data) {
            float sticks = data->mSticks[0][0];
            float powF = pow(sticks, 5);
            powF *= 1000.0f;
            float f = mMs + powF;
            mMs = Clamp<float>(0, mEndMs, f);
        }
    }
}
