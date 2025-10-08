#pragma once
#include "synth/Stream.h"

class HxAudio {
public:
    HxAudio() {}
    virtual ~HxAudio() {}
    virtual bool IsReady() = 0;
    virtual bool Paused() const = 0;
    virtual void SetPaused(bool) = 0;
    virtual void Poll() = 0;
    virtual float GetTime() const = 0;
    virtual Stream *GetSongStream() = 0;
    virtual void SetMasterVolume(float) = 0;
};
