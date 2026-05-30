#include "synth360/GainEffect.h"

float GainEffect::sGain = 1;

GainEffect::GainEffect() {
    GainEffectParams p;
    SetParameters(&p, sizeof(p));
}
