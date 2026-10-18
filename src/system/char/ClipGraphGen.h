#pragma once
#include "char/CharClip.h"
#include "char/ClipDistMap.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/TextStream.h"

class ClipGraphGenerator : public Hmx::Object {
public:
    class Report { // in pdb but doesnt get used? ok
    public:
        struct Node {
            int a; // 0x0
            int b; // 0x4
            ClipDistMap::Node n; // 0x8
        };

    protected:
        TextStream *mOut; // 0x0
        String mWarnings; // 0x4
        String mErrors; // 0xc
        int mMostNodes; // 0x14
        String mMostNodesWarning; // 0x18
        std::vector<Node> mTopWorst; // 0x20
        std::vector<Node> mTopBest; // 0x2c
        int mTotal; // 0x38
        CharClip *mLastA; // 0x3c
        bool mFirstError; // 0x40
        bool mFirstWarning; // 0x41
    };

    // Hmx::Object
    virtual ~ClipGraphGenerator();
    virtual DataNode Handle(DataArray *, bool);

    ClipGraphGenerator();
    ClipDistMap *
    GeneratePair(CharClip *, CharClip *, ClipDistMap::Node *, ClipDistMap::Node *);

    NEW_OBJ(ClipGraphGenerator);

protected:
    const DataArray *mGraphData; // 0x2c
    ClipDistMap *mDmap; // 0x30
    CharClip *mClipA; // 0x34
    CharClip *mClipB; // 0x38

    DataNode OnGenerateTransitions(DataArray *);
};
