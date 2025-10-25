#include "lazer/game/Shuttle.h"
#include "math/Utl.h"
#include "os/Joypad.h"
#include "types.h"
#include <cmath>

Shuttle::Shuttle() : unk0(0.0f), unk4(0.0f), unk8(false), unkc(0) {}

void Shuttle::SetActive(bool b) { unk8 = b; }

void Shuttle::Poll() {
    if (unk8) {
        JoypadData *data = JoypadGetPadData(unkc);
        if (data) {
            float sticks = data->mSticks[0][0];
            float powF = pow(sticks, 5);
            powF *= 1000.0f;
            float f = unk0 + powF;
            unk0 = Clamp<float>(0, unk4, f);
        }
    }
}
