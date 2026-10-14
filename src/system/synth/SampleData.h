#pragma once
#include "obj/Data.h"
#include "obj/PropSync.h"
#include "utl/CRC.h"
#include "utl/Str.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include <vector>

class SampleMarker {
    friend bool PropSync(SampleMarker &, DataNode &, DataArray *, int, PropOp);

public:
    SampleMarker() : name(""), sample(-1) {}
    SampleMarker(const String &str, int i) : name(str), sample(i) {}
    void Save(BinStream &bs) const {
        bs << name;
        bs << sample;
    }
    void Load(BinStream &bs) {
        bs >> name;
        bs >> sample;
    }
    int Sample() const { return sample; }
    const String &Name() const { return name; }

private:
    String name; // 0x0
    int sample; // 0x8
};

typedef void *(*SampleDataAllocFunc)(int, const char *, int, const char *, int);
typedef void (*SampleDataFreeFunc)(void *, const char *, int, const char *);

class SampleData {
public:
    enum Format {
        kPCM = 0x0000,
        kBigEndPCM = 0x0001,
        kVAG = 0x0002,
        kXMA = 0x0003,
        kATRAC = 0x0004,
        kMP3 = 0x0005,
        kNintendoADPCM = 0x0006,
        kNintendoBcWav = 0x0007
    };

    SampleData();
    ~SampleData();
    void Reset();
    void Save(BinStream &) const;
    void Load(BinStream &, const FilePath &);
    void LoadWAV(BinStream &, const FilePath &, bool);
    int SizeAs(Format) const;
    int NumMarkers() const;
    const SampleMarker &GetMarker(int) const;
    void Dealloc();
    int Channels() const { return mChannels; }
    int SampleRate() const { return mSampleRate; }
    int NumSamples() const { return mNumSamples; }
    int SizeBytes() const { return mSizeBytes; }
    Format GetFormat() const { return mFormat; }
    const void *Data() const { return mData; }
    std::vector<SampleMarker> &AccessMarkers() { return mMarkers; }

    static void SetAllocator(SampleDataAllocFunc, SampleDataFreeFunc);

private:
    static SampleDataAllocFunc sAlloc;
    static SampleDataFreeFunc sFree;

    int mCRCName; // 0x0
    int mNumSamples; // 0x4
    int mSampleRate; // 0x8
    int mChannels; // 0xc
    int mSizeBytes; // 0x10
    Format mFormat; // 0x14
    void *mData; // 0x18
    std::vector<SampleMarker> mMarkers; // 0x1c
};
