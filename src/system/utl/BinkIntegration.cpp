#include "utl/BinkIntegration.h"
#include "KeyChain.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "os/CritSec.h"
#include "os/Endian.h"
#include "os/File.h"
#include "os/Timer.h"
#include "synth/Synth.h"
#include "utl/EncryptXTEA.h"
#include "utl/MemMgr.h"
#include "binkxenon/bink.h"
#include "binkxenon/rad.h"
#include <cstring>
#include <cstdio>

CriticalSection gCrit;

static void *BinkAlloc(unsigned int size) {
    return MemAlloc(size, __FILE__, 0x44, "Bink Internal");
}

static void BinkFree(void *ptr) { return MemFree(ptr); }

static void ReadFunc(struct BINKIO *pBinkIO, bool bStartNewRead) {
    BINKFILE *binkFile = (BINKFILE *)pBinkIO->iodata;
    if (pBinkIO->DoingARead) {
        int bytes = 0;
        if (!binkFile->pFile->ReadDone(bytes)) {
            return;
        }
        pBinkIO->DoingARead = 0;
        if (binkFile->mEncryptionHeader.mVersion == 2) {
            START_AUTO_TIMER("XTEA");
            XTEABlock decrypted;
            for (XTEABlock *it = (XTEABlock *)binkFile->pBufBack;
                 it < (XTEABlock *)(binkFile->pBufBack + bytes);
                 it++) {
                it->mData[0] = EndianSwap(it->mData[0]);
                it->mData[1] = EndianSwap(it->mData[1]);
                binkFile->pXTEADecrypter->Encrypt(it, &decrypted);
                unsigned int *itUI = (unsigned int *)it;
                unsigned int *decUI = (unsigned int *)&decrypted;

                itUI[0] = decUI[1];
                itUI[1] = decUI[0];
                itUI[2] = decUI[3];
                itUI[3] = decUI[2];

                // it->mData[0] = decrypted.mData[0];
                // it->mData[1] = decrypted.mData[1];
                // *it = decrypted;
            }
        } else {
            EndianSwapBlock(
                (unsigned int *)binkFile->pBufBack, bytes / sizeof(unsigned int)
            );
        }
        binkFile->pBufBack += bytes;
        if (binkFile->pBufBack >= binkFile->pBufEnd) {
            binkFile->pBufBack = binkFile->pBuffer;
        }
        binkFile->iBufEmpty -= bytes;
        pBinkIO->CurBufUsed += bytes;
        pBinkIO->BytesRead += bytes;
        if (pBinkIO->CurBufUsed > pBinkIO->BufHighUsed) {
            pBinkIO->BufHighUsed = pBinkIO->CurBufUsed;
        }

        unsigned int diff = RADTimerRead() - binkFile->iTimer;
        binkFile->iTimer = diff;
        pBinkIO->TotalTime += diff;

        if (pBinkIO->Suspended) {
            return;
        }
    }
    if (bStartNewRead) {
        int i7 = binkFile->pFile->Size() - binkFile->pFile->Tell();
        if (binkFile->iBufEmpty < 0x8000 || binkFile->pFile->Eof()) {
            pBinkIO->CurBufSize = pBinkIO->CurBufUsed;
        } else {
            pBinkIO->DoingARead = 1;
            if ((i7 & 0xFFFFFFFF) > 0x8000) {
                i7 = 0x8000;
            }
            binkFile->pFile->ReadAsync(binkFile->pBufBack, i7);
        }
    }
}

