#include "synth/ByteGrinder.h"

#include <string.h>
#include <stdio.h>
#include <vector>
#include "types.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "utl/Str.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth/tomcrypt/mycrypt.h"

static unsigned char gHvKeyGreen[64] = {
    0x01, 0x22, 0x00, 0x38, 0xd2, 0x01, 0x78, 0x8b, 0xdd, 0xcd, 0xd0, 0xf0, 0xfe,
    0x3e, 0x24, 0x7f, 0x51, 0x73, 0xad, 0xe5, 0xb3, 0x99, 0xb8, 0x61, 0x58, 0x1a,
    0xf9, 0xb8, 0x1e, 0xa7, 0xbe, 0xbf, 0xc6, 0x22, 0x94, 0x30, 0xd8, 0x3c, 0x84,
    0x14, 0x08, 0x73, 0x7c, 0xf2, 0x23, 0xf6, 0xeb, 0x5a, 0x02, 0x1a, 0x83, 0xf3,
    0x97, 0xe9, 0xd4, 0xb8, 0x06, 0x74, 0x14, 0x6b, 0x30, 0x4c, 0x00, 0x91
};

namespace {
    int GetEncMethod(int ver) {
        int ret = 0;
        switch (ver) {
        case 0xc:
        case 0xd:
            ret = 0;
            break;
        case 0xe:
            ret = 1;
            break;
        case 0xf:
            ret = 2;
            break;
        case 0x10:
            ret = 3;
            break;
        default:
            MILO_NOTIFY(" Wrong encryption version passed to ByteGrinder: [%d] !\n", ver);
            break;
        }
        return ret;
    }
}

void ByteGrinder::HvDecrypt(unsigned char *inBlock, unsigned char *outBlock, int moggVer) {
    symmetric_key key;
    int enc_method = GetEncMethod(moggVer);
    void *placeholder = operator new(0x20C);
    rijndael_setup(&gHvKeyGreen[enc_method * 0x10], 0x10, 0, &key);
    rijndael_ecb_decrypt(inBlock, outBlock, &key);
    delete placeholder;
}

DataNode hashTo5Bits(DataArray *da) {
    static unsigned long hashMapping[0x100];
    unsigned long seed = da->Int(1) & 0xFF;
    unsigned long ret = hashMapping[seed];
    bool reseed = da->Size() > 2;
    if (reseed) {
        seed = da->Int(1);
        int max = DIM(hashMapping);
        for (int idx = 0; idx < max; idx++) {
            hashMapping[idx] = (seed >> 3) & 0x1F;
            seed = (seed * 0x19660D) + 0x3C6EF35F;
        }
        return DataNode(kDataInt, 0);
    }
    return DataNode(kDataInt, ret);
}

DataNode hashTo6Bits(DataArray *da) {
    static unsigned long hashMapping[0x100];
    unsigned long seed = da->Int(1) & 0xFF;
    unsigned long ret = hashMapping[seed];
    bool reseed = da->Size() > 2;
    if (reseed) {
        seed = da->Int(1);
        int max = DIM(hashMapping);
        for (int idx = 0; idx < max; idx++) {
            hashMapping[idx] = (seed >> 2) & 0x3F;
            seed = (seed * 0x19660D) + 0x3C6EF35F;
        }
        return DataNode(kDataInt, 0);
    }
    return DataNode(kDataInt, ret);
}

DataNode getRandomSequence32A(DataArray *da) {
    static unsigned long s_seed = 0x521;
    static bool usedUp[0x20];

    bool reseed = da->Size() > 1;
    if (reseed) {
        unsigned long dataint = da->Int(1);
        memset(usedUp, 0, 0x20);
        if (dataint != 0) {
            s_seed = dataint;
        }
        return DataNode(kDataInt, 0x610A660F);
    } else {
        bool loop = true;
        int idx = 0;
        while (loop) {
            s_seed = s_seed * 0x19660D + 0x3C6EF35F;
            idx = (s_seed >> 2 & 0x1F);
            if (usedUp[idx] == false) {
                loop = false;
                usedUp[idx] = true;
            }
        }
        return DataNode(kDataInt, idx);
    }
}

