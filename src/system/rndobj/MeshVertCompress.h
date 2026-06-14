#pragma once
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "math/Vec.h"
#include "utl/BinStream.h"

// size 0x24
struct CompressedVertex_Xbox {
    float unk0, unk4, unk8; // packed pos
    unsigned int unkc; // packed color
    unsigned int unk10; // packed tex
    unsigned int unk14; // packed norm
    unsigned int unk18; // packed unk50
    unsigned int unk1c; // packed boneweights
    unsigned int unk20; // packed boneIndices
};

#define kBitsOutput 0x20

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
    int kOffsetW = bitsX + bitsY + bitsZ;
    MILO_ASSERT(kOffsetW + bitsW == kBitsOutput, 0x4E);
}

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void FillCompressedVertex(
    CompressedVertex_Xbox &vertXbox, const RndMesh::Vert &vert, bool b3
) {
    vertXbox.unkc = ((unsigned int)(vert.color.alpha * 255.0f) & 0xFF) << 24
        | ((unsigned int)(vert.color.red * 255.0f) & 0xFF) << 16
        | ((unsigned int)(vert.color.green * 255.0f) & 0xFF) << 8
        | ((unsigned int)(vert.color.blue * 255.0f) & 0xFF);
    PackVector(vertXbox.unk1c, vert.boneWeights, 10, 10, 10, 2, false);
    vertXbox.unk0 = vert.pos.x;
    vertXbox.unk4 = vert.pos.y;
    vertXbox.unk8 = vert.pos.z;
    vertXbox.unk10 =
        (FloatToHalfFloat(vert.tex.x) << 0x10) | FloatToHalfFloat(vert.tex.y);
    Vector4 norm(vert.norm.x, vert.norm.y, vert.norm.z, 0);
    PackVector(vertXbox.unk14, norm, 10, 10, 10, 2, true);
    PackVector(vertXbox.unk18, vert.tangent, 10, 10, 10, 2, true);
    vertXbox.unk20 =
        ((((vert.boneIndices[3] & 0xffffffU) * 0x100 + vert.boneIndices[2] & 0xffffff)
                  * 0x100
              + vert.boneIndices[1]
          & 0xffffffff)
         << 8)
        + vert.boneIndices[0];
}

inline void SaveCompressedVertex(const CompressedVertex_Xbox &vert, BinStream &bs) {
    bs << vert.unk0 << vert.unk4 << vert.unk8;
    bs << vert.unkc;
    bs << vert.unk10;
    bs << vert.unk14;
    bs << vert.unk18;
    bs << vert.unk1c;
    bs << vert.unk20;
}
