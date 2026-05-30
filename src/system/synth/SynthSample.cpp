#include "synth/SynthSample.h"
#include "os/Memory.h"
#include "SampleData.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Platform.h"
#include "synth/SampleInst.h"
#include "utl/BinStream.h"
#include "utl/BufStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

FileLoader *SynthSample::sLoader = nullptr;
SynthSample *SynthSample::sLoading = nullptr;
bool sDisabled = false;

void *SampleAlloc(int size, const char *, int, const char *, int) {
    return MemAlloc(size, __FILE__, 0x1C, "Sample Data");
}

#pragma region SynthSample

SynthSample::SynthSample() {}

SynthSample::~SynthSample() {
    FOREACH (it, mSampleInsts) {
        (*it)->Stop(true);
    }
    if (sLoading == this) {
        RELEASE(sLoader);
        sLoading = nullptr;
    }
}

BEGIN_HANDLERS(SynthSample)
    HANDLE_EXPR(platform_size_kb, GetPlatformSize(kPlatformNone) / 1024)
    HANDLE_EXPR(num_markers, NumMarkers())
    HANDLE_EXPR(marker_name, mSampleData.GetMarker(_msg->Int(2)).Name())
    HANDLE_EXPR(marker_sample, mSampleData.GetMarker(_msg->Int(2)).Sample())
    HANDLE_EXPR(sample_length, LengthMs() / 1000.0f)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(SampleMarker)
    SYNC_PROP(sample, o.sample)
    SYNC_PROP(name, o.name)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(SynthSample)
    SYNC_PROP_MODIFY(file, mFile, Sync(sync0))
    SYNC_PROP_SET(
        sample_rate,
        mSampleData.GetSampleRate(),
        MILO_NOTIFY("can't set property %s", "sample_rate")
    )
    SYNC_PROP(markers, mSampleData.AccessMarkers())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(SynthSample)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mFile;
    if (bs.Cached()) {
        mSampleData.Save(bs);
    }
END_SAVES

BEGIN_COPYS(SynthSample)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(SynthSample)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mFile)
        }
    END_COPYING_MEMBERS
    Sync(sync0);
END_COPYS

BEGIN_LOADS(SynthSample)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(6, 0)

void SynthSample::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(6, 0)
    if (d.rev > 1) {
        Hmx::Object::Load(bs);
    }
    d >> mFile;
    if (d.rev <= 5) {
        bool b;
        int x, y;
        d >> b >> x;
        if (d.rev >= 3) {
            d >> y;
        }
    }
    if (bs.Cached() && d.rev >= 5) {
        mSampleData.Load(bs, mFile);
    } else if (d.rev > 3 && !sDisabled) {
        sLoader = dynamic_cast<FileLoader *>(TheLoadMgr.AddLoader(mFile, kLoadFront));
        sLoading = this;
    }
}

void SynthSample::PostLoad(BinStream &bs) {
    sLoader = nullptr;
    sLoading = nullptr;
    Sync(bs.Cached() ? sync1 : sync0);
}

void SynthSample::Disable() { sDisabled = true; }
int SynthSample::GetNumChannels() const { return mSampleData.NumChannels(); }
std::vector<SampleMarker> &SynthSample::AccessMarkers() {
    return mSampleData.AccessMarkers();
}
int SynthSample::NumMarkers() const { return mSampleData.NumMarkers(); }
int SynthSample::GetPlatformSize(Platform) {
    return mSampleData.SizeAs(SampleData::kPCM);
}

void SynthSample::Sync(SyncType ty) {
    if (ty == sync0) {
        mSampleData.Reset();
        if (!sDisabled && !mFile.empty()) {
            FileLoader *fl = dynamic_cast<FileLoader *>(TheLoadMgr.ForceGetLoader(mFile));
            int i80;
            const char *cc;
            if (fl) {
                cc = fl->GetBuffer(&i80);
            } else
                cc = nullptr;
            delete fl;
            if (cc) {
                BufStream bs((void *)cc, i80, true);
                if (TheLoadMgr.GetPlatform() == kPlatformPC) {
                    mSampleData.LoadWAV(bs, mFile, false);
                } else {
                    mSampleData.Load(bs, mFile);
                }
                delete cc;
            }
        }
    }
}

void SynthSample::RegisterChild(SampleInst *inst) { mSampleInsts.push_back(inst); }

void SynthSample::UnregisterChild(SampleInst *inst) {
    FOREACH (it, mSampleInsts) {
        if (*it == inst) {
            mSampleInsts.erase(it);
            return;
        }
    }
    MILO_NOTIFY("Could not find child instance for unregistration!");
}

void SynthSample::Init() {
    REGISTER_OBJ_FACTORY(SynthSample);
    SampleData::SetAllocator(SampleAlloc, MemFree);
}

int SynthSample::GetSampleRate() const { return mSampleData.GetSampleRate(); }

#pragma endregion
