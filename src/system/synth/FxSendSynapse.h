#pragma once
#include "obj/Object.h"
#include "synth/FxSend.h"

/** "synapse effect" */
class FxSendSynapse : public FxSend {
public:
    OBJ_CLASSNAME(FxSendSynapse);
    OBJ_SET_TYPE(FxSendSynapse);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x10);
    NEW_OBJ(FxSendSynapse)

protected:
    FxSendSynapse();

    /** "amount of correction". Ranges from 0 to 1. */
    float mAmount; // 0x60
    /** "amount of correction dependence on error". Ranges from 0 to 1. */
    float mProximityEffect; // 0x64
    /** "focus for amount of correction dependence on error". Ranges from 0 to 1. */
    float mProximityFocus; // 0x68
    /** "target note 1 frequency". Ranges from 10 to 1000. */
    float mNote1Hz; // 0x6c
    /** "target note 2 frequency". Ranges from 10 to 1000. */
    float mNote2Hz; // 0x70
    /** "target note 3 frequency". Ranges from 10 to 1000. */
    float mNote3Hz; // 0x74
    /** "whether the three voices would sing in unison" */
    bool mUnisonTrio; // 0x78
    /** "attack time ms for correction". Ranges from 10 to 500. */
    float mAttackSmoothing; // 0x7c
    /** "release time ms for correction". Ranges from 10 to 500. */
    float mReleaseSmoothing; // 0x80
};
