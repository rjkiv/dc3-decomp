#pragma once
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

/** "TexRender renders a draw and cam into a texture." */
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
    virtual void DrawPreClear() { DrawToTexture(); }
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
    static void Init() { REGISTER_OBJ_FACTORY(RndTexRenderer) }

    void DrawToTexture();
    RndTex *GetOutputTexture() const { return mOutputTexture; }
    void SetOutputTexture(RndTex *tex) { mOutputTexture = tex; }

protected:
    RndTexRenderer();
    void InitTexture();

    DataNode OnGetRenderTextures(DataArray *);

    bool mDirty; // 0x58; -0x8C
    /** "Force rendering every frame" */
    bool mForce; // 0x59 / -0x8B
    /** "Renders the texture before the rest of the scene is rendered.
        Useful for rendering large textures" */
    bool mDrawPreClear; // 0x5A / -0x8A
    /** "Renders the texture only on 'world' frames,
       while skipping rendering on post processing frames" */
    bool mDrawWorldOnly; // 0x5B / -0x89
    /** "If true, exclusively draws the draw,
        if false the scene will draw it too, use with caution!" */
    bool mDrawResponsible; // 0x5C / -0x88
    /** "If [draw] will not get enter, exit, or poll automatically,
        it will be up to script hooks to do any of that" */
    bool mNoPoll; // 0x5D / -0x87
    bool unk_0x5E, unk_0x5F; // 0x5E/5F; -0x86/85
    /** "Height for imposter rendering with current camera" */
    float mImpostorHeight; // 0x60; -0x84
    /** "Texture to write to" */
    ObjPtr<RndTex> mOutputTexture; // 0x64; -0x80
    /** "Draw Object to render to texture" */
    ObjPtr<RndDrawable> mDrawable; // 0x78; -0x6C
    /** "Camera to use, if you want specific one,
        defaults to proxy cam, if none and draw is proxy" */
    ObjPtr<RndCam> mCamera; // 0x8C; -0x58
    /** "Environment to set before rendering to texture" */
    ObjPtr<RndEnviron> mEnviron; // 0xA0; -0x44
    /** "Check this if rendering multiple characters to a texture.
        Will draw 2x if checked." */
    bool mPrimeDraw; // 0xB4; -0x30
    bool mFirstDraw; // 0xB5; -0x2F
    /** "Generate mip maps for the texture." */
    bool mForceMips; // 0xB6; -0x2E
    /** "We will mirror this cam about whatever mesh is associated
        with our output texture to automatically position
        the render-2-tex cam for mirroring" */
    ObjPtr<RndCam> mMirrorCam; // 0xB8; -0x2C
    bool mClearBuffer; // 0xCC; -0x18
    Hmx::Color mClearColor; // 0xD0; -0x14
};
