#include "utl/WaveFile.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/ChunkIDs.h"
#include "utl/Chunks.h"
#include "utl/Str.h"

namespace {
    struct CuePoint {
        CuePoint(int frame, int id) : mFrame(frame), mID(id) {}
        // total size: 0x8
        int mFrame; // offset 0x0, size 0x4
        int mID; // offset 0x4, size 0x4
    };

    struct Label {
        Label(const String &s, int i) : mName(s), mID(i) {}
        // total size: 0x10
        class String mName; // offset 0x0, size 0xC
        int mID; // offset 0xC, size 0x4
    };

    bool CompareCuePoints(const CuePoint &p1, const CuePoint &p2) {
        return p1.mFrame < p2.mFrame;
    }
}

WaveFile::WaveFile(BinStream &bs) : mMarkers(), mRiffList(bs, true) {
    ReadFormat();
    ReadMarkers();
    ReadNumSamples();
}

WaveFile::~WaveFile() {}

void WaveFile::ReadFormat() {
    mRiffList.Reset();
    ChunkHeader *chunk = (ChunkHeader *)mRiffList.Next(kWaveFormatChunkID);
    MILO_ASSERT(chunk != null, 0x6e);
    IDataChunk dchunk(mRiffList);
    dchunk >> mFormat;
    dchunk >> mNumChannels;
    dchunk >> mSamplesPerSec;
    dchunk >> mAvgBytesPerSec;
    dchunk >> mBlockAlign;
    dchunk >> mBitsPerSample;
    MILO_ASSERT(mAvgBytesPerSec == mSamplesPerSec * mBlockAlign, 0x78);
    MILO_ASSERT(mBlockAlign == mNumChannels * mBitsPerSample / 8, 0x79);
}

void WaveFile::ReadMarkers() {
    std::vector<CuePoint> cuevec;
    std::vector<Label> labelvec;
    mRiffList.Reset();
    if (!mRiffList.Next(kWaveCueChunkID))
        return;
    int cuesize, i;
    {
        IDataChunk iChunk(mRiffList);
        iChunk >> cuesize;

        for (i = 0; i < cuesize; i++) {
            int id, frame;
            iChunk >> id >> frame;
            cuevec.push_back(CuePoint(frame, id));
            iChunk.Seek(0x10, BinStream::kSeekCur);
        }
    }

    if (cuesize == 0)
        return;

    mRiffList.Reset();
    if (mRiffList.Next(kWaveAdditionalChunkID)) {
        IListChunk iChunk(mRiffList);
        for (i = 0; i < cuesize; i++) {
            iChunk.Next();
            IDataChunk dataChunk(iChunk);
            if (dataChunk.Header()->ID() == kWaveLabelChunkID) {
                int len = dataChunk.Header()->Length() - 4;
                int datalen;
                dataChunk >> datalen;
                String str;
                str.resize(len);
                dataChunk.Read((void *)str.c_str(), len);
                labelvec.push_back(Label(str, datalen));
            } else if (dataChunk.Header()->ID() == kWaveTextChunkID) {
                int len = dataChunk.Header()->Length() - 20;
                int datalen;
                dataChunk >> datalen;
                int j, k;
                dataChunk >> j;
                dataChunk >> k;
                short w, x, y, z;
                dataChunk >> w;
                dataChunk >> x;
                dataChunk >> y;
                dataChunk >> z;
                String str;
                str.resize(len);
                dataChunk.Read((void *)str.c_str(), len);
                labelvec.push_back(Label(str, datalen));
            }
        }
    }

    std::sort(cuevec.begin(), cuevec.end(), CompareCuePoints);
    for (i = 0; i < cuesize; i++) {
        String strc8;
        for (int j = 0; j < labelvec.size(); j++) {
            if (labelvec[j].mID == cuevec[i].mID) {
                strc8 = labelvec[j].mName;
                break;
            }
        }
        mMarkers.push_back(WaveFileMarker(cuevec[i].mFrame, cuevec[i].mID, strc8));
    }
}

void WaveFile::ReadNumSamples() {
    mRiffList.Reset();
    ChunkHeader *chunk = (ChunkHeader *)mRiffList.Next(kWaveDataChunkID);
    MILO_ASSERT(chunk != null, 0xEA);
    mNumSamples = chunk->Length() / mBlockAlign;
}

IListChunk &WaveFile::PrepareToProvideData() {
    mRiffList.Reset();
    ChunkHeader *chunk = (ChunkHeader *)mRiffList.Next(kWaveDataChunkID);
    MILO_ASSERT(chunk != null, 0x17e);
    return mRiffList;
}

WaveFileData::WaveFileData(WaveFile &wf)
    : IDataChunk(wf.PrepareToProvideData()), mWaveFile(wf) {}

WaveFileData::~WaveFileData() {}
