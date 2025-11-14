#pragma once
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "char/CharClipGroup.h"
#include "char/CharWeightable.h"
#include "char/CharPollable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "utl/Symbol.h"

class CharDriver : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharDriver();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CharDriver);
    OBJ_SET_TYPE(CharDriver);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    ObjectDir *ClipDir() const { return mClips; }
    CharClip *FirstClip();

protected:
    CharDriver();

    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x30
    /** "pointer to clips object" */
    ObjPtr<ObjectDir> mClips; // 0x44
    int unk58;
    ObjPtr<CharClip> unk5c;
    ObjPtr<Hmx::Object> unk70;
    ObjPtr<CharClipGroup> unk84;
    bool unk98;
    Symbol unk9c;
    DataNode unka0;
    float unka8;
    bool unkac;
    float unkb0;
    float unkb4;
    Symbol unkb8;
    int unkbc;
    int unkc0;
    bool unkc4;
    std::map<CharClip *, float> unkc8;
};
