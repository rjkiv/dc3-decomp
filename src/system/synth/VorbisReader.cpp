#include "synth/VorbisReader.h"
#include "KeyChain.h"
#include "VorbisReader.h"
#include "codec.h"
#include "math/Utl.h"
#include "obj/DataFile.h"
#include "ogg.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/Endian.h"
#include "os/Timer.h"
#include "synth/Synth.h"
#include "utl/BufStream.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbox.h"

namespace {
    unsigned char gKey[256];
    std::list<VorbisReader *> gReaders;
    std::list<VorbisReader *> gNewReaders;
    CriticalSection gLock;

    int gCipher = -1;
    int gKeySize = 16;
    unsigned char gRB1Key[16] = { 0x37, 0xB2, 0xE2, 0xB9, 0x1C, 0x74, 0xFA, 0x9E,
                                  0x38, 0x81, 0x08, 0xEA, 0x36, 0x23, 0xDB, 0xE4 };
    HANDLE gEvent = INVALID_HANDLE_VALUE;

    DWORD DecodeThreadEntry(HANDLE) {
        // this thread is meant to run forever
        while (true) {
            WaitForSingleObject(gEvent, -1);
            gLock.Enter();
            if (!gNewReaders.empty()) {
                gReaders.insert(gReaders.end(), gNewReaders.begin(), gNewReaders.end());
            }
            gLock.Exit();

            bool b2;
            do {
                b2 = false;
                for (auto it = gReaders.begin(); it != gReaders.end();) {
                    VorbisReader *cur = *it;
                    if (cur->Unk24()) {
                        it = gReaders.erase(it);
                        // set unk24 to false
                    } else {
                        ++it;
                        b2 = cur->DecodeThreadPoll() || !b2;
                    }
                }
            } while (b2);
        }
        return 0;
    }
}

#define VORBIS_FAIL(name, err)                                                           \
    MILO_NOTIFY("Ogg Vorbis failure: %s, error code %i", name, err);

VorbisReader::VorbisReader(File *file, bool expectMap, StandardStream *stream, bool b2)
    : mNumChannels(-1), mSampleRate(-1), mFile(file), mHeadersRead(0), mReadBuffer(0),
      mEnableReads(true), mDecryptBytes(0), mNeedInitDecoder(0), mDone(0),
      mStream(stream), mOggSync(0), mOggStream(0), mVorbisInfo(0), mVorbisComment(0),
      mVorbisDsp(0), mVorbisBlock(0), mDecodePending(0), mSeekTarget(-1),
      mSamplesToSkip(0), mHdrSize(0), mHdrBuf(0), mCtrState(0), unkec(b2), unked(0),
      mEof(0), mFail(0), unk100(-1), unk108(0) {
    MILO_ASSERT(mFile, 0xEC);
    if (expectMap) {
        mHdrBuf = new char[60000];
        mFile->ReadAsync(mHdrBuf, 60000);
        mFail = mFile->Fail();
    }
    mOggSync = new ogg_sync_state;
    ogg_sync_init(mOggSync);
    unk24 = false;
    if (gEvent == INVALID_HANDLE_VALUE) {
        gEvent = CreateEventA(nullptr, false, false, nullptr);
        MILO_ASSERT(gEvent, 0xFE);
        HANDLE t = CreateThread(nullptr, 0x10000, DecodeThreadEntry, nullptr, 4, nullptr);
        MILO_ASSERT(t, 0x103);
        DWORD ret = XSetThreadProcessor(t, 2);
        MILO_ASSERT(ret != -1, 0x107);
        ret = ResumeThread(t);
        MILO_ASSERT(ret != -1, 0x109);
    }
    CritSecTracker tracker(&gLock);
    gNewReaders.push_front(this);
}

