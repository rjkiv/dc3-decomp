#pragma once

/** The map of tempos for the current song. */
#include "utl/MemMgr.h"
class TempoMap {
public:
    TempoMap() {}
    virtual ~TempoMap() {}
    virtual float TickToTime(float tick) const = 0;
    virtual float TimeToTick(float time) const = 0;
    virtual float GetTempo(int tick) const = 0;
    virtual int GetTempoInMicroseconds(int tick) const = 0;
    virtual float GetTempoBPM(int tick) const = 0;
    virtual int GetNumTempoChangePoints() const = 0;
    virtual int GetTempoChangePoint(int index) const = 0;
    virtual void Finalize() {}
    virtual void ClearLoopPoints() = 0;
    virtual void SetLoopPoints(int start, int end) = 0;
    virtual int GetLoopTick(int tick, int &tickOffset) const = 0;
    virtual int GetLoopTick(int tick) const = 0;
    virtual float GetTimeInLoop(float time) = 0;

    MEM_OVERLOAD(TempoMap, 0x13);
};

/** A tempomap with one single tempo for the whole song (i.e. an electronic song). */
class SimpleTempoMap : public TempoMap {
private:
    /** The tempo, in ms. */
    float mTempo; // 0x4
public:
    SimpleTempoMap(float f) { mTempo = f / 480.0f; }
    virtual ~SimpleTempoMap() {}
    virtual float TickToTime(float f) const { return f * mTempo; }
    virtual float TimeToTick(float f) const { return f / mTempo; }
    virtual float GetTempo(int) const { return 480.0f * mTempo; }
    virtual int GetTempoInMicroseconds(int i) const { return GetTempo(i) * 1000.0f; }
    virtual float GetTempoBPM(int) const { return 60000.0f / (480.0f * mTempo); }
    virtual int GetNumTempoChangePoints() const { return 1; }
    virtual int GetTempoChangePoint(int) const { return 0; }
    virtual void ClearLoopPoints() {}
    virtual void SetLoopPoints(int, int) {}
    virtual int GetLoopTick(int, int &) const { return 0; }
    virtual int GetLoopTick(int) const { return 0; }
    virtual float GetTimeInLoop(float time) { return time; }
};

void SetTheTempoMap(TempoMap *);
void ResetTheTempoMap();

extern TempoMap *TheTempoMap;
