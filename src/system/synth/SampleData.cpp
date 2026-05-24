#include "synth/SampleData.h"
#include "obj/Object.h"
#include "os/File.h"
#include "synth/WavMgr.h"
#include "utl/BinStream.h"
#include "utl/CRC.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"

SampleDataAllocFunc SampleData::sAlloc = nullptr;
SampleDataFreeFunc SampleData::sFree = nullptr;

SampleData::SampleData() : mData(0), mMarkers() { Reset(); }
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
    mNumChannels = 1;
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
    bs << mCRC;
    bs << mFormat;
    bs << mNumSamples;
    bs << mSampleRate;
    bs << mSizeBytes;
    bool hasData = mData;
    bs << hasData;
    if (hasData) {
        WriteChunks(bs, mData, mSizeBytes, 0x8000);
    }
    bs << mMarkers;
    bs << mNumChannels;
}

BinStreamRev &operator>>(BinStreamRev &d, SampleMarker &s) {
    s.Load(d.stream);
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
        d >> (int &)mCRC;
    } else {
        mCRC = Hmx::CRC(FileRelativePath(FileExecRoot(), fp.c_str()));
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
        if (mCRC) {
            TheWavMgr->CreateSample(mCRC, mData, mSizeBytes);
        } else {
            mData = sAlloc(mSizeBytes, __FILE__, 0x6F, "SampleData", 0);
        }
        ReadChunks(bs, mData, mSizeBytes, 0x8000);
    }
    if (d.rev >= 0xE) {
        d >> mMarkers;
    }
    if (d.rev >= 0x10) {
        d >> mNumChannels;
    }
}

void SampleData::Dealloc() {
    if (mCRC && !TheWavMgr->ReleaseRes(mCRC)) {
        sFree(mData, __FILE__, 0xC4, "SampleData");
    }
    mData = nullptr;
    (int &)mCRC = 0;
}
