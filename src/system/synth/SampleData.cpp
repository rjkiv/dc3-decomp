#include "synth/SampleData.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "synth/WavMgr.h"
#include "utl/BinStream.h"
#include "utl/CRC.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/WaveFile.h"

SampleDataAllocFunc SampleData::sAlloc = nullptr;
SampleDataFreeFunc SampleData::sFree = nullptr;

SampleData::SampleData() : mCRCName(0), mData(0), mMarkers() { Reset(); }
SampleData::~SampleData() { Dealloc(); }

void SampleData::SetAllocator(SampleDataAllocFunc a, SampleDataFreeFunc f) {
    sAlloc = a;
    sFree = f;
    TheWavMgr->SetAllocator((WavMgrAllocFunc)a, (WavMgrFreeFunc)f);
}

void SampleData::Reset() {
    Dealloc();
    mFormat = kPCM;
    mSizeBytes = 0;
    mSampleRate = 0;
    mNumSamples = 0;
    mChannels = 1;
    mMarkers.clear();
}

int SampleData::NumMarkers() const { return mMarkers.size(); }

const SampleMarker &SampleData::GetMarker(int idx) const { return mMarkers[idx]; }

BinStream &operator<<(BinStream &bs, const SampleMarker &s) {
    s.Save(bs);
    return bs;
}

void SampleData::Save(BinStream &bs) const {
    SAVE_REVS(0x10, 0);
    bs << mCRCName;
    bs << mFormat << mNumSamples << mSampleRate << mSizeBytes;
    bool hasData = mData;
    bs << hasData;
    if (hasData) {
        WriteChunks(bs, mData, mSizeBytes, 0x8000);
    }
    bs << mMarkers;
    bs << mChannels;
}

BinStreamRev &operator>>(BinStreamRev &d, SampleMarker &s) {
    s.Load(d);
    return d;
}

INIT_REVS(0x10, 0)

void SampleData::Load(BinStream &bs, const FilePath &fp) {
    Reset();
    LOAD_REVS(bs)
    if (d.rev > 0x10) {
        MILO_FAIL("%s can't load new %s version %d > %d", fp, "SampleData", d.rev, gRev);
    }
    if (d.altRev > 0) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d", fp, "SampleData", d.altRev, gAltRev
        );
    }
    if (d.rev > 0xE) {
        d >> mCRCName;
    } else {
        mCRCName = Hmx::CRC(FileRelativePath(FileExecRoot(), fp.c_str()));
    }
    int fmt;
    d >> fmt >> mNumSamples >> mSampleRate;
    d >> mSizeBytes;
    bool b70 = true;
    mFormat = (Format)fmt;
    if (d.rev >= 11) {
        d >> b70;
    }
    if (b70) {
        if (mCRCName) {
            TheWavMgr->CreateSample(mCRCName, mData, mSizeBytes);
        } else {
            mData = sAlloc(mSizeBytes, __FILE__, 0x6F, "SampleData", 0);
        }
        ReadChunks(bs, mData, mSizeBytes, 0x8000);
    }
    if (d.rev >= 0xE) {
        d >> mMarkers;
    }
    if (d.rev >= 0x10) {
        d >> mChannels;
    }
}

void SampleData::LoadWAV(BinStream &bs, const FilePath &fp, bool b3) {
    Reset();
    WaveFile waveFile(bs);
    if (waveFile.BitsPerSample() != 16) {
        MILO_NOTIFY("Wave file %s is not 16-bit", fp);
        return;
    } else if (waveFile.Format() != 1) {
        MILO_NOTIFY("Wave file %s is compressed", fp);
        return;
    } else {
        if (!b3) {
            mCRCName = Hmx::CRC(FileRelativePath(FileExecRoot(), fp.c_str()));
        } else {
            mCRCName = Hmx::CRC();
        }
        mFormat = kPCM;
        mChannels = waveFile.NumChannels();
        mNumSamples = waveFile.NumSamples();
        mSampleRate = waveFile.SamplesPerSec();
        mSizeBytes = SizeAs(kPCM);
        if (mCRCName) {
            if (!TheWavMgr->CreateSample(mCRCName, mData, mSizeBytes)) {
                WaveFileData data(waveFile);
                data.Read(mData, mSizeBytes);
            }
        } else {
            mData = sAlloc(mSizeBytes, __FILE__, 0xA5, "SampleData", 0);
            WaveFileData data(waveFile);
            data.Read(mData, mSizeBytes);
        }
        for (int i = 0; i < waveFile.NumMarkers(); i++) {
            mMarkers.push_back(SampleMarker(
                waveFile.Markers()[i].GetName(), waveFile.Markers()[i].GetFrame()
            ));
        }
    }
}

void SampleData::Dealloc() {
    if (!mCRCName || !TheWavMgr->ReleaseRes(mCRCName)) {
        sFree(mData, __FILE__, 0xC4, "SampleData");
    }
    mData = nullptr;
    mCRCName = 0;
}

int SampleData::SizeAs(Format f) const {
    switch (f) {
    case kPCM:
    case kBigEndPCM:
        return mChannels * mNumSamples * 2;
    case kVAG:
        return ((mNumSamples + 0x6f) / 0x70) * mChannels * 64;
    case kATRAC:
        return ((mNumSamples + 0x3ff) / 1024) * mChannels * 0xc0;
    case kXMA:
        MILO_NOTIFY("don't know size as XMA");
        return mNumSamples / 5;
    case kMP3:
        return ((mNumSamples + 0x3ff) / 1024) * mChannels * 0xc0;
    case kNintendoBcWav: {
        int ret = ((mChannels * mNumSamples) * 2) / 3.4f;
        return ret + 0x60;
    }
    case kNintendoADPCM: {
        int ret = ((mChannels * mNumSamples) * 2) / 3.4f;
        return ret + 0x60;
    }
    default:
        MILO_ASSERT(0, 299);
        return 0;
    }
}
