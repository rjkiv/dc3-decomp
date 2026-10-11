#pragma once
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include <list>

/** "Class to play back clips, has a tree view of layers" */
class HamDriver : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    struct Layer {
        Layer() : blendStart(-kHugeFloat) {}
        virtual ~Layer() {}
        virtual void Eval(float) = 0;
        virtual void Play(CharBones &) = 0;
        virtual bool Replace(ObjRef *, Hmx::Object *) = 0;
        virtual CharClip *FirstClip() = 0;
        virtual void OffsetSec(float);

        MEM_OVERLOAD(Layer, 0x27)

        float blendStart; // 0x4 - beat?
        float usedWeight;
    };

    struct LayerArray : public Layer {
        LayerArray() { name[0] = 0; }
        virtual ~LayerArray() { Clear(); }
        virtual void Eval(float);
        virtual void Play(CharBones &);
        virtual bool Replace(ObjRef *, Hmx::Object *);
        virtual CharClip *FirstClip();
        virtual void OffsetSec(float);

        void Clear();

        char name[0x20];
        std::list<Layer *> layers;
    };

    struct LayerClip : public Layer {
        LayerClip(Hmx::Object *);
        virtual void Eval(float);
        virtual void Play(CharBones &);
        virtual bool Replace(ObjRef *, Hmx::Object *);
        virtual CharClip *FirstClip();
        virtual void OffsetSec(float);

        float startBeat;
        ObjOwnerPtr<CharClip> clip; // 0x10
    };

    // Hmx::Object
    virtual ~HamDriver();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(HamDriver);
    OBJ_SET_TYPE(HamDriver);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit() { RndPollable::Exit(); }
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x18)
    NEW_OBJ(HamDriver)

    void Clear();
    void SetClipWeightMap();
    LayerClip *NewLayerClip();
    void OffsetSec(float);
    CharClip *FirstClip();
    LayerArray &Layers() { return mLayers; }
    const std::map<CharClip *, float> &ClipWeights() const { return mClipWeights; }

protected:
    HamDriver();

    float Display(float);
    float DisplayRecurse(Layer *, int, float);
    void SetClipMapRecurse(HamDriver::Layer *);

    /** "The CharBones object to add into." */
    ObjPtr<CharBonesObject> mBones; // 0x30
    LayerArray mLayers; // 0x44
    float mOldBeat; // 0x78 - beat?
    std::map<CharClip *, float> mClipWeights; // 0x7c
};
