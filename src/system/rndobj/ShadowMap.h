#pragma once
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Tex.h"

class RndShadowMap {
public:
    static void Init();
    static void Terminate();
    static void EndShadow();
    static bool PrepShadow(RndDrawable *, RndEnviron *);

    // public: static const int32_t kShadowBufferWidth;
    // public: static const int32_t kShadowBufferHeight;
    // public: static const int32_t kShadowBufferBpp;

protected:
    static RndCam *sLightCam;
    static RndTex *sShadowTex;
};
