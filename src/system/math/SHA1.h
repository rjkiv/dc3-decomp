#pragma once
#include "utl/BinStream.h"

class CSHA1 {
public:
    struct Digest {
        unsigned char digits[20];
        void Copy(unsigned char *) const;
        void ReportHash(char *, unsigned char) const;
    };

    CSHA1();
    ~CSHA1();
    void Reset() {
        m_state[0] = 0x67452301;
        m_state[1] = 0xEFCDAB89;
        m_state[2] = 0x98BADCFE;
        m_state[3] = 0x10325476;
        m_state[4] = 0xC3D2E1F0;
        m_count[0] = 0;
        m_count[1] = 0;
    }
    void Update(const unsigned char *, unsigned int);
    const Digest &Final();

private:
    union SHA1_WORKSPACE_BLOCK {
        unsigned char c[64];
        unsigned long l[16];
    };
    void Transform(unsigned int *, const unsigned char *);

    unsigned int m_state[5]; // 0x0
    unsigned int m_count[2]; // 0x14
    unsigned int m_reserved1[1]; // 0x1c
    unsigned char m_buffer[64]; // 0x20
    Digest m_digest; // 0x60
    unsigned int m_reserved2[3]; // 0x74
    unsigned char m_workspace[64]; // 0x80
    SHA1_WORKSPACE_BLOCK *m_block; // 0xc0
};

BinStream &operator<<(BinStream &, const CSHA1::Digest &);
BinStream &operator>>(BinStream &, CSHA1::Digest &);
