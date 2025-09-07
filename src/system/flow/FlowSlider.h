#pragma once
#include "flow/FlowNode.h"
#include "flow/PropertyEventListener.h"
#include "math/Easing.h"

/** "Ramps intensity up/down on child nodes based on value of slider" */
class FlowSlider : public FlowNode, public PropertyEventListener {
public:
    // Hmx::Object
    virtual ~FlowSlider();
    OBJ_CLASSNAME(FlowSlider)
    OBJ_SET_TYPE(FlowSlider)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual bool IsRunning();
    virtual void UpdateIntensity();

    void ReActivate();

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(FlowSlider)

protected:
    FlowSlider();

    void UpdateEase();
    void UpdateActivations();

    /** "Do we listen for changes on our value property?" */
    bool mPersistent; // 0x74
    /** "If true, we run all children regardless of intensity.
        If false, we stop and start them based on intensity" */
    bool mAlwaysRun; // 0x75
    /** "position on slider" */
    float mValue; // 0x78
    /** "Easing to apply to slider intensity output" */
    EaseType mEaseType; // 0x7c
    /** "Modifier to easing equation" */
    float mEasePower; // 0x80
    EaseFunc *mEaseFunc; // 0x84
};
