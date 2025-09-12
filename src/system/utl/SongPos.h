#pragma once

// size 0x18
class SongPos {
private:
    float mTotalTick; // 0x0
    float mTotalBeat; // 0x4
    int mPhrase; // 0x8
    int mMeasure; // 0xc
    int mBeat; // 0x10
    int mTick; // 0x14
public:
    SongPos()
        : mTotalTick(0), mTotalBeat(0), mPhrase(0), mMeasure(0), mBeat(0), mTick(0) {}
    float GetTotalTick() const { return mTotalTick; }
    float GetTotalBeat() const { return mTotalBeat; }
    int GetMeasure() const { return mMeasure; }
    int GetBeat() const { return mBeat; }
    int GetTick() const { return mTick; }
    int GetPhrase() const { return mPhrase; }

    int &AccessMeasure() { return mMeasure; }
    int &AccessBeat() { return mBeat; }
    int &AccessTick() { return mTick; }
    int &AccessPhrase() { return mPhrase; }
    float &AccessTotalTick() { return mTotalTick; }
    float &AccessTotalBeat() { return mTotalBeat; }
};
