#pragma once
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Mesh.h"
#include "rndobj/PostProc.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class Spotlight;
class SpotlightDrawer;

class SpotDrawParams {
    friend class SpotlightDrawer;

public:
    SpotDrawParams(SpotlightDrawer *);
    SpotDrawParams &operator=(const SpotDrawParams &);
    void Save(BinStream &);
    void Load(BinStreamRev &);

private:
    float mIntensity; // 0x0
    Hmx::Color mColor; // 0x4
    float mBaseIntensity; // 0x14
    float mSmokeIntensity; // 0x18
    float mHalfDistance; // 0x1c
    float mLightingInfluence; // 0x20
    ObjPtr<RndTex> mTexture; // 0x24
    ObjPtr<RndDrawable> mProxy; // 0x38
    SpotlightDrawer *mOwner; // 0x4c
};

/** "A SpotlightDrawer draws spotlights." */
class SpotlightDrawer : public RndDrawable, public PostProcessor {
public:
    class SpotMeshEntry { // from RB3 decomp
    public:
        SpotMeshEntry() : unk0(0), unk4(0), unk8(0) {}
        RndMesh *unk0;
        RndMesh *unk4;
        Spotlight *unk8;
        int unkc;
        Transform unk10;
    };

    class SpotlightEntry { // from RB3 decomp
    public:
        SpotlightEntry() : unk0(0), unk4(0) {}
        unsigned int unk0; // 0x0 - id?
        Spotlight *unk4; // 0x4 - the spotlight
    };

    // Hmx::Object
    virtual ~SpotlightDrawer();
    OBJ_CLASSNAME(SpotlightDrawer);
    OBJ_SET_TYPE(SpotlightDrawer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    // PostProcessor
    virtual void EndWorld();
    virtual float Priority();
    virtual const char *GetProcType() { return "SpotlightDrawer"; }

    OBJ_MEM_OVERLOAD(0x34)
    NEW_OBJ(SpotlightDrawer)

    static RndEnviron *sEnviron;

    static void Init();

    void Select();
    void DeSelect();
    void ClearLights();
    void UpdateBoxMap();
    void ApplyLightingApprox(BoxMapLighting &, float) const;

    static SpotlightDrawer *Current() { return sCurrent; }

protected:
    // SpotlightDrawer
    virtual void SetAmbientColor(const Hmx::Color &);
    virtual void SortLights();
    virtual void DrawWorld();
    virtual void DrawShadow();
    virtual void DrawMeshVec(std::vector<SpotMeshEntry> &);
    virtual void DrawAdditional(SpotlightEntry *, SpotlightEntry *const &);
    virtual void DrawLenses(SpotlightEntry *, SpotlightEntry *const &);
    virtual void DrawBeams(SpotlightEntry *, SpotlightEntry *const &);
    virtual void DrawFlares(SpotlightEntry *, SpotlightEntry *const &);
    virtual void ClearPostDraw();
    virtual void ClearPostProc() {}

    SpotlightDrawer();

    static SpotlightDrawer *sCurrent;
    static SpotlightDrawer *sDefault;
    static bool sNeedDraw;
    static std::vector<SpotlightEntry> sLights;
    static std::vector<SpotMeshEntry> sCans;
    static std::vector<Spotlight *> sShadowSpots;
    static int sNeedBoxMap;
    static bool sHaveAdditionals;
    static bool sHaveLenses;
    static bool sHaveFlares;

    SpotDrawParams mParams; // 0x44
};

class ByColor {
public:
    bool operator()(
        const SpotlightDrawer::SpotlightEntry &e1,
        const SpotlightDrawer::SpotlightEntry &e2
    ) const {
        return e1.unk0 < e2.unk0;
    }
};

class ByEnvMesh {
public:
    bool operator()(
        const SpotlightDrawer::SpotMeshEntry &e1, const SpotlightDrawer::SpotMeshEntry &e2
    ) const {
        if (e1.unk4 < e2.unk4)
            return true;
        else if (e1.unk4 > e2.unk4)
            return false;
        else
            return e1.unk0 < e2.unk0;
    }
};