DataNode getRandomSequence32B(DataArray *da) {
    static unsigned long s_seed = 0x303F;
    static bool usedUp[0x20];

    bool reseed = da->Size() > 1;
    if (reseed) {
        unsigned long dataint = da->Int(1);
        memset(usedUp, 0, 0x20);
        if (dataint != 0) {
            s_seed = dataint;
        }
        return DataNode(kDataInt, 0x610A660F);
    } else {
        bool loop = true;
        int idx = 0;
        while (loop) {
            s_seed = s_seed * 0x19660D + 0x3C6EF35F;
            idx = (s_seed >> 2 & 0x1F);
            if (usedUp[idx] == false) {
                loop = false;
                usedUp[idx] = true;
            }
        }
        return DataNode(kDataInt, idx);
    }
}
#define OP_ROT_L(byte, dist)                                                             \
    (unsigned char)((byte << (dist & 31) | byte >> (8 - dist & 31)) & 255)
#define OP_ROT_R(byte, dist)                                                             \
    (unsigned char)((byte >> (dist & 31) | byte << (8 - dist & 31)) & 255)

DataNode op0(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    int operandByte = u8(operand);
    int wByte = u8(w);
    return DataNode(kDataInt, operandByte ^ wByte);
}

DataNode op1(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;
    return DataNode(kDataInt, (w + operand) & 0xFF);
}

DataNode op2(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (working >> (operand & 7)) & 0xFF);
}

DataNode op3(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (working >> (operand == 0)) & 0xFF);
}

DataNode op4(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 notW = w == 0;
    u32 working = (notW << 8) | notW;
    return DataNode(kDataInt, (working >> (operand == 0)) & 0xFF);
}

DataNode op5(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    u32 ret;
    // r5 = u8(r3 NOR r3)
    // r3 = (r31 << 29) >> 29;
    // r5 |= r5 << 8
    // r5 >>= r3
    // r0 = u8(r5)
    ret = u8(~(w | w));
    u32 r3 = (operand << 29) >> 29, r4 = ret << 8;
    ret |= r4;
    ret >>= r3;
    return DataNode(kDataInt, u8(ret));
}

DataNode op6(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    int operandByte = u8(operand);
    int wByte = u8(!w);
    return DataNode(kDataInt, operandByte ^ wByte);
}

DataNode op7(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    return DataNode(kDataInt, ((w == 0) + operand) & 0xFF);
}

DataNode op8(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    u8 operandByte = operand;
    u8 sum = w + operandByte;
    return DataNode(kDataInt, sum ^ operandByte);
}

DataNode op9(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2);
    u8 operandByte = operand;
    u8 wByte = w;
    return DataNode(kDataInt, ((wByte ^ operandByte) + operandByte) & 0xFF);
}

DataNode op10(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> (operand == 0)) ^ operand) & 0xFF);
}

DataNode op11(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> (operand & 7)) ^ operand) & 0xFF);
}

DataNode op12(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> (operand & 7)) + operand) & 0xFF);
}

DataNode op13(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> (operand == 0)) + operand) & 0xFF);
}

DataNode op14(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 1) + operand) & 0xFF);
}

DataNode op15(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 2) + operand) & 0xFF);
}

DataNode op16(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 3) + operand) & 0xFF);
}

DataNode op17(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 4) + operand) & 0xFF);
}

DataNode op18(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 5) + operand) & 0xFF);
}

DataNode op19(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 6) + operand) & 0xFF);
}

DataNode op20(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 7) + operand) & 0xFF);
}

DataNode op21(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 1) ^ operand) & 0xFF);
}

DataNode op22(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 2) ^ operand) & 0xFF);
}

DataNode op23(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 3) ^ operand) & 0xFF);
}

DataNode op24(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 4) ^ operand) & 0xFF);
}

DataNode op25(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 5) ^ operand) & 0xFF);
}

DataNode op26(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 6) ^ operand) & 0xFF);
}

DataNode op27(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 7) ^ operand) & 0xFF);
}

DataNode op28(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (((working >> 5) + operand) ^ operand) & 0xFF);
}

DataNode op29(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (((working >> 3) + operand) ^ operand) & 0xFF);
}

DataNode op30(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (((working >> 3) ^ operand) + operand) & 0xFF);
}

DataNode op31(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, (((working >> 5) ^ operand) + operand) & 0xFF);
}

DataNode op32(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = w ^ 0xFFu;
    u16 tmp = ((working2 | (w << 8)) >> 3);
    return u8(tmp ^ operand);
}

DataNode op33(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = w ^ 0xFFu;
    u16 tmp = ((working2 | (w << 8)) >> 5);
    return u8(tmp ^ operand);
}

