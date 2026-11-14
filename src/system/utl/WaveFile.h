#pragma once
#include "utl/BinStream.h"
#include "utl/Chunks.h"
#include "synth/SampleData.h"
#include <vector>

class WaveFileMarker {
public:
    WaveFileMarker(int frame, int id, const String &name)
        : mFrame(frame), mID(id), mName(name) {} // total size: 0x14

    int GetFrame() const { return mFrame; }
    const String &GetName() const { return mName; }
    void SetName(const char *name) { mName = name; }

    //  public: void SetName(const char*);
    //  public: int32_t Size() const;
    //  public: int32_t TextSize() const;
    //  public: void WriteCue(BinStream&);
    //  public: void WriteLabel(BinStream&);

private:
    int mFrame; // offset 0x0, size 0x4
    int mID; // offset 0x4, size 0x4
    String mName; // offset 0x8, size 0xC
};

class WaveFile {
    friend class WaveFileData;

public:
    WaveFile(BinStream &);
    ~WaveFile();

    bool IsCompressed() const { return mFormat != 1; }
    int NumChannels() const { return mNumChannels; }
    int SamplesPerSec() const { return mSamplesPerSec; }
    //   public: int32_t AvgBytesPerSec() const;
    //   public: int32_t BlockAlign() const;
    int BitsPerSample() const { return mBitsPerSample; }
    //   public: int32_t DataSize() const;
    int NumSamples() const { return mNumSamples; }
    int NumMarkers() const { return mMarkers.size(); }
    const WaveFileMarker &GetMarker(int idx) const { return mMarkers[idx]; }
    void SetMarkerName(int idx, const char *name) { mMarkers[idx].SetName(name); }
    //   public: void Write(const char*);

private:
    void ReadFormat();
    void ReadMarkers();
    void ReadNumSamples();
    IListChunk &PrepareToProvideData();

    //   private: void ReadData();
    //   private: void WriteRiffHeader(BinStream&);
    //   private: void WriteFormatChunk(BinStream&);
    //   private: void WriteDataChunk(BinStream&);
    //   private: void WriteCueChunk(BinStream&);
    //   private: void WriteLablChunk(BinStream&);
    //   private: int32_t RiffSize() const;
    //   private: int32_t LablSize() const;
    //   private: int32_t CueChunkSize() const;

    short mFormat; // 0x0
    unsigned short mNumChannels; // 0x2
    unsigned int mSamplesPerSec; // 0x4
    unsigned int mAvgBytesPerSec; // 0x8
    unsigned short mBlockAlign; // 0xC
    unsigned short mBitsPerSample; // 0xE
    int mNumSamples; // 0x10
    std::vector<WaveFileMarker> mMarkers; // 0x14
    IListChunk mRiffList; // 0x20
};

class WaveFileData : public IDataChunk {
public:
    WaveFileData(WaveFile &);
    virtual ~WaveFileData();

private:
    WaveFile &mWaveFile; // 0x28
};