VorbisReader::~VorbisReader() {
    unk24 = true;
    unked = false;
    while (unk24) {
        SetEvent(gEvent);
    }
    delete[] mHdrBuf;
    mHdrBuf = nullptr;
    if (mOggStream) {
        ogg_stream_clear(mOggStream);
    }
    if (mVorbisBlock) {
        vorbis_block_clear(mVorbisBlock);
    }
    if (mVorbisDsp) {
        vorbis_dsp_clear(mVorbisDsp);
    }
    if (mVorbisComment) {
        vorbis_comment_clear(mVorbisComment);
    }
    if (mVorbisInfo) {
        vorbis_info_clear(mVorbisInfo);
    }
    ogg_sync_clear(mOggSync);
    RELEASE(mVorbisBlock);
    RELEASE(mVorbisDsp);
    RELEASE(mVorbisComment);
    RELEASE(mVorbisInfo);
    RELEASE(mOggStream);
    RELEASE(mOggSync);
    RELEASE(mCtrState);
}

void VorbisReader::Poll(float until) {
    START_AUTO_TIMER("vorbis_reader_poll");
    if (!TryEnter()) {
        return;
    }
    CritSecTracker tracker(this);
    Exit();
    if (mFail) {
        return;
    }
    if (mNeedInitDecoder) {
        return;
    }
    if (!CheckHmxHeader()) {
        return;
    }
    if (mDone) {
        return;
    }
    if (mSeekTarget >= 0 && !DoSeek()) {
        return;
    }

    DoFileRead();
    mEof = mFile->Eof();
    if (mHeadersRead < 3) {
        while (TryReadHeader())
            ;
        if (mHeadersRead < 3) {
            return;
        } else {
            mNumChannels = mVorbisInfo->channels;
            mSampleRate = mVorbisInfo->rate;
            unkf4.resize(mNumChannels);
            for (int i = 0; i < mNumChannels; i++) {
                unkf4[i].reserve(0x1000);
            }
            Init();
            mNeedInitDecoder = true;
            return;
        }
    } else {
        Timer timer;
        timer.Start();
        std::vector<short *> shorts;
        shorts.resize(mNumChannels);
        int i12 = 0;
        while (unk108 < unkf4[0].size() && i12 < 0x800) {
            for (int c = 0; c < mNumChannels; c++) {
                shorts[c] = &unkf4[c][unk108]; // something up here
            }
            int i8;
            if (unk100 == -1) {
                i8 = -1;
            } else {
                i8 = unk100 + unk108;
            }
            int ret = ConsumeData((void **)&shorts, unkf4[0].size() - unk108, i8);
            i12 += ret;
            unk108 += ret;
            if (ret == 0)
                break;
        }
        unked = true;
    }
}

void VorbisReader::Seek(int sample) {
    CritSecTracker tracker(this);
    MILO_ASSERT(mHeadersRead == 3, 0x1BD);
    MILO_ASSERT(mEnableReads, 0x1BE);
    MILO_ASSERT(sample >= 0, 0x1BF);
    mSeekTarget = sample;
    mDone = false;
    unked = false;
}

void VorbisReader::Init() {
    MILO_ASSERT(mStream, 0x41F);
    mStream->InitInfo(mNumChannels, mSampleRate, false, mOggMap.GetSongLengthSamples());
}

int VorbisReader::ConsumeData(void **pcm, int samples, int startSamp) {
    MILO_ASSERT(mSeekTarget == -1, 0x436);
    if (mSamplesToSkip > 0) {
        int consumed = Min(samples, mSamplesToSkip);
        mSamplesToSkip -= consumed;
        return consumed;
    } else {
        MILO_ASSERT(mStream, 0x43F);
        return mStream->ConsumeData(pcm, samples, startSamp);
    }
}

