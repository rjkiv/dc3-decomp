#pragma once
#include "math/SHA1.h"
#include "utl/MemMgr.h"

class StreamChecksum {
private:
    int mState; // this is an enum - what the state enums are? that's anybody's guess
    CSHA1 mSHA1;

public:
    StreamChecksum() : mState(0), mSHA1() {}
    ~StreamChecksum() {}
    void Begin();
    void Update(const unsigned char *, unsigned int);
    void End();
    void GetHash(unsigned char *);
};

class StreamChecksumValidator {
private:
    void HandleError(const char *);
    bool SetFileChecksum(bool);
    bool ValidateChecksum(const unsigned char *);

    StreamChecksum mStreamChecksum;
    const unsigned char *mSignature;
    const char *mFile;

public:
    StreamChecksumValidator() : mStreamChecksum(), mSignature(0), mFile(0) {}
    ~StreamChecksumValidator() {}

    MEM_OVERLOAD(StreamChecksumValidator, 0x3D);
    bool Begin(const char *, bool);
    void Update(const unsigned char *, unsigned int);
    void End();
    bool Validate();
};
