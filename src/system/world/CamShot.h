#pragma once
#include "math/Mtx.h"
#include "rndobj/Anim.h"
#include "rndobj/Trans.h"

/** "A camera shot. This is an animated camera path with keyframed settings." */
class CamShot : public RndAnimatable, public RndTransformable {
public:
    // Hmx::Object
    virtual ~CamShot();
    OBJ_CLASSNAME(CamShot);
    OBJ_SET_TYPE(CamShot);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void StartAnim();
    virtual void EndAnim();
    virtual void SetFrame(float, float);
    virtual float StartFrame() { return 0; }
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget();
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;
    // CamShot
    virtual void SetPreFrame(float, float) {}
    virtual CamShot *CurrentShot() {}
    virtual bool CheckShotStarted();
    virtual bool CheckShotOver(float);

protected:
    CamShot();

    // these three could be re-ordered, unsure of current order rn
    virtual void ApplyDynamicOffsetPreLookAt(Transform &, bool);
    virtual void ApplyDynamicOffsetPostLookAt(Transform &);
    virtual void ApplyFinalCamTransform(Transform &);

    virtual float ZoomFovOffset() { return 0; }
};