void VorbisReader::setupCypher(int moggVersion) {
    DataArray *arr = DataReadString("{Na 42 'O32'}");
    unsigned int iEval = arr->Evaluate(0).Int();
    arr->Release();

    char i6 = (iEval % 13);
    i6 = i6 + 'A';
    char script[256];
    unsigned char masterKey[256];
    sprintf(script, "{%c %d %c}", i6, (int)masterKey ^ iEval, i6);
    DataArray *buf118Arr = DataReadString(script);
    buf118Arr->Evaluate(0);
    buf118Arr->Release();
    KeyChain::getKey(mKeyIndex, gKey, masterKey);
    TheSynth->Grinder().GrindArray(mMagicA, mMagicB, gKey, 0x10, moggVersion);
    for (int i = 0; i < 16; i++) {
        gKey[i] ^= mKeyMask[i];
    }
    int ret = ctr_start(gCipher, mNonce, gKey, gKeySize, 0, mCtrState);
    memset(gKey, 0, gKeySize);
    MILO_ASSERT(ret == 0, 0xB0);

    sprintf(script, "{ha %d 1}", mMagicA);
    DataArray *magicGenA = DataReadString(script);
    mMagicHashA = magicGenA->Evaluate(0).Int();
    magicGenA->Release();

    sprintf(script, "{ha %d 2}", mMagicB);
    DataArray *magicGenB = DataReadString(script);
    mMagicHashB = magicGenB->Evaluate(0).Int();
    magicGenB->Release();
}

bool VorbisReader::TryReadHeader() {
    if (!mOggStream) {
        ogg_page page;
        int pageOut = ogg_sync_pageout(mOggSync, &page);
        if (pageOut < 0)
            VORBIS_FAIL("StreamInit", pageOut);
        if (pageOut > 0) {
            mOggStream = new ogg_stream_state;
            ogg_stream_init(mOggStream, ogg_page_serialno(&page));
            ogg_stream_pagein(mOggStream, &page);
            mVorbisInfo = new vorbis_info;
            vorbis_info_init(mVorbisInfo);
            mVorbisComment = new vorbis_comment;
            vorbis_comment_init(mVorbisComment);
        } else {
            return false;
        }
    }
    ogg_packet packet;
    if (mHeadersRead != 3) {
        if (TryReadPacket(packet)) {
            int vorbisErr =
                vorbis_synthesis_headerin(mVorbisInfo, mVorbisComment, &packet);
            if (vorbisErr < 0)
                VORBIS_FAIL("HeaderIn", vorbisErr);
            mHeadersRead++;
            return true;
        }
    }
    return false;
}

void VorbisReader::InitDecoder() {
    MILO_ASSERT(!mVorbisDsp && !mVorbisBlock, 0x2C0);
    MILO_ASSERT(mHeadersRead == 3, 0x2C1);
    mVorbisDsp = new vorbis_dsp_state;
    vorbis_synthesis_init(mVorbisDsp, mVorbisInfo);
    mVorbisBlock = new vorbis_block;
    vorbis_block_init(mVorbisDsp, mVorbisBlock);
}

bool VorbisReader::DoSeek() {
    mEnableReads = false;
    DoFileRead();
    if (!mFail && !mReadBuffer) {
        int i1, i2;
        mOggMap.GetSeekPos(mSeekTarget, i1, i2);
        DoRawSeek(i1);
        mSamplesToSkip = mSeekTarget - i2;
        MILO_ASSERT(mSamplesToSkip >= 0, 0x3C2);
        mSeekTarget = -1;
        mEnableReads = true;
        return true;
    }
    return false;
}

int VorbisReader::QueuedOutputSamples() {
    if (mVorbisDsp) {
        START_AUTO_TIMER("vorbis_synthesis_pcmout_cpu");
        return vorbis_synthesis_pcmout(mVorbisDsp, nullptr);
    } else
        return 0;
}

bool VorbisReader::TryReadPacket(ogg_packet &pk) {
    MILO_ASSERT(mOggStream, 0x39C);
    while (true) {
        int streamErr = ogg_stream_packetout(mOggStream, &pk);
        if (streamErr < 0) {
            VORBIS_FAIL("PacketOut", streamErr);
        }
        if (streamErr > 0)
            return true;
        ogg_page page;
        int syncErr = ogg_sync_pageout(mOggSync, &page);
        if (syncErr > 0)
            ogg_stream_pagein(mOggStream, &page);
        else
            return false;
    }
}