DataNode op34(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = w ^ 0xFFu;
    u16 tmp = ((working2 | (w << 8)) >> 2);
    return u8(tmp ^ operand);
}

DataNode op35(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = w ^ 0xFFu;
    u16 tmp = ((working2 | (w << 8)) >> 6);
    return u8(tmp ^ operand);
}

DataNode op36(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working2 = (w << 8) ^ 0xFF00u;
    u32 tmp = ((working2 | w) >> 2);
    return DataNode(kDataInt, (tmp ^ operand) & 0xFF);
}

DataNode op37(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working2 = (w << 8) ^ 0xFF00u;
    u32 tmp = ((working2 | w) >> 5);
    return DataNode(kDataInt, (tmp ^ operand) & 0xFF);
}

DataNode op38(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working2 = (w << 8) ^ 0xFF00u;
    u32 tmp = ((working2 | w) >> 6);
    return DataNode(kDataInt, (tmp ^ operand) & 0xFF);
}

DataNode op39(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working2 = (w << 8) ^ 0xFF00u;
    u32 tmp = ((working2 | w) >> 3);
    return DataNode(kDataInt, (tmp ^ operand) & 0xFF);
}

DataNode op40(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 6);
    return u8(tmp ^ operand);
}

DataNode op41(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 2);
    return u8(tmp ^ operand);
}

DataNode op42(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 3);
    return u8(tmp ^ operand);
}

DataNode op43(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 5);
    return u8(tmp ^ operand);
}

DataNode op44(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x36u);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 2);
    return u8(tmp ^ operand);
}

DataNode op45(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x36u);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 3);
    return u8(tmp ^ operand);
}

DataNode op46(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x36u);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 4);
    return u8(tmp ^ operand);
}

DataNode op47(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x36u);
    u16 working3 = (w << 8);
    u16 tmp = ((working2 | working3) >> 1);
    return u8(tmp ^ operand);
}

DataNode op48(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x63u);
    u16 working3 = (w << 8) ^ 0x5Cu;
    u16 tmp = ((working2 | working3) >> 4);
    return u8(tmp ^ operand);
}

DataNode op49(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x63u);
    u16 working3 = (w << 8) ^ 0x5Cu;
    u16 tmp = ((working2 | working3) >> 3);
    return u8(tmp ^ operand);
}

DataNode op50(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x63u);
    u16 working3 = (w << 8) ^ 0x5Cu;
    u16 tmp = ((working2 | working3) >> 5);
    return u8(tmp ^ operand);
}

DataNode op51(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x63u);
    u16 working3 = (w << 8) ^ 0x5Cu;
    u16 tmp = ((working2 | working3) >> 6);
    return u8(tmp ^ operand);
}

DataNode op52(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8) ^ 0x36u;
    u16 tmp = ((working2 | working3) >> 1);
    return u8(tmp ^ operand);
}

DataNode op53(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u32 working = (w << 8) | w;
    return DataNode(kDataInt, ((working >> 7) ^ operand) & 0xFF);
}

DataNode op54(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8) ^ 0x36u;
    u16 tmp = ((working2 | working3) >> 3);
    return u8(tmp ^ operand);
}

DataNode op55(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x5Cu);
    u16 working3 = (w << 8) ^ 0x36u;
    u16 tmp = ((working2 | working3) >> 5);
    return u8(tmp ^ operand);
}

DataNode op56(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x3Cu);
    u16 working3 = (w << 8) ^ 0x65u;
    u16 tmp = ((working2 | working3) >> 4);
    return u8(tmp ^ operand);
}

DataNode op57(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x3Cu);
    u16 working3 = (w << 8) ^ 0x65u;
    u16 tmp = ((working2 | working3) >> 5);
    return u8(tmp ^ operand);
}

DataNode op58(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x65u);
    u16 working3 = (w << 8) ^ 0x3Cu;
    u16 tmp = ((working2 | working3) >> 6);
    return u8(tmp ^ operand);
}

DataNode op59(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0x3Cu);
    u16 working3 = (w << 8) ^ 0x65u;
    u16 tmp = ((working2 | working3) >> 2);
    return u8(tmp ^ operand);
}

DataNode op60(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0xFFu);
    u16 working3 = (w << 8) ^ 0xAAu;
    u16 tmp = ((working2 | working3) >> 4);
    return u8(tmp ^ operand);
}

