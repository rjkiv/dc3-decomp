#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/PostProc.h"
#include "rndobj/Tex.h"

enum BloomBlurStyle {
    kBloomBlurStyle_Gaussian = 0x0000,
    kBloomBlurStyle_Streak = 0x0001,
    kBloomBlurStyle_Glare = 0x0002,
};

enum BloomBlurDirection {
    kBloomBlurDirection_Horizontal = 0x0000,
    kBloomBlurDirection_Vertical = 0x0001,
};

class NgPostProc : public RndPostProc {
public:
    class BloomTextureSet {
    public:
        BloomTextureSet();
        virtual ~BloomTextureSet();

        void AllocateTextures(unsigned int, unsigned int);
        void FreeTextures();
        RndTex *GetTex(unsigned int idx) { return mBloomTexture[idx]; }

    private:
        RndTex *mBloomTexture[2]; // 0x4
    };

    template <int N>
    class BloomTextures {
    public:
        BloomTextures() {}
        virtual ~BloomTextures() {
            for (int i = 0; i < N; i++) {
                mBloomSet[i].FreeTextures();
            }
        }

        void AllocateTextures(unsigned int w, unsigned int h) {
            for (int i = 0; i < DIM(mBloomSet); i++) {
                w /= 4;
                h /= 4;
                mBloomSet[i].AllocateTextures(w, h);
            }
        }

        void FreeTextures() {
            for (int i = 0; i < DIM(mBloomSet); i++) {
                mBloomSet[i].FreeTextures();
            }
        }

        RndTex *GetTex(unsigned int i, unsigned int j) { return mBloomSet[i].GetTex(j); }

    private:
        BloomTextureSet mBloomSet[N];
    };

    NgPostProc();
    virtual ~NgPostProc();
    OBJ_CLASSNAME(PostProc);
    OBJ_SET_TYPE(PostProc);
    virtual void Select();
    virtual void QueueMotionBlurObject(class RndDrawable *);
    virtual void SetBloomColor();
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
    void CheckHueConverge();
    void CheckHallOfTime();
    void DoBloom();
    void ModulateColorXfm();
    void CheckRefract();
    void CheckChromaticAberration();
    void CheckPosterizeAndKaleidoscope();

    Vector2 mPhase; // 0x22c
    Vector2 mRefractAccumDistance; // 0x234
    ObjPtrList<RndDrawable> mMotionBlurObjects; // 0x23c
    bool mFirstFrame; // 0x250
};