bool VorbisReader::TryDecode() {
    if (mFail)
        return false;
    if (QueuedOutputSamples() > 0)
        return false;
    if (!mDecodePending && TryReadPacket(mPendingPacket)) {
        mDecodePending = true;
    }
    if (mDecodePending) {
        int pollErr;
        {
            START_AUTO_TIMER("vorbis_synthesis_poll_cpu");
            if (mVorbisBlock->synthesis_state == vorbis_block::vss_init) {
                START_AUTO_TIMER("vorbis_synthesis_vssinit_cpu");
                pollErr = vorbis_synthesis_poll(mVorbisBlock, &mPendingPacket);
            } else if (mVorbisBlock->synthesis_state == vorbis_block::vss_decode) {
                START_AUTO_TIMER("vorbis_synthesis_vssdecode_cpu");
                pollErr = vorbis_synthesis_poll(mVorbisBlock, &mPendingPacket);
            } else {
                START_AUTO_TIMER("vorbis_synthesis_vssmdct_cpu");
                pollErr = vorbis_synthesis_poll(mVorbisBlock, &mPendingPacket);
            }
        }
        if (pollErr == OV_ENOTAUDIO) {
            mDecodePending = false;
        } else {
            if (pollErr == -0x32)
                return true;
            if (pollErr < 0) {
                VORBIS_FAIL("Synthesis", pollErr);
            }
            mDecodePending = false;
            if (pollErr == 0) {
                START_AUTO_TIMER("vorbis_synthesis_blockin_cpu");
                int blockErr = vorbis_synthesis_blockin(mVorbisDsp, mVorbisBlock);
                if (blockErr < 0) {
                    VORBIS_FAIL("BlockIn", blockErr);
                }
                return true;
            }
        }
    } else if (mEof && !mReadBuffer && QueuedOutputSamples() == 0 && !mDone
               && unk108 >= unkf4[0].size()) {
        EndData();
        mDone = true;
    }
    return false;
}

void VorbisReader::SignalDecodeThread() {
    if (gEvent != INVALID_HANDLE_VALUE) {
        SetEvent(gEvent);
    }
}

void VorbisReader::DoRawSeek(int byte) {
    if (mReadBuffer) {
        mEnableReads = false;
        while (!mFail && mReadBuffer)
            DoFileRead();
        mEnableReads = true;
    }
    for (int i = 0; i < mNumChannels; i++) {
        unkf4[i].clear();
    }
    unk108 = 0;
    unk100 = -1;
    int streamErr = ogg_stream_reset(mOggStream);
    if (streamErr < 0)
        VORBIS_FAIL("StreamReset", streamErr);
    int syncErr = ogg_sync_reset(mOggSync);
    if (syncErr < 0)
        VORBIS_FAIL("SyncReset", syncErr);
    vorbis_block_clear(mVorbisBlock);
    int restartErr = vorbis_synthesis_restart(mVorbisDsp);
    if (restartErr < 0)
        VORBIS_FAIL("DspReset", restartErr);
    vorbis_block_init(mVorbisDsp, mVorbisBlock);
    mDecodePending = false;
    mFile->Seek(byte + mHdrSize, 0);
    if (mCtrState) {
        MILO_ASSERT(byte%16 == 0, 0x3F4);
        // this is the part where the word that makes up byte,
        // gets assigned to the word that makes up mNonce
        *(int *)mNonce = EndianSwap((unsigned int)(byte / 16));
        int ret = ctr_reinit(gCipher, mNonce, mCtrState);
        MILO_ASSERT(ret == 0, 0x3F7);
    }
    mDone = false;
    mEof = false;
}

#define kMaxHeader 60000

