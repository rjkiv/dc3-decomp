#pragma once
#include "os/File.h"
#include "utl/EncryptXTEA.h"

struct BINKENCRYPTIONHEADER {
    // total size: 0x38
    unsigned int mSignature; // offset 0x0, size 0x4
    unsigned int mVersion; // offset 0x4, size 0x4
    unsigned int mKeyIndex; // offset 0x8, size 0x4
    unsigned int mMagicA; // offset 0xC, size 0x4
    unsigned int mMagicB; // offset 0x10, size 0x4
    unsigned long long mNonce[2]; // offset 0x18, size 0x10
    unsigned char mKeyMask[16]; // offset 0x28, size 0x10
};

struct BINKFILE {
    // total size: 0x70
    File *pFile; // offset 0x0, size 0x4
    unsigned int iCloseFile; // offset 0x4, size 0x4
    unsigned char *pBuffer; // offset 0x8, size 0x4
    unsigned char *pBufEnd; // offset 0xC, size 0x4
    unsigned char *pBufPos; // offset 0x10, size 0x4
    unsigned char *pBufBack; // offset 0x14, size 0x4
    unsigned int iBufEmpty; // offset 0x18, size 0x4
    unsigned int iFileBufPos; // offset 0x1C, size 0x4
    unsigned int iSimulateBPS; // offset 0x20, size 0x4
    unsigned int iTimer; // offset 0x24, size 0x4
    unsigned int iHeaderSize; // offset 0x28, size 0x4
    struct BINKENCRYPTIONHEADER mEncryptionHeader; // offset 0x30, size 0x38
    XTEABlockEncrypter *pXTEADecrypter; // offset 0x68, size 0x4
};

void BinkInit();
