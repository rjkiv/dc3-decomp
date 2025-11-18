#pragma once
#include "char/CharClip.h"
#include "char/ClipDistMap.h"
#include "obj/Data.h"
#include "obj/Object.h"

class ClipGraphGenerator : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~ClipGraphGenerator();
    virtual DataNode Handle(DataArray *, bool);

    ClipGraphGenerator();
    ClipDistMap *
    GeneratePair(CharClip *, CharClip *, ClipDistMap::Node *, ClipDistMap::Node *);

    NEW_OBJ(ClipGraphGenerator);

    const DataArray *unk2c;
    ClipDistMap *mDmap; // 0x30
    CharClip *mClipA; // 0x34
    CharClip *mClipB; // 0x38

protected:
    DataNode OnGenerateTransitions(DataArray *);
};
