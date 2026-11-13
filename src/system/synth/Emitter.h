#pragma once
#include "rndobj/Draw.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "synth/Sfx.h"
#include "utl/MemMgr.h"

/** "A 3D positional emitter.  The volume and surround-panning of
 *  a sfx are controlled by its position relative to a listener (usually the
 *  camera)." */
class SynthEmitter : public RndTransformable, public RndDrawable, public RndPollable {
public:
    // Hmx::Object
    virtual ~SynthEmitter();
    OBJ_CLASSNAME(SynthEmitter);
    OBJ_SET_TYPE(SynthEmitter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void Highlight() { RndDrawable::Highlight(); }
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x1B);
    NEW_OBJ(SynthEmitter);
    REGISTER_OBJ(SynthEmitter);

private:
    void CheckLoadResources();

protected:
    SynthEmitter();

    /** "sfx this emitter should play" */
    ObjPtr<Sfx> mSfx; // 0x108
    ObjPtr<SfxInst> mInst; // 0x11c
    /** "object representing the listener's position" */
    ObjPtr<RndTransformable> mListener; // 0x130
    bool mEnabled; // 0x144
    /** "volume and pan are fixed inside this radius." */
    float mRadInner; // 0x148
    /** "the sound starts playing when you cross inside this radius." */
    float mRadOuter; // 0x14c
    /** "volume at inner radius (and inside)" */
    float mVolInner; // 0x150
    /** "volume at outer radius, in dB" */
    float mVolOuter; // 0x154
};
