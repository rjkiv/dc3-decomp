#include "WavReader.h"

#include "os/Memcard.h"

WavReader::WavReader(File *file, StandardStream *stream) : mInFile(file), mOutStream(stream){
    MILO_ASSERT(mInFile, 0x1a);
    mInFileStream = new FileStream(file, true);
    mInWaveFile = new WaveFile(*mInFileStream);
    MILO_ASSERT(mInWaveFile->SamplesPerSec() == 44100, 0x21);
    MILO_ASSERT(mInWaveFile->BitsPerSample() == 16, 0x22);
    MILO_ASSERT(mInWaveFile->NumChannels() <= 2, 0x23);
    mNumChannels = mInWaveFile->NumChannels();
    mSampleRate = mInWaveFile->SamplesPerSec();
    mSamplesLeft = mInWaveFile->NumSamples();
    for (int i = 0; i < mInWaveFile->NumMarkers(); i++) {
        stream->AddMarker(mInWaveFile->Markers()[i].mName);
    }
    mInWaveFileData = new WaveFileData(*mInWaveFile);
    mInputBuffers[0] = new unsigned short[0x1000];
    mInputBuffers[1] = new unsigned short[0x1000];
    mRawInputBuffer = new unsigned short[0x2000];
    mTotalSamplesConsumed = 0;
    mBufNumSamples = 0;
    mBufOffset = 0;
    mEnableReads = true;
    mInitted = true;
}

WavReader::~WavReader() {
    delete mInWaveFileData;
    delete mInWaveFile;
    delete mInFileStream;
    delete[] mInputBuffers[0];
    delete[] mInputBuffers[1];
    delete mRawInputBuffer;
}

void WavReader::Seek(int samples) {
    mInWaveFileData->Seek(mNumChannels * samples * 2, BinStream::SeekType::kSeekBegin);
    mSamplesLeft = (mSamplesLeft - samples) + mBufNumSamples + mTotalSamplesConsumed;
    mTotalSamplesConsumed = samples;
    mBufOffset = 0;
    mBufNumSamples = 0;
}

void WavReader::Init() {
    MILO_ASSERT(mOutStream, 0xaa);
    mOutStream->InitInfo(mNumChannels, mSampleRate, false, mInWaveFile->NumSamples());
}

int WavReader::ConsumeData(void **data, int samples, int startSamp) {
    MILO_ASSERT(mOutStream, 0xb1);
    return mOutStream->ConsumeData(data, samples, startSamp);
}