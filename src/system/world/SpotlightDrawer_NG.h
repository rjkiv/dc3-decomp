#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "world/Spotlight.h"
#include "world/SpotlightDrawer.h"

class NgSpotlightDrawer : public SpotlightDrawer {
public:
    // Hmx::Object
    virtual ~NgSpotlightDrawer();
    OBJ_CLASSNAME(NgSpotlightDrawer)
    OBJ_SET_TYPE(NgSpotlightDrawer)

    // PostProcessor
    virtual void EndWorld();
    virtual void DoPost();
    virtual char const *GetProcType();

    NEW_OBJ(NgSpotlightDrawer);

    static void Init();

    NgSpotlightDrawer();
    void RenderScene();

    RndCam *unk94;
    ObjPtr<RndCam> unk98;
    int unkac;
    bool unkb0;

protected:
    // RndDrawable
    virtual void SetAmbientColor(Hmx::Color const &);
    virtual void ClearPostDraw();
    virtual void ClearPostProc();

    static int RTWidth();
    static int RTHeight();
    static bool CheckSharedResources();

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
};
