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
public: // according to RB2
    SpotDrawParams(SpotlightDrawer *);
    SpotDrawParams &operator=(const SpotDrawParams &);
    void Save(BinStream &);
    void Load(BinStreamRev &);

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
    // size 0x50
    class SpotMeshEntry { // from RB2 DWARF
    public:
        SpotMeshEntry() : mMesh(0), mEnv(0), mLight(0) {}
        SpotMeshEntry &operator=(const SpotMeshEntry &e) {
            memcpy(this, &e, sizeof(SpotMeshEntry));
            return *this;
        }
        RndMesh *mMesh; // 0x0
        RndEnviron *mEnv; // 0x4
        Spotlight *mLight; // 0x8
        unsigned int mPACKING; // 0xc
        Transform mXfm; // 0x10
    };

    class SpotlightEntry { // from RB2 DWARF
    public:
        SpotlightEntry() : mPackedColor(0), mLight(0) {}
        unsigned int mPackedColor; // 0x0
        Spotlight *mLight; // 0x4
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
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    // PostProcessor
    virtual void EndWorld();
    virtual float Priority() { return 0.1f; }
    virtual const char *GetProcType() { return "SpotlightDrawer"; }

    OBJ_MEM_OVERLOAD(0x34)
    NEW_OBJ(SpotlightDrawer)

    static void Init();

    void Select();
    void DeSelect();
    void ClearLights();
    void UpdateBoxMap();
    void ApplyLightingApprox(BoxMapLighting &, float) const;

    const SpotDrawParams &Params() const { return mParams; }

    static SpotlightDrawer *Current() { return sCurrent; }
    static bool DrawNGSpotlights();
    static void DrawLight(Spotlight *);
    static void RemoveFromLists(Spotlight *);

protected:
    // RndDrawable
    virtual void DrawShowing();
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
    static RndEnviron *sEnviron;
    static bool sNeedDraw;
    static std::vector<SpotlightEntry> sLights;
    static std::vector<SpotMeshEntry> sCans;
    static std::vector<Spotlight *> sShadowSpots;
    static int sNeedBoxMap;
    static bool sHaveAdditionals;
    static bool sHaveLenses;
    static bool sHaveFlares;
    static bool sNoBeams;

    SpotDrawParams mParams; // 0x44
};

class ByColor {
public:
    bool operator()(
        const SpotlightDrawer::SpotlightEntry &e1,
        const SpotlightDrawer::SpotlightEntry &e2
    ) const {
        return e1.mPackedColor < e2.mPackedColor;
    }
};

class ByEnvMesh {
public:
    bool operator()(
        const SpotlightDrawer::SpotMeshEntry &e1, const SpotlightDrawer::SpotMeshEntry &e2
    ) const {
        if (e1.mEnv < e2.mEnv)
            return true;
        else if (e1.mEnv > e2.mEnv)
            return false;
        else
            return e1.mMesh < e2.mMesh;
    }
};
