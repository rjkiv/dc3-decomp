#pragma once

// size 0x18
class SongPos {
public:
    SongPos(
        float totalTick = 0,
        float totalBeat = 0,
        int phrase = 0,
        int m = 0,
        int b = 0,
        int t = 0
    )
        : mTotalTick(totalTick), mTotalBeat(totalBeat), mPhrase(phrase), mMeasure(m),
          mBeat(b), mTick(t) {}
    float GetTotalTick() const { return mTotalTick; }
    float GetTotalBeat() const { return mTotalBeat; }
    int GetMeasure() const { return mMeasure; }
    int GetBeat() const { return mBeat; }
    int GetTick() const { return mTick; }
    int GetPhrase() const { return mPhrase; }

    float mTotalTick; // 0x0
    float mTotalBeat; // 0x4
    int mPhrase; // 0x8
    int mMeasure; // 0xc
    int mBeat; // 0x10
    int mTick; // 0x14
};
