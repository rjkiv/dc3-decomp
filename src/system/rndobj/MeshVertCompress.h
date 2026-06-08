#pragma once
#include "rndobj/Mesh.h"
#include "math/Vec.h"
#include "utl/BinStream.h"

// size 0x24
struct CompressedVertex_Xbox {
    float unk0, unk4, unk8; // packed pos
    int unkc; // packed color
    unsigned int unk10; // packed tex
    unsigned int unk14; // packed norm
    unsigned int unk18; // packed unk50
    unsigned int unk1c;
    unsigned int unk20; // packed boneIndices
};

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void PackVector(
    unsigned int &,
    const Vector4 &,
    unsigned char,
    unsigned char,
    unsigned char,
    unsigned char,
    bool
);

// defined twice in rndobj and rnddx9, guess they forgot to inline it
void FillCompressedVertex(
    CompressedVertex_Xbox &vertXbox, const RndMesh::Vert &vert, bool b3
) {
    vertXbox.unkc = vert.color.PackAlpha();
    PackVector(vertXbox.unk1c, vert.boneWeights, 10, 10, 10, 2, false);
    vertXbox.unk0 = vert.pos.x;
    vertXbox.unk4 = vert.pos.y;
    vertXbox.unk8 = vert.pos.z;
    unsigned short texX = vert.tex.x;
    unsigned short texY = vert.tex.y;
    vertXbox.unk10 = (texX << 0x10) | texY;
    Vector4 norm(vert.norm.x, vert.norm.y, vert.norm.z, 0);
    PackVector(vertXbox.unk14, norm, 10, 10, 10, 2, true);
    PackVector(vertXbox.unk18, vert.unk50, 10, 10, 10, 2, true);
    // pack bone indices into vertxbox.unk20
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