static unsigned int BinkFileReadHeader(
    struct BINKIO *pBinkIO, int iOffset, void *pDest, unsigned int iReadSize
) {
    BINKFILE *pBinkFileInfo = (BINKFILE *)pBinkIO->iodata;
    File *pReadFile = pBinkFileInfo->pFile;
    BINKENCRYPTIONHEADER *pEncryptionHeader = &pBinkFileInfo->mEncryptionHeader;
    if (pEncryptionHeader->mSignature == 0) {
        int iEncryptionHeaderReadSize =
            pReadFile->Read(pEncryptionHeader, sizeof(BINKENCRYPTIONHEADER));
        EndianSwapBlock(&pEncryptionHeader->mSignature, 5);
        pEncryptionHeader->mNonce[0] = EndianSwap(pEncryptionHeader->mNonce[0]);
        pEncryptionHeader->mNonce[1] = EndianSwap(pEncryptionHeader->mNonce[1]);
        if (pEncryptionHeader->mSignature == 'EBIK') {
            pBinkFileInfo->pXTEADecrypter = new XTEABlockEncrypter();
            DataArray *dataStr = DataReadString("{Na 42 'O32'}");
            unsigned int iEval = dataStr->Evaluate(0).Int();
            dataStr->Release();

            char functionName = (iEval % 13);
            functionName += 'A';
            char script[256];
            unsigned char masterKey[256];
            sprintf(
                script, "{%c %d %c}", functionName, (int)masterKey ^ iEval, functionName
            );
            DataArray *scriptArr = DataReadString(script);
            scriptArr->Evaluate(0);
            scriptArr->Release();
            unsigned char key[16];
            KeyChain::getKey(pEncryptionHeader->mKeyIndex, key, masterKey);
            TheSynth->GetGrinder().GrindArray(
                pEncryptionHeader->mMagicA, pEncryptionHeader->mMagicB, key, 16, 12
            );
            for (int i = 0; i < 16; i++) {
                key[i] ^= pEncryptionHeader->mKeyMask[i];
            }
            EndianSwapBlock((unsigned int *)key, 4);
            pBinkFileInfo->pXTEADecrypter->SetKey(key);
            pBinkFileInfo->pXTEADecrypter->SetNonce(
                pBinkFileInfo->mEncryptionHeader.mNonce, 0
            );
            pBinkFileInfo->iFileBufPos += iEncryptionHeaderReadSize;
        } else {
            memset(pEncryptionHeader, 0, iEncryptionHeaderReadSize);
            pReadFile->Seek(-iEncryptionHeaderReadSize, FILE_SEEK_CUR);
            if (pBinkIO->bink && pBinkIO->bink->NumTracks > 2
                && pBinkIO->bink->Width < 8) {
                MILO_LOG("Attempting read of unsecure Bink song file!\n");
            }
        }
    }

    unsigned int bytes = pReadFile->Read(pDest, iReadSize);
    if (bytes != iReadSize) {
        pBinkIO->ReadError = 1;
    }
    pBinkFileInfo->iHeaderSize += bytes;
    pBinkFileInfo->iFileBufPos += bytes;
    unsigned int size = pReadFile->Size() - pBinkFileInfo->iFileBufPos;
    if (size >= pBinkIO->BufSize) {
        size = pBinkIO->BufSize;
    }
    pBinkIO->CurBufSize = size;
    EndianSwapBlock((unsigned int *)pDest, bytes / sizeof(unsigned int));
    return bytes;
}

#define BINK_FILE_INFO ((BINKFILE *)pBinkIO->iodata)

