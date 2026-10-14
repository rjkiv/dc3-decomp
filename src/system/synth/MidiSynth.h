#pragma once
#include "synth/MidiChannel.h"

class MidiSynth {
public:
    MidiSynth();

private:
    std::vector<MidiChannel> mChannels; // 0x0
};
