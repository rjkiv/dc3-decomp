#pragma once
#include "LightPreset.h"
#include "LightPresetManager.h"
#include "PhysicsManager.h"
#include "ThreeDSoundManager.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"
#include "world/CameraManager.h"
#include "world/CameraShot.h"
#include "world/LightHue.h"
#include "world/LightPreset.h"

/**
 * @brief An ObjectDir dedicated to holding world objects.
 * Original _objects description:
 * "A WorldDir contains world objects."
 */
class WorldDir : public PanelDir {
public:
    struct PresetOverride {
        PresetOverride(Hmx::Object *owner) : preset(owner), hue(owner) {}
        void Sync(bool set) {
            if (preset)
                preset->SetHue(set ? hue : nullptr);
        }

        /** "Subdir preset to modify" */
        ObjPtr<LightPreset> preset; // 0x0
        /** "Hue texture to use" */
        ObjPtr<LightHue> hue; // 0x14
    };

    struct BitmapOverride {
        BitmapOverride(Hmx::Object *owner) : original(owner), replacement(owner) {}
        void Sync(bool);

        /** "Subdir texture to replace" */
        ObjPtr<RndTex> original; // 0x0
        /** "Curdir texture to replace with" */
        ObjPtr<RndTex> replacement; // 0x14
    };

    struct MatOverride {
        MatOverride(Hmx::Object *owner) : mesh(owner), mat(owner), mat2(owner) {}
        void Sync(bool);

        /** "Subdir mesh to modify" */
        ObjPtr<RndMesh> mesh; // 0x0
        /** "Curdir material to set" */
        ObjPtr<RndMat> mat; // 0x14
        ObjPtr<RndMat> mat2; // 0x28
    };

    WorldDir();
    // Hmx::Object
    virtual ~WorldDir();
    OBJ_CLASSNAME(WorldDir);
    OBJ_SET_TYPE(WorldDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects();
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x1E)
    NEW_OBJ(WorldDir)

    static void Init();

    void ClearDeltas();
    CameraManager *GetCameraManager() const { return unk300; }

    DataNode OnGetPhysicsManager(const DataArray *);

private:
    void SyncHUD();
    void SyncHides(bool);
    void SyncCamShots(bool);

protected:
    ObjList<PresetOverride> mPresetOverrides; // 0x268
    ObjList<BitmapOverride> mBitmapOverrides; // 0x274
    ObjList<MatOverride> mMatOverrides; // 0x280
    /** "Subdir objects to hide" */
    ObjPtrList<RndDrawable> mHideOverrides; // 0x28c
    /** "Subdir camshots to inhibit" */
    ObjPtrList<CamShot> mCamShotOverrides; // 0x2a0
    /** "Things to show when ps3_per_pixel on CamShot" */
    ObjPtrList<RndDrawable> mPS3PerPixelShows; // 0x2b4
    /** "Things to hide when ps3_per_pixel on CamShot" */
    ObjPtrList<RndDrawable> mPS3PerPixelHides; // 0x2c8
    /** "HUD Preview Dir" */
    FilePath mHUDFilename; // 0x2dc
    RndDir *unk2e4; // 0x2e4
    /** "Whether to draw the HUD preview" */
    bool mShowHUD; // 0x2e8
    /** "hud to be drawn last" */
    ObjPtr<RndDir> mHUD; // 0x2ec
    ObjPtr<CameraManager> unk300; // 0x300
    bool unk314; // 0x314
    ThreeDSoundManager m3DSoundMgr; // 0x318
    LightPresetManager mLightPresetMgr; // 0x38c
    PhysicsManager *mPhysicsMgr; // 0x3dc
    bool unk3e0;
    bool mEchoMsgs; // 0x3e1
    float mDeltaSincePoll[4]; // 0x3e4
    bool unk3f4;
    /** "The first light preset to start" */
    ObjPtr<LightPreset> mTestLightPreset1; // 0x3f8
    /** "The second light preset to start" */
    ObjPtr<LightPreset> mTestLightPreset2; // 0x40c
    /** "animation time in beats" */
    float mTestAnimTime; // 0x420
    /** "TRUE if we explicitly do the postprocing" */
    bool mExplicitPostProc; // 0x424
};

inline BinStream &operator<<(BinStream &bs, const WorldDir::BitmapOverride &o) {
    bs << o.original << o.replacement;
    return bs;
}

BinStream &operator<<(BinStream &, const WorldDir::MatOverride &);
BinStream &operator<<(BinStream &, const WorldDir::PresetOverride &);

void SetTheWorld(WorldDir *);

extern WorldDir *TheWorld;
