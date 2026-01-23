#ifndef UTL_WAVEFILE_H
#define UTL_WAVEFILE_H

#include "utl/BinStream.h"
#include "utl/Chunks.h"
#include "synth/SampleData.h"
#include <vector>

class WaveFileMarker {
public:
    WaveFileMarker(int frame, int id, const String &name)
        : mFrame(frame), mID(id), mName(name) {} // total size: 0x14
    int mFrame; // offset 0x0, size 0x4
    int mID; // offset 0x4, size 0x4
    class String mName; // offset 0x8, size 0xC
};

class WaveFile {
    friend class WaveFileData;

private:
    void ReadFormat();
    void ReadMarkers();
    void ReadNumSamples();
    IListChunk &PrepareToProvideData();

    short mFormat; // 0x0
    unsigned short mNumChannels; // 0x2
    unsigned int mSamplesPerSec; // 0x4
    unsigned int mAvgBytesPerSec; // 0x8
    unsigned short mBlockAlign; // 0xC
    unsigned short mBitsPerSample; // 0xE
    int mNumSamples; // 0x10
    std::vector<WaveFileMarker> mMarkers; // 0x14
    IListChunk mRiffList; // 0x20

public:
    WaveFile(BinStream &);
    ~WaveFile();

    unsigned short NumChannels() const { return mNumChannels; }
    unsigned short BitsPerSample() const { return mBitsPerSample; }
    unsigned int SamplesPerSec() const { return mSamplesPerSec; }
    int NumSamples() const { return mNumSamples; }
    short Format() const { return mFormat; }
    int NumMarkers() const { return mMarkers.size(); }
    std::vector<WaveFileMarker> &Markers() { return mMarkers; }
};

class WaveFileData : public IDataChunk {
public:
    WaveFileData(WaveFile &);

    virtual ~WaveFileData();

    WaveFile *mWaveFile;
};

#endif