static unsigned int BinkFileReadFrame(
    struct BINKIO *pBinkIO,
    unsigned int iFrameNum,
    int iOffset,
    void *pDest,
    unsigned int iReadSize
) {
    CritSecTracker t(&gCrit);
    BINKFILE *pBinkFileInfo = BINK_FILE_INFO;
    if (pBinkIO->ReadError != 0) {
        return 0;
    } else {
        int i14 = (pBinkFileInfo->mEncryptionHeader.mSignature != 0 ? 56 : 0) + iOffset;
        if (i14 + iReadSize > pBinkFileInfo->pFile->Size()) {
            pBinkIO->ReadError = 1;
            return 0;
        } else {
            unsigned int read1 = RADTimerRead();
            if (i14 != -1 && i14 != BINK_FILE_INFO->iFileBufPos) {
                unsigned int i4 = 0;
                if (i14 > BINK_FILE_INFO->iFileBufPos
                    && i14 <= pBinkFileInfo->pFile->Tell()) {
                    unsigned int i6 = i14 - BINK_FILE_INFO->iFileBufPos;
                    BINK_FILE_INFO->pBufPos += i6;
                    if (BINK_FILE_INFO->pBufPos > BINK_FILE_INFO->pBufEnd) {
                        BINK_FILE_INFO->pBufPos -= pBinkIO->BufSize;
                    }
                    BINK_FILE_INFO->iBufEmpty += i6;
                    pBinkIO->CurBufUsed -= i6;
                } else {
                    while (pBinkIO->DoingARead) {
                        ReadFunc(pBinkIO, false);
                    }
                    BINK_FILE_INFO->pBufPos = BINK_FILE_INFO->pBuffer;
                    BINK_FILE_INFO->pBufBack = BINK_FILE_INFO->pBuffer;
                    BINK_FILE_INFO->iBufEmpty = pBinkIO->BufSize;
                    pBinkIO->CurBufUsed = 0;
                    if (pBinkFileInfo->mEncryptionHeader.mVersion == 2) {
                        unsigned int headerSize = pBinkFileInfo->iHeaderSize;
                        unsigned int i7 = i14 - headerSize - 56;
                        i4 = i7 & 0xF;
                        BINK_FILE_INFO->pBufPos = (i7 & 0xF) + BINK_FILE_INFO->pBuffer;
                        i14 = (i7 & 0xFFFFFFF0) + headerSize + 56;
                        pBinkFileInfo->pXTEADecrypter->SetNonce(
                            pBinkFileInfo->mEncryptionHeader.mNonce, i7 / 16
                        );
                    }
                    pBinkFileInfo->pFile->Seek(i14, 0);
                    pBinkIO->DoingARead = 0;
                }
                BINK_FILE_INFO->iFileBufPos = i14 + i4;
            }
            unsigned int ret = 0;
            if (!BINK_FILE_INFO->pBuffer) {
                unsigned int read2 = RADTimerRead();
                unsigned int numRead = pBinkFileInfo->pFile->Read(pDest, iReadSize);
                if (numRead < iReadSize) {
                    pBinkIO->ReadError = 1;
                }
                ret = numRead;
                pBinkIO->BytesRead += numRead;
                BINK_FILE_INFO->iFileBufPos += numRead;
                unsigned int read3 = RADTimerRead();
                u32 value = pBinkIO->TotalTime;
                u32 diffA = read3 - read2;
                pBinkIO->TotalTime = value + diffA;
                pBinkIO->TotalTime += read3 - read1;
                EndianSwapBlock((unsigned int *)pDest, numRead / 4);
            } else {
                while (iReadSize != 0 && pBinkIO->ReadError == 0) {
                    ReadFunc(pBinkIO, true);
                    unsigned int u1 = pBinkIO->CurBufUsed;
                    if (u1 > iReadSize) {
                        u1 = iReadSize;
                    }
                    if (u1 != 0) {
                        iReadSize -= u1;
                        ret += u1;
                        unsigned int u11 =
                            BINK_FILE_INFO->pBufEnd - BINK_FILE_INFO->pBufPos;
                        BINK_FILE_INFO->iFileBufPos += u1;
                        if (u11 <= u1) {
                            memcpy(pDest, BINK_FILE_INFO->pBufPos, u11);
                            pDest = (char *)pDest + u11;
                            u1 -= u11;
                            BINK_FILE_INFO->iBufEmpty += u11;
                            BINK_FILE_INFO->pBufPos = BINK_FILE_INFO->pBuffer;
                            pBinkIO->CurBufUsed -= u11;
                        }
                        if (u1) {
                            memcpy(pDest, BINK_FILE_INFO->pBufPos, u1);
                            pDest = (char *)pDest + u1;
                            BINK_FILE_INFO->pBufPos += u1;
                            BINK_FILE_INFO->iBufEmpty += u1;
                            pBinkIO->CurBufUsed -= u1;
                        }
                    }
                }
                pBinkIO->ForegroundTime += RADTimerRead() - read1;
            }

            unsigned int u7 = pBinkFileInfo->pFile->Size() - BINK_FILE_INFO->iFileBufPos;
            if (u7 >= pBinkIO->BufSize) {
                u7 = pBinkIO->BufSize;
            }
            pBinkIO->CurBufSize = u7;
            if (pBinkIO->CurBufUsed + 0x8000 > pBinkIO->CurBufSize) {
                pBinkIO->CurBufSize = pBinkIO->CurBufUsed;
            }
            return ret;
        }
    }
}

