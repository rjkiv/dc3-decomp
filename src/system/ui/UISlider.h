#pragma once
#include "ResourceDirPtr.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "ui/UIComponent.h"
#include "ui/ScrollSelect.h"
#include "os/JoypadMsgs.h"
#include "utl/MemMgr.h"

/** "A component with animatable whose frames correspond to a
 *  range of values. The resources don't have to look like a slider;
 *  they could easily be a knob, dial, etc." */
class UISlider : public UIComponent, public ScrollSelect {
public:
    // Hmx::Object
    OBJ_CLASSNAME(UISlider)
    OBJ_SET_TYPE(UISlider)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetTypeDef(DataArray *);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    // UIComponent
    virtual void Enter();
    virtual void SetCurrent(int);
    // ScrollSelect
    virtual int SelectedAux() const;

    OBJ_MEM_OVERLOAD(0x18);
    NEW_OBJ(UISlider)
    REGISTER_OBJ(UISlider)
    static void Init();

    float Frame() const;
    void SetNumSteps(int);
    void SetFrame(float);
    int Current() const;

private:
    void SyncSlider();

protected:
    UISlider();

    virtual void SetSelectedAux(int);
    virtual void OldResourcePreload(BinStream &);

    void Update();
    DataNode OnMsg(const ButtonDownMsg &);

    ResourceDirPtr<RndDir> mSliderResource; // 0x50
    RndMesh *unk68; // 0x68
    RndMat *unk6c[UIComponent::kNumStates]; // 0x6c
    int mCurrent; // 0x80
    int mNumSteps; // 0x84
    bool mVertical; // 0x88
};
