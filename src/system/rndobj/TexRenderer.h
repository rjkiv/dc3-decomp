#pragma once
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

class RndTexRenderer : public RndDrawable, public RndAnimatable, public RndPollable {
public:
    // Hmx::Object
    virtual ~RndTexRenderer() {}
    OBJ_CLASSNAME(TexRenderer);
    OBJ_SET_TYPE(TexRenderer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable: `this` at 0x0
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual void DrawPreClear();
    virtual void UpdatePreClearState();
    // RndAnimatable: `this` at 0x34
    virtual void SetFrame(float frame, float blend);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // RndPollable: `this` at 0x50
    virtual void ListPollChildren(std::list<RndPollable *> &) const;

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(RndTexRenderer)

    void DrawToTexture(void);

protected:
    RndTexRenderer();
    void InitTexture(void);

    bool unk_0x58, mForce, mDrawPreClear, mDrawWorldOnly; // 0x58/59/5A/5B; -0x8C/8B/8A/89
    bool mDrawResponsible, mNoPoll, unk_0x5E, unk_0x5F; // 0x5C/5D/5E/5F; -0x88/87/86/85
    float mImpostorHeight; // 0x60; -0x84
    ObjPtr<RndTex> mOutputTexture; // 0x64; -0x80
    ObjPtr<RndDrawable> mDrawable; // 0x78; -0x6C
    ObjPtr<RndCam> mCamera; // 0x8C; -0x58
    ObjPtr<RndEnviron> mEnviron; // 0xA0; -0x44
    bool mPrimeDraw; // 0xB4; -0x30
    bool unk_0xB5; // 0xB5; -0x2F
    bool mForceMips; // 0xB6; -0x2E
    ObjPtr<RndCam> mMirrorCam; // 0xB8; -0x2C
    bool mClearBuffer; // 0xCC; -0x18
    Hmx::Color mClearColor; // 0xD0; -0x14
};
