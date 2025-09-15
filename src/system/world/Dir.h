#pragma once
#include "LightPreset.h"
#include "LightPresetManager.h"
#include "ThreeDSoundManager.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "ui/PanelDir.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"
#include "world/CameraManager.h"
#include "world/CameraShot.h"

/**
 * @brief An ObjectDir dedicated to holding world objects.
 * Original _objects description:
 * "A WorldDir contains world objects."
 */
class WorldDir : public PanelDir {
public:
    struct PresetOverride {};
    struct BitmapOverride {};
    struct MatOverride {};
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

    void ClearDeltas();

protected:
    ObjList<PresetOverride> mPresetOverrides; // 0x268
    ObjList<BitmapOverride> mBitmapOverrides; // 0x274
    ObjList<MatOverride> mMatOverrides; // 0x280
    ObjPtrList<RndDrawable> unk28c; // 0x28c
    ObjPtrList<CamShot> unk2a0; // 0x2a0
    ObjPtrList<RndDrawable> unk2b4; // 0x2b4
    ObjPtrList<RndDrawable> unk2c8; // 0x2c8
    FilePath unk2dc; // 0x2dc
    int unk2e4; // 0x2e4
    bool unk2e8; // 0x2e8
    ObjPtr<RndDir> unk2ec; // 0x2ec
    ObjPtr<CameraManager> unk300; // 0x300
    bool unk314; // 0x314
    ThreeDSoundManager m3DSoundMgr; // 0x318
    LightPresetManager mLightPresetMgr; // 0x38c
    int unk3dc; // 0x3dc
    bool unk3e0;
    bool unk3e1;
    float mDeltaSincePoll[4]; // 0x3e4
    bool unk3f4;
    ObjPtr<LightPreset> unk3f8;
    ObjPtr<LightPreset> unk40c;
    float unk420;
    bool unk424;
};