DataNode op61(DataArray *msg) {
    u32 operand = msg->Int(2);
    u32 w = msg->Int(1) & 0xFF;

    u16 working2 = (w ^ 0xAAu);
    u16 working3 = (w << 8) ^ 0xFFu;
    u16 tmp = ((working2 | working3) >> 3);
    return u8(tmp ^ operand);
}

DataNode op62(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0xAFu);
    u16 working3 = (w << 8) ^ 0xFAu;
    u16 tmp = ((working2 | working3) >> 5);
    return u8(tmp ^ operand);
}

DataNode op63(DataArray *msg) {
    u32 operand = msg->Int(1);
    u32 w = msg->Int(2) & 0xFF;

    u16 working2 = (w ^ 0xFFu);
    u16 working3 = (w << 8) ^ 0xAFu;
    u16 tmp = ((working2 | working3) >> 6);
    return u8(tmp ^ operand);
}

extern DataArray *DataReadString(const char *);

unsigned long ByteGrinder::pickOneOf32A(bool b, long l) {
    DataArray *a;
    char script[256];
    if (b) {
        sprintf(script, "{xa %d}");
        a = DataReadString(script);
    } else {
        a = DataReadString("{xa}");
    }
    unsigned long result = a->Evaluate(0).Int();
    a->Release();
    return result;
}

unsigned long ByteGrinder::pickOneOf32B(bool b, long l) {
    DataArray *a;
    char script[256];
    if (b) {
        sprintf(script, "{ya %d}");
        a = DataReadString(script);
    } else {
        a = DataReadString("{ya}");
    }
    unsigned long result = a->Evaluate(0).Int();
    a->Release();
    return result;
}

DataNode getRandomLong(DataArray *da) {
    static u32 s_seed = 0x521;
    bool hasOne = da->Size() > 1;
    if (hasOne) {
        s_seed = s_seed * 0x19660D + 0x3C6EF35F;
    }
    return (s32)s_seed;
}

DataNode magicNumberGenerator(DataArray *da) {
    long magic = 0x5c5c5c5c;
    if (da->Int(2) == 2) {
        magic = 0x36363636;
    }
    int idx = da->Int(1);
    long v = ((idx ^ magic) * 0x19660d + 0x3c6ef35f);
    if (da->Int(2) == 1) {
        v = (v * 0x19660d + 0x3c6ef35f);
    }
    return DataNode(kDataInt, v);
}

void ByteGrinder::GrindArray(
    long seedA, long seedB, unsigned char *arrayToGrind, int arrayLen, int moggVersion
) {
    char script[256];
    DataArray *mainScriptArray;

    sprintf(script, "{ma %d 2}", seedA);
    mainScriptArray = DataReadString(script);
    mainScriptArray->Evaluate(0).Int();
    mainScriptArray->Release();

    sprintf(script, "{za %d 2}", seedB);
    mainScriptArray = DataReadString(script);
    mainScriptArray->Evaluate(0).Int();
    mainScriptArray->Release();

    String mainScript;
    int encMethod = GetEncMethod(moggVersion);
    mainScript = "($foo $bar){O68($ix 0){O64{>{O65 $bar}$ix}{O66{ma{O67 $bar $ix}}";
    if (encMethod != 0) {
        mainScript = "($foo $bar){O68($ix 0){O64{>{O65 $bar}$ix}{O66{za{O67 $bar $ix}}";
    }

    pickOneOf32B(true, seedB);
    for (int i = 0; i < 0x20; i++) {
        char block[256];
        char callName[16];
        sprintf(callName, "O%d", pickOneOf32B(false, 0));
        sprintf(block, "(%d{O70 $ix}{O69 $foo{%s{O67 $bar $ix}$foo}})", i, callName);
        mainScript += block;
    }

    if (encMethod != 0) {
        pickOneOf32B(true, seedA);
        for (int i = 0x20; i < 0x40; i++) {
            char block[256];
            char callName[16];
            sprintf(callName, "O%d", pickOneOf32B(false, 0) + 0x20);
            sprintf(block, "(%d{O70 $ix}{O69 $foo{%s{O67 $bar $ix}$foo}})", i, callName);
            mainScript += block;
        }
    }

    mainScript += "}{O70 $ix}}}$foo";
    mainScriptArray = DataReadString(mainScript.c_str());
    for (int i = 0; i < arrayLen; i++) {
        char itoaBuffer[32];
        unsigned char w = arrayToGrind[i];
        String stringArgs("");
        Hx_snprintf(itoaBuffer, sizeof(itoaBuffer), "%d", w);
        stringArgs += itoaBuffer;
        stringArgs += " (";
        for (int j = 0; j < 0x10; j++) {
            Hx_snprintf(itoaBuffer, sizeof(itoaBuffer), "%d", arrayToGrind[j]);
            stringArgs += itoaBuffer;
            stringArgs += " ";
        }
        stringArgs += ")";
        DataArray *args = DataReadString(stringArgs.c_str());
        arrayToGrind[i] = mainScriptArray->ExecuteScript(0, nullptr, args, 0).Int();
        args->Release();
    }
    mainScriptArray->Release();
}

