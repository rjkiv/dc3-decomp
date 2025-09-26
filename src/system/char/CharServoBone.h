#pragma once
#include "char/CharBonesMeshes.h"
#include "char/CharPollable.h"
#include "rndobj/Highlight.h"

class CharServoBone : public RndHighlightable,
                      public CharPollable,
                      public CharBonesMeshes {
public:
    // Hmx::Object
    virtual ~CharServoBone();
    OBJ_CLASSNAME(CharServoBone);
    OBJ_SET_TYPE(CharServoBone);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight() {}
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);
    // CharBonesMeshes
    virtual void ReallocateInternal();

protected:
    CharServoBone();
};
