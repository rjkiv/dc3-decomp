#pragma once

/** Handles MOGG/BIK encryption, as it is unique from BinStream encryption. */
class ByteGrinder {
public:
    ByteGrinder() {}
    virtual ~ByteGrinder() {} // generic dtor
    void GrindArray(long, long, unsigned char *, int, int);
    void Init();
    void HvDecrypt(unsigned char *, unsigned char *, int);

private:
    unsigned long pickOneOf32A(bool, long);
    unsigned long pickOneOf32B(bool, long);
};