void ByteGrinder::Init() {
    char functionName[0x100];
    // This *must* be written out in reverse to match
    functionName[1] = 'a';
    functionName[0] = 'N';
    functionName[2] = '\0';
    DataRegisterFunc(functionName, getRandomLong);
    functionName[0] = 'h';
    DataRegisterFunc(functionName, magicNumberGenerator);
    functionName[0] = 'm';
    DataRegisterFunc(functionName, hashTo5Bits);
    functionName[0] = 'z';
    DataRegisterFunc(functionName, hashTo6Bits);
    functionName[0] = 'x';
    DataRegisterFunc(functionName, getRandomSequence32A);
    functionName[0] = 'y';
    DataRegisterFunc(functionName, getRandomSequence32B);
    std::vector<DataFunc *> funPtrs;
    funPtrs.push_back(op0);
    funPtrs.push_back(op1);
    funPtrs.push_back(op2);
    funPtrs.push_back(op3);
    funPtrs.push_back(op4);
    funPtrs.push_back(op5);
    funPtrs.push_back(op6);
    funPtrs.push_back(op7);
    funPtrs.push_back(op8);
    funPtrs.push_back(op9);
    funPtrs.push_back(op10);
    funPtrs.push_back(op11);
    funPtrs.push_back(op12);
    funPtrs.push_back(op13);
    funPtrs.push_back(op14);
    funPtrs.push_back(op15);
    funPtrs.push_back(op16);
    funPtrs.push_back(op17);
    funPtrs.push_back(op18);
    funPtrs.push_back(op19);
    funPtrs.push_back(op20);
    funPtrs.push_back(op21);
    funPtrs.push_back(op22);
    funPtrs.push_back(op23);
    funPtrs.push_back(op24);
    funPtrs.push_back(op25);
    funPtrs.push_back(op26);
    funPtrs.push_back(op27);
    funPtrs.push_back(op28);
    funPtrs.push_back(op29);
    funPtrs.push_back(op30);
    funPtrs.push_back(op31);
    pickOneOf32A(true, 0xD5);
    for (int i = 0; i < funPtrs.size(); i++) {
        sprintf(functionName, "O%d", pickOneOf32A(false, 0));
        DataRegisterFunc(functionName, funPtrs[i]);
    }
    funPtrs.clear();
    funPtrs.push_back(op32);
    funPtrs.push_back(op33);
    funPtrs.push_back(op34);
    funPtrs.push_back(op35);
    funPtrs.push_back(op36);
    funPtrs.push_back(op37);
    funPtrs.push_back(op38);
    funPtrs.push_back(op39);
    funPtrs.push_back(op40);
    funPtrs.push_back(op41);
    funPtrs.push_back(op42);
    funPtrs.push_back(op43);
    funPtrs.push_back(op44);
    funPtrs.push_back(op45);
    funPtrs.push_back(op46);
    funPtrs.push_back(op47);
    funPtrs.push_back(op48);
    funPtrs.push_back(op49);
    funPtrs.push_back(op50);
    funPtrs.push_back(op51);
    funPtrs.push_back(op52);
    funPtrs.push_back(op53);
    funPtrs.push_back(op54);
    funPtrs.push_back(op55);
    funPtrs.push_back(op56);
    funPtrs.push_back(op57);
    funPtrs.push_back(op58);
    funPtrs.push_back(op59);
    funPtrs.push_back(op60);
    funPtrs.push_back(op61);
    funPtrs.push_back(op62);
    funPtrs.push_back(op63);
    pickOneOf32A(true, 0x23E);
    for (int i = 0; i < funPtrs.size(); i++) {
        sprintf(functionName, "O%d", pickOneOf32A(false, 0) + 32);
        DataRegisterFunc(functionName, funPtrs[i]);
    }
}
