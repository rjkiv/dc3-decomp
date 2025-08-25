#pragma once
#include "math/Key.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include <vector>
#include "types.h"

class RndRibbon : public RndPollable, public RndDrawable {
public:
    virtual void Poll();
    // Hmx::Object
    virtual ~RndRibbon() {}
    OBJ_CLASSNAME(Ribbon);
    OBJ_SET_TYPE(Ribbon);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

protected:
    RndRibbon();
    void ExposeMesh(void);
    void SetActive(bool);

    float mDecay; // 0x48
    // RndMesh *unk_0x4c; // i think???
    int unk_0x4c;
    ObjPtr<RndMat> mMaterial;
    bool unk_0x68;
    u8 pad1[0xb];

    bool mActive; // 0x70
    std::vector<Key<Transform> > mTransforms;

    float mWidth; // ?
    int mNumSides; // ?????
    int mNumSegments; // ?
    float mFollowWeight; // ?
    bool mTaper; // ?

    u8 pad2[0x20];
};
