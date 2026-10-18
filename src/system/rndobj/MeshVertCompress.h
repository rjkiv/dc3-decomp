#pragma once
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "math/Vec.h"
#include "utl/BinStream.h"
#include "xdk/D3D9.h"
#include "xdk/win_types.h"

// size 0x24
struct CompressedVertex_Xbox {
    enum {
        kVersion = 1
    };
    float mPosX; // 0x0
    float mPosY; // 0x4
    float mPosZ; // 0x8
    unsigned int mColor; // 0xc
    unsigned int mTex; // 0x10
    unsigned int mNormal; // 0x14
    unsigned int mTangent; // 0x18
    unsigned int mWeight; // 0x1c
    unsigned int mIndices; // 0x20
};

#define kBitsOutput 32

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void PackVector(
    unsigned int &outValue,
    const Vector4 &vec,
    unsigned char bitsX,
    unsigned char bitsY,
    unsigned char bitsZ,
    unsigned char bitsW,
    bool isSigned
) {
    MILO_ASSERT((bitsX + bitsY + bitsZ + bitsW) == kBitsOutput, 0x39);
    int kOffsetX = 0;
    int kOffsetY = bitsX;
    int kOffsetZ = bitsX + bitsY;
    int kOffsetW = bitsX + bitsY + bitsZ;
    MILO_ASSERT(kOffsetW + bitsW == kBitsOutput, 0x4E);
}

// defined twice in rndobj and rnddx9, guess they forgot to inline it
static void
FillCompressedVertex(CompressedVertex_Xbox &vertXbox, const RndMesh::Vert &vert, bool b3) {
    const Hmx::Color &c = vert.color;
    vertXbox.mColor = D3DCOLOR_COLORVALUE(c.red, c.green, c.blue, c.alpha);
    PackVector(vertXbox.mWeight, vert.boneWeights, 10, 10, 10, 2, false);
    vertXbox.mPosX = vert.pos.x;
    vertXbox.mPosY = vert.pos.y;
    vertXbox.mPosZ = vert.pos.z;
    vertXbox.mTex = (FloatToHalfFloat(vert.tex.x) << 0x10) | FloatToHalfFloat(vert.tex.y);
    Vector4 norm(vert.norm.x, vert.norm.y, vert.norm.z, 0);
    PackVector(vertXbox.mNormal, norm, 10, 10, 10, 2, true);
    PackVector(vertXbox.mTangent, vert.tangent, 10, 10, 10, 2, true);
    // terrible way of basically just packing the indices, 8 bits per index
    // if you do the way MAKEFOURCC does, with << 24, << 16, etc, it won't match
    vertXbox.mIndices =
        (((((vert.bone3 << 8) + vert.bone2) << 8) + vert.bone1) << 8) + vert.bone0;
}

inline void SaveCompressedVertex(const CompressedVertex_Xbox &vert, BinStream &bs) {
    bs << vert.mPosX << vert.mPosY << vert.mPosZ;
    bs << vert.mColor;
    bs << vert.mTex;
    bs << vert.mNormal;
    bs << vert.mTangent;
    bs << vert.mWeight;
    bs << vert.mIndices;
}
