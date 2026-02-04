#include "synth/FxSend.h"
#include "Sfx.h"
#include "math/Decibels.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "synth/Synth.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

FxSend::FxSend()
    : mNextSend(this), mStage(0), mBypass(0), mDryGain(kDbSilence), mWetGain(0),
      mInputGain(0), mReverbMixDb(kDbSilence), mReverbEnable(0), mEnableUpdates(1),
      mChannels(kSendAll) {}

bool FxSend::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mNextSend) {
        mNextSend.SetObj(to);
        RebuildChain();
        return true;
    } else
        return Hmx::Object::Replace(from, to);
}

BEGIN_HANDLERS(FxSend)
    HANDLE_ACTION(test_with_mic, TestWithMic())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(FxSend)
    SYNC_PROP_SET(next_send, NextSend(), SetNextSend(_val.Obj<FxSend>()))
    SYNC_PROP_SET(stage, Stage(), SetStage(_val.Int()))
    SYNC_PROP_MODIFY(dry_gain, mDryGain, UpdateMix())
    SYNC_PROP_MODIFY(wet_gain, mWetGain, UpdateMix())
    SYNC_PROP_MODIFY(input_gain, mInputGain, UpdateMix())
    SYNC_PROP_MODIFY(reverb_mix_db, mReverbMixDb, UpdateMix())
    SYNC_PROP_MODIFY(reverb_enable, mReverbEnable, RebuildChain())
    SYNC_PROP_MODIFY(channels, (int &)mChannels, RebuildChain())
    SYNC_PROP_MODIFY(bypass, mBypass, UpdateMix())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(FxSend)
    SAVE_REVS(8, 7)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mNextSend;
    bs << mStage;
    bs << mChannels;
    bs << mDryGain;
    bs << mWetGain;
    bs << mInputGain;
    bs << mBypass;
    bs << mReverbMixDb;
    bs << mReverbEnable;
END_SAVES

BEGIN_COPYS(FxSend)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FxSend)
    BEGIN_COPYING_MEMBERS
        mNextSend.SetObj(c->mNextSend);
        COPY_MEMBER(mStage)
        COPY_MEMBER(mWetGain)
        COPY_MEMBER(mDryGain)
        COPY_MEMBER(mInputGain)
        COPY_MEMBER(mChannels)
        COPY_MEMBER(mBypass)
        COPY_MEMBER(mReverbMixDb)
        COPY_MEMBER(mReverbEnable)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(FxSend)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    Hmx::Object::Load(bs);
    bs >> mNextSend;
    bs >> mStage;
    if (d.rev < 5) {
        if (d.rev >= 2) {
            float x;
            d >> x;
            mDryGain = RatioToDb((100.0f - x) / 100.0f);
            mWetGain = RatioToDb(x / 100.0f);
        }
        if (d.rev >= 3) {
            d >> mBypass;
        }
    }
    if (d.rev >= 4) {
        int channels;
        d >> channels;
        mChannels = (SendChannels)channels;
    }
    if (d.rev >= 5) {
        d >> mDryGain;
        d >> mWetGain;
        d >> mInputGain;
    }
    if (d.rev >= 6) {
        d >> mBypass;
    }
    if (d.rev >= 7) {
        d >> mReverbMixDb;
        d >> mReverbEnable;
    }
    RebuildChain();
    UpdateMix();
END_LOADS

void FxSend::SetNextSend(FxSend *next) {
    if (next != mNextSend && CheckChain(next, mStage)) {
        mNextSend = next;
        RebuildChain();
    }
}

void FxSend::RebuildChain() {
    std::vector<FxSend *> vec;
    BuildChainVector(vec);
    Recreate(vec);
}

void FxSend::BuildChainVector(std::vector<FxSend *> &sends) {
    sends.push_back(this);
    FOREACH (it, Refs()) {
        FxSend *send = dynamic_cast<FxSend *>(it->RefOwner());
        if (send && send->mNextSend == this) {
            send->BuildChainVector(sends);
        } else {
            Sfx *sfx = dynamic_cast<Sfx *>(it->RefOwner());
            if (sfx) {
                sfx->Stop(false);
            }
        }
    }
}

void FxSend::SetChannels(SendChannels chans) {
    if (chans != mChannels) {
        mChannels = chans;
        RebuildChain();
    }
}

void FxSend::EnableUpdates(bool enable) {
    mEnableUpdates = enable;
    if (mEnableUpdates)
        OnParametersChanged();
}

bool FxSend::CheckChain(FxSend *send, int i) {
    FxSend *cur;
    for (cur = send; cur && cur != this; cur = cur->mNextSend)
        ;
    if (cur == this) {
        MILO_NOTIFY("Error: can't have loops in your FX chain.");
        return false;
    } else if (send && send->Stage() <= i) {
        MILO_NOTIFY(
            "Error: output send must be set to a higher stage (%d <= %d).",
            send->Stage(),
            i
        );
        return false;
    } else {
        for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
            FxSend *rsend = dynamic_cast<FxSend *>((*it).RefOwner());
            if (rsend && rsend->NextSend() == this && rsend->Stage() >= i) {
                MILO_NOTIFY(
                    "Error: stage must be higher than all input sends' stages (see %s).",
                    rsend->Name()
                );
                return false;
            }
        }
        return true;
    }
}

void FxSend::SetStage(int stage) {
    if (stage != mStage && CheckChain(mNextSend, stage)) {
        mStage = stage;
        RebuildChain();
    }
}

void FxSend::TestWithMic() {
    if (TheSynth->GetNumMics() == 0) {
        MILO_NOTIFY("There are no mics to test with.");
    } else {
        MILO_ASSERT(TheLoadMgr.EditMode(), 0x10A);
        Mic *mic = TheSynth->GetMic(0);
        mic->Start();
        mic->StartPlayback();
        mic->SetFxSend(this);
    }
}
