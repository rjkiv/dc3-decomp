#pragma once
#include "char/CharDriver.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

class CharDriverMidi : public CharDriver {
public:
    virtual ~CharDriverMidi();
    OBJ_CLASSNAME(CharDriverMidi)
    OBJ_SET_TYPE(CharDriverMidi)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);
    virtual void Enter();
    virtual void Exit();

    bool unke0; // unsure why this is here, its def size 0x1
    Symbol mParser; // 0xe4
    Symbol mFlagParser; // 0xe8
    int mClipFlags; // 0xec
    float mBlendOverridePct; // 0xf0

protected:
    CharDriverMidi();

    DataNode OnMidiParser(DataArray *);
    DataNode OnMidiParserFlags(DataArray *);
    DataNode OnMidiParserGroup(DataArray *);
};
