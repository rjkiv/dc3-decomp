#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/PostProc.h"
#include "rndobj/Tex.h"

class NgPostProc : public RndPostProc {
public:
    class BloomTextureSet {
    public:
        BloomTextureSet();
        virtual ~BloomTextureSet();

        void AllocateTextures(unsigned int, unsigned int);
        void FreeTextures();

    private:
        RndTex *mBloomTexture[2]; // 0x4
    };

    template <int N>
    class BloomTextures {
    public:
        BloomTextures() {}
        virtual ~BloomTextures() {
            for (int i = 0; i < N; i++) {
                mTextures[i].FreeTextures();
            }
        }

        void AllocateTextures(unsigned int w, unsigned int h) {
            for (int i = N; i != 0; i--) {
                mTextures[i].AllocateTextures(w, h);
            }
        }

    private:
        BloomTextureSet mTextures[N];
    };

    NgPostProc();
    virtual ~NgPostProc();
    OBJ_CLASSNAME(PostProc);
    OBJ_SET_TYPE(PostProc);
    virtual void Select();
    virtual void QueueMotionBlurObject(class RndDrawable *);
    virtual void SetBloomColor();
    // virtual void OnSelect();
    // virtual void OnUnselect();
    virtual void EndWorld();
    virtual void DoPost();

    static void Init();
    NEW_OBJ(NgPostProc);
    static void RebuildTex();
    static void Terminate();

protected:
    virtual void OnSelect();
    virtual void OnUnselect();

    static Hmx::Color s_prevBloomColor;
    static float s_prevBloomIntensity;
    static NgPostProc *s_BloomSetter;
    static BloomTextures<3> sBloom;

    static void ReleaseTex();

    void CheckGradientMap();
    void CheckVignette();
    void CheckMotionBlur();
    void CheckBlendPrevious();
    void DoVelocity();
    void CheckNoise();

    float unk22c; // 0x22c
    float unk230; // 0x230
    float unk234; // 0x234
    float unk238; // 0x238
    ObjPtrList<RndDrawable> unk23c; // 0x23c
    bool unk250; // 0x250
};
