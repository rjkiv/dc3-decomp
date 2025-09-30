#include "meta/StreamPlayer.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "synth/Stream.h"
#include "synth/Synth.h"

StreamPlayer::StreamPlayer()
    : mMasterVol(1.0f), mStreamVol(1.0f), mLoop(0), mStarted(0), mPaused(0), mStream(0) {}

void StreamPlayer::StopPlaying() { Delete(); }

StreamPlayer::~StreamPlayer() { Delete(); }

void StreamPlayer::PlayFile(char const *cc, float f1, float f2, bool b) {
    Delete();
    mStream = TheSynth->NewStream(cc, 0.0f, 0.0f, false);
    MILO_ASSERT(mStream, 0x2c);
    mStarted = false;
    mLoop = b;
    mStreamVol = f1;
}

void StreamPlayer::Poll() {
    if (!mStream || mPaused) {
        return;
    } else {
        if (!mStream->IsPlaying()) {
            if (mStream->IsReady()) {
                if (!mStarted) {
                    Init();
                    mStarted = true;
                }
                mStream->Play();
            } else if (mStarted) {
                mStarted = false;
                Delete();
            }
        }
    }
}

void StreamPlayer::Delete() {
    if (mStream)
        mStream->Stop();
    delete mStream;
    mStream = 0;
}

void StreamPlayer::Init() {
    mStream->SetVolume(mStreamVol * mMasterVol);
    MILO_ASSERT(mStream->GetNumChannels() == 2, 0x68);
    mStream->SetPan(0, -1.0f);
    mStream->SetPan(1, 1.0f);
    if (mLoop) {
        mStream->SetJump(-0.25, 0, 0);
    }
}

void StreamPlayer::SetVolume(float value) {
    mMasterVol = value;
    if (mStream) {
        mStream->SetVolume(value * mStreamVol);
    }
}

BEGIN_HANDLERS(StreamPlayer)
    HANDLE_ACTION(set_volume, SetVolume(_msg->Float(2)))
END_HANDLERS
