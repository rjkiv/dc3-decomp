#pragma once
#include "utl/Symbol.h"
#include <list>
#include <vector>

class MidiChannel {
public:
    // got this from DC1
    struct DummySample {};

    struct Note {
        int mNoteNum; // 0x0
        float mVolume; // 0x4
        std::vector<DummySample *> mSamples; // 0x8
        std::vector<float> mTransposes; // 0x14
    };

    MidiChannel();

private:
    int mInst; // 0x0
    int mBank; // 0x4
    float mVolume; // 0x8
    float mPan; // 0xc
    float mTranspose; // 0x10
    std::list<Note> mPlayingNotes; // 0x14
};
