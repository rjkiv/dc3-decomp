#pragma once
#include "obj/Object.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

class PhotoSpotlightPositioner : public RndPollable {
public:
    // Hmx::Object
    virtual ~PhotoSpotlightPositioner();
    OBJ_CLASSNAME(PhotoSpotlightPositioner);
    OBJ_SET_TYPE(PhotoSpotlightPositioner);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(PhotoSpotlightPositioner)
    static void Init();

    Vector3 GetImagePos(Vector2) const;

protected:
    PhotoSpotlightPositioner();

    int mPlayer; // 0x8
    ObjPtr<RndGroup> mSpotlight; // 0xc
    ObjPtr<RndMesh> mRefImage; // 0x20
};
