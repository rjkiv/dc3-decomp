#include "synth/BinkReader.h"
#include "bink.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "synth/StandardStream.h"
#include "utl/BinkIntegration.h"
#include "utl/MemMgr.h"

int BinkReader::sHeap = 0;

BinkReader::BinkReader(File *f, StandardStream *s)
    : mFile(f), mStream(s), mDecodeTrack(false), mSamplesReady(0), mSampleCurrent(0),
      mSamplesJump(0), mState(kOpenBink), mHeap(sHeap) {
    BinkInit();
    BinkSetSoundTrack(0, nullptr);
    mBink = BinkOpen(
        (const char *)f,
        BINKIOPROCESSOR | BINKFILEHANDLE | BINKSNDTRACK | BINKNOFRAMEBUFFERS
    );
    if (mBink) {
        mState = kOpenTracks;
        BinkSetVideoOnOff(mBink, 0);
    } else {
        MILO_NOTIFY("Error opening Bink audio file: %s", BinkGetError());
        mState = kFailure;
    }
}

BinkReader::~BinkReader() {
    if (mState > kOpenTracks) {
        for (unsigned char i = 0; i < mBink->NumTracks; i++) {
            if (mBinkTracks[i]) {
                BinkCloseTrack(mBinkTracks[i]);
            }
            if (mPCMBuffers[i]) {
                MemFree(mPCMBuffers[i]);
            }
        }
    }
    BinkClose(mBink);
}

void BinkReader::Poll(float f1) {
    START_AUTO_TIMER("bink_audio");
    switch (mState) {
    case kOpenTracks: {
        MILO_ASSERT(mBink->NumTracks < BINK_AUDIO_CHANNEL_MAX, 0x5E);
        if (mBink->NumTracks == 0) {
            mState = kDone;
        }
        for (unsigned char i = 0; i < mBink->NumTracks; i++) {
            HBINKTRACK hBinkTrack = BinkOpenTrack(mBink, i);
            mBinkTracks[i] = hBinkTrack;
            MILO_ASSERT(hBinkTrack->Bits == 16, 0x73);
            MILO_ASSERT(hBinkTrack->Frequency == 44100, 0x74);
            MILO_ASSERT(hBinkTrack->Channels == 1, 0x75);
            unsigned char *mem = (unsigned char *)MemAlloc(
                hBinkTrack->MaxSize, __FILE__, 0x78, "Bink Audio", 0x80
            );
            mPCMBuffers[i] = mem;
            mPCMOffsets[i] = mem;
        }
        mState = kInitStream;
        break;
    }
    case kInitStream: {
        mState = kPlay;
        Init();
        break;
    }
    case kPlay: {
        TheBlockMgr.Poll();
        if (mSamplesReady > 0) {
            int iSamplesConsumed =
                mStream->ConsumeData((void **)mPCMOffsets, mSamplesReady, mSampleCurrent);
            MILO_ASSERT(iSamplesConsumed <= mSamplesReady, 0x9B);
            mSampleCurrent += iSamplesConsumed;
            mSamplesReady -= iSamplesConsumed;
            for (unsigned char i = 0; i < mBink->NumTracks; i++) {
                mPCMOffsets[i] += iSamplesConsumed * 2;
            }
            if (mDecodeTrack == mBink->NumTracks) {
                mState = mBink->FrameNum == mBink->Frames ? kDone : kPlay;
                if (mState == kPlay) {
                    BinkNextFrame(mBink);
                }
                mDecodeTrack = 0;
            }
        }
        if (mSamplesReady <= 0) {
            int i2 = 0;
            int i9 = 0xb400;
            do {
                if (mDecodeTrack == mBink->NumTracks)
                    break;
                i2 = BinkGetTrackData(
                    mBinkTracks[mDecodeTrack], mPCMBuffers[mDecodeTrack]
                );
                i9 = i9 - i2;
                mPCMOffsets[mDecodeTrack] = mPCMBuffers[mDecodeTrack] + mSamplesJump * 2;
                mDecodeTrack++;
            } while (i9 > 0);
            if (mDecodeTrack == mBink->NumTracks) {
                unsigned int oldJump = mSamplesJump;
                mSamplesJump = 0;
                mSamplesReady = (i2 / (sizeof(unsigned char) * 2)) - oldJump;
                mSampleCurrent += oldJump;
                mState = mBink->FrameNum == mBink->Frames ? kDone : kPlay;
                if (i9 > 0 && mState == kPlay) {
                    BinkNextFrame(mBink);
                    mDecodeTrack = 0;
                }
            }
        }
        break;
    }
    case kFailure: {
        MILO_FAIL("BinkReader::Poll() failed!");
        break;
    }
    default:
        break;
    }
    if (mBink->ReadError != 0) {
        mState = kFailure;
    }
}

void BinkReader::Seek(int i1) {
    if (mBink && mState != kFailure) {
        double kfBinkFreq = mBinkTracks[0]->Frequency;
        double kfBinkRate = (double)mBink->FrameRate / (double)mBink->FrameRateDiv;
        int kiSampleFrame = ((double)i1 / kfBinkFreq - 0.75) * kfBinkRate + 1.0;
        MILO_ASSERT(kiSampleFrame < mBink->Frames, 0x102);
        BinkGoto(mBink, kiSampleFrame, 1);
        // more...
        MILO_ASSERT(mSamplesJump < (kfBinkFreq / kfBinkRate), 0x10B);
        mState = kPlay;
    }
}

void BinkReader::Init() {
    MILO_ASSERT(mStream, 0x114);
    mStream->InitInfo(mBink->NumTracks, mBinkTracks[0]->Frequency, false, -1);
}
