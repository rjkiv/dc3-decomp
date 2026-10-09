#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Tex.h"
#include "rndobj/Cam.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"
#include "xdk/D3D9.h"

class NgSpotlightDrawer : public SpotlightDrawer {
public:
    class SpotlightResources {
    public:
        SpotlightResources()
            : mDepthTex(0), mDepthRT(0), mDensityMap(0), mBlack(0), mWhite(0),
              mFarDepth(0) {}
        virtual ~SpotlightResources() {}
        void Clear();

        MEM_OVERLOAD(SpotlightResources, 0x3B);

        D3DTexture *mDepthTex; // 0x4
        DxTex *mDepthRT; // 0x8
        RndTex *mDensityMap; // 0xC
        RndTex *mBlack; // 0x10
        RndTex *mWhite; // 0x14
        RndTex *mFarDepth; // 0x18
    };

    NgSpotlightDrawer();
    // Hmx::Object
    virtual ~NgSpotlightDrawer();
    OBJ_CLASSNAME(NgSpotlightDrawer)
    OBJ_SET_TYPE(NgSpotlightDrawer)

    // PostProcessor
    virtual void EndWorld();
    virtual void DoPost();
    virtual char const *GetProcType() { return "NgSpotlightDrawer"; }

    NEW_OBJ(NgSpotlightDrawer);

    static void Init();

    SpotlightResources &SR() {
        MILO_ASSERT(sSharedResources, 0xA0);
        return *sSharedResources;
    }

    void RenderScene();

protected:
    // RndDrawable
    virtual void SetAmbientColor(Hmx::Color const &);
    virtual void DrawBeams(SpotlightEntry *, SpotlightEntry *const &) {}
    virtual void ClearPostDraw();
    virtual void ClearPostProc();

    static int RTWidth();
    static int RTHeight();
    static bool CheckSharedResources();
    static bool CheckRTs(SpotlightResources *);

    static SpotlightResources *sSharedResources;

    bool RestoreCam();
    bool CheckFogTexture();
    void SetXSectionTexture(Spotlight::BeamDef const &);
    void SetupFogDensityMap();
    void RenderFogProxy();
    void RenderSphere(Spotlight *);
    void RenderSheet(Spotlight *);
    void SetupXSection(Spotlight *, Spotlight::BeamDef const &);
    void RenderConeDefs(Spotlight *, Hmx::Color const &);
    void SetupFogDensityState();
    void RenderCone(Spotlight *);
    void RenderBeams(Hmx::Matrix4 const &);
    bool CheckCam();
    void BlurRT(float, float);
    void BlurRT();
    void SetupForPostProcess();

    RndCam *mCam; // 0x94
    ObjPtr<RndCam> mHoldCam; // 0x98
    RndTex *mFogDensityMap; // 0xac
    bool mDbgDrawEnable; // 0xb0
};
