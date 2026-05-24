#pragma once
#include "oggvorbis/codec.h"
#include "oggvorbis/ogg.h"
#include "os/CritSec.h"
#include "os/File.h"
#include "synth/OggMap.h"
#include "synth/StandardStream.h"
#include "synth/StreamReader.h"
#include "synth/tomcrypt/mycrypt.h"

class VorbisReader : public StreamReader, public CriticalSection {
public:
    VorbisReader(File *, bool, StandardStream *, bool);
    virtual ~VorbisReader();
    virtual void Poll(float until);
    virtual void Seek(int sample);
    virtual void EnableReads(bool enable) { mEnableReads = enable; }
    virtual bool Done() { return mDone; }
    virtual bool Fail() { return mFail; }

    static void SignalDecodeThread();

private:
    bool TryReadHeader();
    bool TryReadPacket(ogg_packet &);
    void InitDecoder();
    bool DoSeek();
    bool DoFileRead();
    int QueuedOutputSamples();
    bool TryDecode();
    bool CheckHmxHeader();

protected:
    virtual void Init();
    virtual int ConsumeData(void **pcm, int samples, int startSamp);
    virtual void EndData() {}

    void setupCypher(int);
    void DoRawSeek(int byte);

    // volatile so the dtor matches.
    // doesn't seem to affect any other function so i guess it's fine
    volatile bool unk24;
    int mNumChannels; // 0x28
    int mSampleRate; // 0x2c
private:
    File *mFile; // 0x30
    int mHeadersRead; // 0x34
    unsigned char *mReadBuffer; // 0x38
    bool mEnableReads; // 0x3c
    int mDecryptBytes; // 0x40
    bool mNeedInitDecoder; // 0x44
    bool mDone; // 0x45
    float mStartMs; // 0x48
    StandardStream *mStream; // 0x4c
    ogg_sync_state *mOggSync; // 0x50
    ogg_stream_state *mOggStream; // 0x54
    vorbis_info *mVorbisInfo; // 0x58
    vorbis_comment *mVorbisComment; // 0x5c
    vorbis_dsp_state *mVorbisDsp; // 0x60
    vorbis_block *mVorbisBlock; // 0x64
    long mMagicA; // 0x68 - byte grinder seed A
    long mMagicB; // 0x6c - byte grinder seed B
    long mKeyIndex; // 0x70
    long mMagicHashA; // 0x74
    long mMagicHashB; // 0x78
    ogg_packet mPendingPacket; // 0x80
    bool mDecodePending; // 0xa0
    int mSeekTarget; // 0xa4
    int mSamplesToSkip; // 0xa8
    OggMap mOggMap; // 0xac
    int mHdrSize; // 0xc0
    char *mHdrBuf; // 0xc4
    symmetric_CTR *mCtrState; // 0xc8
    unsigned char mNonce[16]; // 0xcc
    unsigned char mKeyMask[16]; // 0xdc
    bool unkec; // 0xec
    bool unked; // 0xed
    bool mEof; // 0xee
    bool mFail; // 0xef
    /** The mogg's encryption version. */
    int mVersion; // 0xf0
    std::vector<std::vector<short> > unkf4; // 0xf4
    s64 unk100;
    int unk108;
};
