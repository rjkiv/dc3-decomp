#include "utl/EncryptXTEA.h"
#include <cstring>

XTEABlockEncrypter::XTEABlockEncrypter() {
    mNonce[0] = 0;
    mNonce[1] = 0;
}

void XTEABlockEncrypter::SetKey(const unsigned char *uc) { memcpy(mKey, uc, 0x10); }

void XTEABlockEncrypter::SetNonce(const unsigned long long *nonce, unsigned int shift) {
    mNonce[0] = nonce[0] + shift;
    mNonce[1] = nonce[1] + shift;
}

void XTEABlockEncrypter::Encrypt(const XTEABlock *in, XTEABlock *out) {
    unsigned int *key = mKey;
    unsigned long long *nonce = mNonce;
    unsigned long offset = (char *)out - (char *)in;
    for (int i = 0; i < 2; i++) {
        *(unsigned long long *)(offset + (char *)in) =
            *(unsigned long long *)in ^ Encipher(*nonce, key);
        *nonce += 1;
        nonce++;
        in = (const XTEABlock *)((char *)in + 8);
    }
}

unsigned long long
XTEABlockEncrypter::Encipher(unsigned long long nonce, unsigned int *key) {
    unsigned long v1 = nonce & 0xFFFFFFFF;
    unsigned long v2 = nonce >> 32;
    unsigned int sum = 0;
    for (int i = 0; i < 4; i++) {
        v1 += (v2 + (v2 << 4 ^ v2 >> 5)) ^ sum + (key[(sum & 3)]);
        sum += 0x9E3779B9;
        v2 += (v1 + (v1 << 4 ^ v1 >> 5)) ^ sum + key[(sum >> 11) & 3];
    }
    return static_cast<unsigned long long>(v2) << 32
        | static_cast<unsigned long long>(v1) & 0xFFFFFFFF;
}
