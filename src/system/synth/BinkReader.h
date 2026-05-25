#pragma once
#include "os/File.h"
#include "synth/StandardStream.h"
#include "binkxenon/bink.h"
#include "synth/StreamReader.h"

#define BINK_AUDIO_CHANNEL_MAX 16

enum BinkReaderState {
    kOpenBink = 0,
    kOpenTracks = 1,
    kInitStream = 2,
    kPlay = 3,
    kDone = 4,
    kFailure = 5,
};

class BinkReader : public StreamReader {
public:
    BinkReader(File *, StandardStream *);
    virtual ~BinkReader();
    virtual void Poll(float);
    virtual void Seek(int);
    virtual void EnableReads(bool) {}
    virtual bool Done() { return mState == kDone; }
    virtual bool Fail() { return mState == kFailure; }

protected:
    virtual void Init();

private:
    static int sHeap;

    File *mFile; // 0x4
    StandardStream *mStream; // 0x8
    BINK *mBink; // 0xc
    BINKTRACK *mBinkTracks[BINK_AUDIO_CHANNEL_MAX]; // 0x10
    unsigned char *mPCMBuffers[BINK_AUDIO_CHANNEL_MAX]; // 0x50
    unsigned char *mPCMOffsets[BINK_AUDIO_CHANNEL_MAX]; // 0x90
    unsigned char mDecodeTrack; // 0xd0
    int mSamplesReady; // 0xd4
    unsigned int mSampleCurrent; // 0xd8
    unsigned int mSamplesJump; // 0xdc
    BinkReaderState mState; // 0xe0
    int mHeap; // 0xe4
};
