#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "utl/MemMgr.h"

/** "Resource object for scrolly stuff" */
class HamScrollSpeedIndicator : public RndDir {
public:
    HamScrollSpeedIndicator();
    // Hmx::Object
    virtual ~HamScrollSpeedIndicator() {}
    OBJ_CLASSNAME(HamScrollSpeedIndicator);
    OBJ_SET_TYPE(HamScrollSpeedIndicator);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects() { RndDir::SyncObjects(); }
    // RndDrawable
    virtual void DrawShowing();
    // RndAnimatable
    virtual void SetFrame(float frame, float blend) {
        RndAnimatable::SetFrame(frame, blend);
    }
    virtual float StartFrame();
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x11)
    NEW_OBJ(HamScrollSpeedIndicator)

    void Show(bool);
    void HandleEnter();
    void HandleExit();
    void Draw(const Transform &);

protected:
    bool unk1fc; // 0x1fc
    /** "Frame at which the bar reaches the slow-scroll indicator" */
    float mSlowScrollThresholdFrame; // 0x200
    /** "Frame at which the bar reaches the fast-scroll indicator" */
    float mFastScrollThresholdFrame; // 0x204
    ObjPtr<RndAnimatable> mEnterAnim; // 0x208
    ObjPtr<RndAnimatable> mExitAnim; // 0x21c
    ObjPtr<RndAnimatable> mIndicatorAnim; // 0x230
};
