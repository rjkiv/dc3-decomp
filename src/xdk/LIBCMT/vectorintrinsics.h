#pragma once

// A C representation of a VMX register.
typedef struct __declspec(intrin_type) __declspec(align(16)) __vector4 { /* Size=0x10 */
    union {
        /* 0x0000 */ float vector4_f32[4];
        /* 0x0000 */ unsigned int vector4_u32[4];
        struct {
            /* 0x0000 */ float x;
            /* 0x0004 */ float y;
            /* 0x0008 */ float z;
            /* 0x000c */ float w;
        };
        /* 0x0000 */ float v[4];
        /* 0x0000 */ unsigned int u[4];
    };
} __vector4;
typedef __vector4 XMVECTOR;

struct XMVECTORF32 { /* Size=0x10 */
    union {
        /* 0x0000 */ float f[4];
        /* 0x0000 */ __vector4 v;
    };
};

struct XMVECTORI32 { /* Size=0x10 */
    union {
        /* 0x0000 */ int i[4];
        /* 0x0000 */ __vector4 v;
    };
};

struct XMVECTORU32 { /* Size=0x10 */
    union {
        /* 0x0000 */ unsigned int u[4];
        /* 0x0000 */ __vector4 v;
    };
};

typedef struct _XMMATRIX { /* Size=0x40 */
    union {
        /* 0x0000 */ XMVECTOR r[4];
        struct {
            /* 0x0000 */ float _11;
            /* 0x0004 */ float _12;
            /* 0x0008 */ float _13;
            /* 0x000c */ float _14;
            /* 0x0010 */ float _21;
            /* 0x0014 */ float _22;
            /* 0x0018 */ float _23;
            /* 0x001c */ float _24;
            /* 0x0020 */ float _31;
            /* 0x0024 */ float _32;
            /* 0x0028 */ float _33;
            /* 0x002c */ float _34;
            /* 0x0030 */ float _41;
            /* 0x0034 */ float _42;
            /* 0x0038 */ float _43;
            /* 0x003c */ float _44;
        };
        /* 0x0000 */ float m[4][4];
    };

    _XMMATRIX(const float *);
    // clang-format off
    _XMMATRIX(
        float, float, float, float,
        float, float, float, float,
        float, float, float, float,
        float, float, float, float
    );
    // clang-format on
    _XMMATRIX(const XMVECTOR, const XMVECTOR, const XMVECTOR, const XMVECTOR);
    _XMMATRIX();
    float &operator()(unsigned int, unsigned int);
    float operator()(unsigned int, unsigned int) const;
    _XMMATRIX &operator=(const _XMMATRIX &);
    _XMMATRIX &operator*=(const _XMMATRIX &);
    _XMMATRIX operator*(const _XMMATRIX &) const;
} XMMATRIX;

#ifdef __cplusplus
extern "C" {
#endif

// VMX128 Intrinsics let's goooooooo

// Based on what I found from pdbs,
// lvx and stvx are the only two VMX128 intrinsics
// that actually require an explicit implementation.
// Everything else, you can just add the declaration below
// and it'll just work:tm:

static inline XMVECTOR __lvx(const void *base, int offset) {
    const char *ret = (const char *)base + offset;
    return *(XMVECTOR *)ret;
}

static inline void __stvx(XMVECTOR vSrc, void *base, int offset) {
    char *dest = (char *)base + offset;
    XMVECTOR *vDst = (XMVECTOR *)dest;
    *vDst = vSrc;
}

// To add an intrinsic for a particular opcode, please consult:
// https://github.com/encounter/powerpc-rs/blob/main/isa.yaml#L4635
// Each VMX register corresponds to an XMVECTOR.
// Try to infer the function signature based off the args of the opcode.
// You'll know you got it right if you can call your function
// and the actual VMX128 PPC instruction shows up.

// Vector128 Multiply Add Floating Point
XMVECTOR __vmaddfp(XMVECTOR mul1, XMVECTOR mul2, XMVECTOR addend);

// Vector128 Splat Word
XMVECTOR __vspltw(XMVECTOR vSrcA, unsigned int uImmed);

#ifdef __cplusplus
}
#endif