bool VorbisReader::CheckHmxHeader() {
    if (!mHdrBuf)
        return true;
    else {
        int bytes;
        if (mFile->ReadDone(bytes)) {
            BufStream bs(mHdrBuf, 60000, true);
            bs >> mVersion;
            bs >> mHdrSize;
            MILO_ASSERT(mVersion >= 10, 0x239);
            MILO_ASSERT(mVersion <= 16, 0x23A);
            MILO_ASSERT(mHdrSize <= kMaxHeader, 0x23B);
            MILO_ASSERT(mHdrSize >= 0, 0x23C);
            mOggMap.Read(bs);
            mKeyIndex = 0;
            memset(mKeyMask, 0, sizeof(mKeyMask));
            mMagicA = mMagicB = 0;
            mMagicHashA = mMagicHashB = 0;
            if (mVersion >= 0xC && mVersion <= 0x10) {
                bs.Read(mNonce, sizeof(mNonce));
                s64 idx;
                bs >> idx;
                mMagicA = idx;
                bs >> idx;
                mMagicB = idx;
                unsigned char stuff[16];
                bs.Read(stuff, sizeof(stuff));
                bs.Read(stuff, sizeof(stuff));
                bs >> idx;
                mKeyIndex = (int)idx % 6 + 6;
                TheSynth->Grinder().HvDecrypt(stuff, mKeyMask, mVersion);
                gCipher = register_cipher(&rijndael_desc);
                MILO_ASSERT(gCipher >= 0, 0x268);
                mCtrState = new symmetric_CTR;
                int keyIndex = mKeyIndex;
                MILO_ASSERT_RANGE(keyIndex, 0, KeyChain::getNumKeys(), 0x26D);
                setupCypher(mVersion);
            } else if (mVersion == 0xB) {
                bs.Read(mNonce, sizeof(mNonce));
                gCipher = register_cipher(&rijndael_desc);
                MILO_ASSERT(gCipher >= 0, 0x276);
                mCtrState = new symmetric_CTR;
                int ret = ctr_start(gCipher, mNonce, gRB1Key, gKeySize, 0, mCtrState);
                MILO_ASSERT(ret == 0, 0x27E);
            } else {
                MILO_NOTIFY_ONCE("old mogg version!");
            }
            delete[] mHdrBuf;
            mHdrBuf = nullptr;
            mFile->Seek(mHdrSize, BinStream::kSeekBegin);
        }
        mFail = mFile->Fail();
        return !mHdrBuf;
    }
}

void VorbisReader::Decrypt(unsigned char *data, int bytes) {
    unsigned char in[0x4000];
    unsigned char out[0x4000];

    if (mCtrState) {
        for (int i = 0; i < bytes;) {
            int n = Min<int>(bytes - i, sizeof(in));
            memcpy(in, data + i, n);
            int ret = ctr_decrypt(in, out, n, mCtrState);
            if ((mMagicHashA != 0 || mMagicHashB != 0) && out[0] == 'H' && out[1] == 'M'
                && out[2] == 'X' && out[3] == 'A') {
                out[2] = 'g';
                out[1] = 'g';
                out[0] = 'O';
                out[3] = 'S';
                if (n >= 16) {
                    *(long *)&out[12] ^= mMagicHashA;
                }
                if (n >= 24) {
                    *(long *)&out[20] ^= mMagicHashB;
                }
            }
            MILO_ASSERT(ret == 0, 0x224);
            memcpy(data + i, out, n);
            i += n;
        }
    }
}

bool VorbisReader::DoFileRead() {
    START_AUTO_TIMER("vorbis_file_read");
    bool ret = false;
    if (mFail) {
        return false;
    }
    if (mEnableReads && !mReadBuffer && !mFile->Eof()
        && mOggSync->fill - mOggSync->returned < 0x10000) {
        mReadBuffer = (unsigned char *)ogg_sync_buffer(mOggSync, 0x4000);
        {
            static Timer *t = AutoTimer::GetTimer("synth_poll");
            if (t) {
                t->Stop();
            }
        }
        mFile->ReadAsync(mReadBuffer, 0x4000);
        {
            static Timer *t = AutoTimer::GetTimer("synth_poll");
            if (t) {
                t->Start();
            }
        }
        mFail = mFile->Fail();
        ret = true;
    }
    int bytes = 0;
    if (!mFail && mReadBuffer && mFile->ReadDone(bytes) && mDecryptBytes == 0) {
        mFail = mFile->Fail();
        if (mFail) {
            return false;
        }
        MILO_ASSERT(bytes > 0, 0x1E7);
        ret = true;
        mDecryptBytes = bytes;
    }
    mFail = mFile->Fail();
    return ret;
}
