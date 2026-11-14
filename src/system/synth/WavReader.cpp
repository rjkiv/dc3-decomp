#include "WavReader.h"
#include "synth/StreamReader.h"

WavReader::WavReader(File *file, StandardStream *stream)
    : mInFile(file), mOutStream(stream) {
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
        const WaveFileMarker &cur = mInWaveFile->GetMarker(i);
        int frame = cur.GetFrame();
        float samps = mInWaveFile->SamplesPerSec();
        float pos = (frame * 1000.0f) / samps;
        stream->AddMarker(Marker(cur.GetName(), frame, pos));
    }
    mInWaveFileData = new WaveFileData(*mInWaveFile);
    mInputBuffers[0] = new unsigned short[0x1000];
    mInputBuffers[1] = new unsigned short[0x1000];
    mRawInputBuffer = new unsigned short[0x2000];
    mTotalSamplesConsumed = 0;
    mBufNumSamples = 0;
    mBufOffset = 0;
    mEnableReads = true;
    mInitted = false;
}

WavReader::~WavReader() {
    delete mInWaveFileData;
    delete mInWaveFile;
    delete mInFileStream;
    delete[] mInputBuffers[0];
    delete[] mInputBuffers[1];
    delete mRawInputBuffer;
}

void WavReader::Poll(float until) {
    if (!mInitted) {
        mInitted = true;
        Init();
    }
    if (mBufNumSamples) {
        void *buffers[2] = { mInputBuffers[0] + mBufOffset,
                             mInputBuffers[1] + mBufOffset };
        int i6 = ConsumeData(&buffers[0], mBufNumSamples, mTotalSamplesConsumed);
        mTotalSamplesConsumed += i6;
        mBufNumSamples -= i6;
        mBufOffset += i6;
        if (mBufNumSamples) {
            return;
        }
    }
    if (mEnableReads) {
        while (mSamplesLeft != 0) {
            if (mSamplesLeft < mNumChannels) {
                mSamplesLeft = 0;
                return;
            }
            mBufNumSamples = Min(mSamplesLeft / mNumChannels, 0x1000);
            mInWaveFileData->Read(mRawInputBuffer, mNumChannels * mBufNumSamples * 2);
            mBufOffset = 0;
            mSamplesLeft -= mBufNumSamples;
            if (mNumChannels == 1) {
                for (int i = 0; i < mBufNumSamples; i++) {
                    mInputBuffers[0][i] =
                        (mRawInputBuffer[i] << 8) | (mRawInputBuffer[i] >> 8);
                }
            } else {
                for (int i = 0; i < mBufNumSamples; i++) {
                    mInputBuffers[0][i] =
                        (mRawInputBuffer[i * 2] << 8) | (mRawInputBuffer[i * 2] >> 8);
                    mInputBuffers[1][i] = (mRawInputBuffer[i * 2 + 1] << 8)
                        | (mRawInputBuffer[i * 2 + 1] >> 8);
                }
            }
            if (mBufNumSamples) {
                int i6 = ConsumeData(
                    (void **)&mInputBuffers[0], mBufNumSamples, mTotalSamplesConsumed
                );
                mTotalSamplesConsumed += i6;
                mBufNumSamples -= i6;
                mBufOffset += i6;
                if (mBufNumSamples) {
                    return;
                }
            }
        }
    }
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
