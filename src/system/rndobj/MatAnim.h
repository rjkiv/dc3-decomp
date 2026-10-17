#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

/**
 * @brief A class for animating Mats.
 * Original _objects description:
 * "MatAnim objects animate material properties."
 */
class RndMatAnim : public RndAnimatable {
public:
    class TexPtr : public ObjPtr<RndTex> {
    public:
        TexPtr(RndTex *tex = nullptr) : ObjPtr<RndTex>(sOwner, tex) {}
    };

    class TexKeys : public Keys<TexPtr, RndTex *> {
    public:
        TexKeys(Hmx::Object *owner) : mOwner(owner) {}
        void operator=(const TexKeys &);
        int Add(RndTex *, float, bool);

        Hmx::Object *mOwner;
    };

    // Hmx::Object
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(MatAnim);
    OBJ_SET_TYPE(MatAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget() { return mMat; }
    virtual void SetKey(float);

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(RndMatAnim)
    REGISTER_OBJ(RndMatAnim)

    Keys<Vector3, Vector3> &TransKeys() { return mKeysOwner->mTransKeys; }
    Keys<Vector3, Vector3> &ScaleKeys() { return mKeysOwner->mScaleKeys; }
    TexKeys &GetTexKeys() { return mKeysOwner->mTexKeys; }
    Keys<Vector3, Vector3> &RotKeys() { return mKeysOwner->mRotKeys; }
    Keys<float, float> &AlphaKeys() { return mKeysOwner->mAlphaKeys; }
    Keys<Hmx::Color, Hmx::Color> &ColorKeys() { return mKeysOwner->mColorKeys; }
    RndMatAnim *KeysOwner() const { return mKeysOwner; }

protected:
    RndMatAnim();

    static Hmx::Object *sOwner;

    void LoadStage(BinStreamRev &);
    void LoadStages(BinStreamRev &);

    /** The Mat to animate. */
    ObjPtr<RndMat> mMat; // 0x10
    /** The MatAnim that owns all of these keys. */
    ObjOwnerPtr<RndMatAnim> mKeysOwner; // 0x24
    /** The collection of Color keys. */
    Keys<Hmx::Color, Hmx::Color> mColorKeys; // 0x38
    /** The collection of alpha keys. */
    Keys<float, float> mAlphaKeys; // 0x50
    /** The collection of position keys. */
    Keys<Vector3, Vector3> mTransKeys; // 0x5c
    /** The collection of scale keys. */
    Keys<Vector3, Vector3> mScaleKeys; // 0x68
    /** The collection of rotation keys. */
    Keys<Vector3, Vector3> mRotKeys; // 0x74
    /** The collection of texture keys. */
    TexKeys mTexKeys; // 0x50
};

void Interp(const RndMatAnim::TexPtr &, const RndMatAnim::TexPtr &, float, RndTex *&);
