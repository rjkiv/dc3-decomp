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
            for (XTEABlock *it = (XTEABlock *)binkFile->pBufBack;
                 it < (XTEABlock *)(binkFile->pBufBack + bytes);
                 it++) {
                it->mData[0] = EndianSwap(it->mData[0]);
                it->mData[1] = EndianSwap(it->mData[1]);
                XTEABlock decrypted;
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
            TheSynth->Grinder().GrindArray(
                pEncryptionHeader->mMagicA, pEncryptionHeader->mMagicB, masterKey, 16, 12
            );
            for (int i = 0; i < 16; i++) {
                masterKey[i] ^= pEncryptionHeader->mKeyMask[i];
            }
            EndianSwapBlock((unsigned int *)masterKey, 4);
            pBinkFileInfo->pXTEADecrypter->SetKey(masterKey);
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

unsigned int BinkFileReadFrame(
    struct BINKIO *pBinkIO,
    unsigned int iFrameNum,
    int iOffset,
    void *pDest,
    unsigned int iReadSize
);

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
    unsigned int mask = size & 0xFFFF8000;
    BINKFILE *binkFile = (BINKFILE *)pBinkIO->iodata;
    binkFile->pBuffer = (unsigned char *)buffer;
    binkFile->pBufPos = (unsigned char *)buffer;
    binkFile->pBufBack = (unsigned char *)buffer;
    binkFile->pBufEnd = (unsigned char *)buffer + mask;
    binkFile->iBufEmpty = mask;
    pBinkIO->BufSize = mask;
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
