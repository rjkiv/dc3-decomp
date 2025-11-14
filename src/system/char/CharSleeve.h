#pragma once
#include "char/CharPollable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

class CharSleeve : public RndHighlightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharSleeve();
    OBJ_CLASSNAME(CharSleeve)
    OBJ_SET_TYPE(CharSleeve)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(Hmx::Object const *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    // RndHighlightable
    virtual void Highlight();

    NEW_OBJ(CharSleeve)

    ObjPtr<RndTransformable> mSleeve; // 0x10
    ObjPtr<RndTransformable> mTopSleeve; // 0x24
    float unk38;
    float unk3c;
    float unk40;
    u32 unk44;
    float unk48;
    float unk4c;
    float unk50;
    u32 unk54;
    float unk58;
    float mInertia; // unk5c
    float mGravity; // 0x60
    float mRange; // 0x64
    float mNegLength; // 0x68
    float mPosLength; // 0x6c
    float mStiffness; // 0x70

protected:
    CharSleeve();
};
