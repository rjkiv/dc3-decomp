#pragma once
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "math/Vec.h"
#include "utl/BinStream.h"
#include "xdk/D3D9.h"
#include "xdk/win_types.h"

// size 0x24
struct CompressedVertex_Xbox {
    float x, y, z; // 0x0 - packed pos
    DWORD color; // 0xc
    DWORD tex; // 0x10
    unsigned int norm; // 0x14
    unsigned int tangent; // 0x18
    unsigned int boneWeights; // 0x1c
    unsigned int boneIndices; // 0x20
};

#define kBitsOutput 32

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void PackVector(
    unsigned int &packed,
    const Vector4 &vec,
    unsigned char bitsX,
    unsigned char bitsY,
    unsigned char bitsZ,
    unsigned char bitsW,
    bool
) {
    MILO_ASSERT((bitsX + bitsY + bitsZ + bitsW) == kBitsOutput, 0x39);
    int kOffsetX = 0;
    int kOffsetY = bitsX;
    int kOffsetZ = bitsX + bitsY;
    int kOffsetW = bitsX + bitsY + bitsZ;
    MILO_ASSERT(kOffsetW + bitsW == kBitsOutput, 0x4E);
}

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void FillCompressedVertex(
    CompressedVertex_Xbox &vertXbox, const RndMesh::Vert &vert, bool b3
) {
    vertXbox.color = D3DCOLOR_COLORVALUE(
        vert.color.red, vert.color.green, vert.color.blue, vert.color.alpha
    );
    PackVector(vertXbox.boneWeights, vert.boneWeights, 10, 10, 10, 2, false);
    vertXbox.x = vert.pos.x;
    vertXbox.y = vert.pos.y;
    vertXbox.z = vert.pos.z;
    vertXbox.tex = (FloatToHalfFloat(vert.tex.x) << 0x10) | FloatToHalfFloat(vert.tex.y);
    Vector4 norm(vert.norm.x, vert.norm.y, vert.norm.z, 0);
    PackVector(vertXbox.norm, norm, 10, 10, 10, 2, true);
    PackVector(vertXbox.tangent, vert.tangent, 10, 10, 10, 2, true);
    // terrible way of basically just packing the indices, 8 bits per index
    // if you do the way MAKEFOURCC does, with << 24, << 16, etc, it won't match
    vertXbox.boneIndices =
        (((((vert.boneIndices[3] << 8) + vert.boneIndices[2]) << 8) + vert.boneIndices[1])
         << 8)
        + vert.boneIndices[0];
}

inline void SaveCompressedVertex(const CompressedVertex_Xbox &vert, BinStream &bs) {
    bs << vert.x << vert.y << vert.z;
    bs << vert.color;
    bs << vert.tex;
    bs << vert.norm;
    bs << vert.tangent;
    bs << vert.boneWeights;
    bs << vert.boneIndices;
}