static unsigned int BinkFileGetBufferSize(struct BINKIO *, unsigned int size) {
    unsigned int ret = size + 0x7FFF & 0xFFFF8000;
    if (ret >= 0x10000) {
        return ret;
    } else {
        return 0x10000;
    }
}

static void BinkFileSetInfo(
    struct BINKIO *pBinkIO,
    void *buffer,
    unsigned int size,
    unsigned int filesize,
    unsigned int simulate
) {
    size &= 0xFFFF8000;
    BINKFILE *binkFile = (BINKFILE *)pBinkIO->iodata;
    binkFile->pBuffer = (unsigned char *)buffer;
    binkFile->pBufPos = (unsigned char *)buffer;
    binkFile->pBufBack = (unsigned char *)buffer;
    binkFile->pBufEnd = (unsigned char *)buffer + size;
    binkFile->iBufEmpty = size;
    pBinkIO->BufSize = size;
    pBinkIO->CurBufUsed = 0;
    binkFile->iSimulateBPS = simulate;
}

static unsigned int BinkFileIdle(struct BINKIO *pBinkIO) {
    if (pBinkIO->ReadError) {
        return 0;
    }
    if (pBinkIO->Suspended) {
        return 0;
    }
    if (pBinkIO->DoingARead) {
        CritSecTracker t(&gCrit);
        ReadFunc(pBinkIO, false);
    }
    return pBinkIO->DoingARead;
}

static void BinkFileClose(struct BINKIO *pBinkIO) {
    BINKFILE *binkFile = (BINKFILE *)pBinkIO->iodata;
    if (binkFile->iCloseFile != 0) {
        RELEASE(binkFile->pFile);
    }
    if (binkFile->mEncryptionHeader.mVersion == 2) {
        delete binkFile->pXTEADecrypter;
    }
}

static int BinkFileBGControl(struct BINKIO *pBinkIO, unsigned int Control) {
    if (Control & 1) {
        if (pBinkIO->Suspended == 0) {
            pBinkIO->Suspended = 1;
        }
        if (Control & 0x80000000)
            while (pBinkIO->DoingARead)
                ;
    } else if (Control & 2) {
        if (pBinkIO->Suspended == 1) {
            pBinkIO->Suspended = 0;
        }
        if (Control & 0x80000000) {
            BinkFileIdle(pBinkIO);
        }
    }
    return pBinkIO->Suspended;
}

static int BinkFileOpen(BINKIO *pBinkIO, const char *pFileName, unsigned int iOpenFlags) {
    memset(pBinkIO, 0, sizeof(BINKIO));
    BINKFILE *binkFile = (BINKFILE *)pBinkIO->iodata;
    if (iOpenFlags & BINKFILEHANDLE) {
        binkFile->pFile = (File *)pFileName; // sure why not
    } else {
        File *file = NewFile(pFileName, FILE_OPEN_READ);
        binkFile->pFile = file;
        binkFile->iCloseFile = 1;
        if (!file) {
            return 0;
        }
    }
    pBinkIO->ReadHeader = BinkFileReadHeader;
    pBinkIO->ReadFrame = BinkFileReadFrame;
    pBinkIO->GetBufferSize = BinkFileGetBufferSize;
    pBinkIO->SetInfo = BinkFileSetInfo;
    pBinkIO->Idle = BinkFileIdle;
    pBinkIO->Close = BinkFileClose;
    pBinkIO->BGControl = BinkFileBGControl;
    return 1;
}

void BinkInit() {
    BinkSetMemory(BinkAlloc, BinkFree);
    BinkSetIO(BinkFileOpen);
}
