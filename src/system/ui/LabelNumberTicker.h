#pragma once
#include "obj/Object.h"
#include "rndobj/EventTrigger.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"

/** "a ticker to control counting up or down for a given number based label" */
class LabelNumberTicker : public UIComponent {
public:
    // Hmx::Object
    virtual ~LabelNumberTicker();
    OBJ_CLASSNAME(LabelNumberTicker)
    OBJ_SET_TYPE(LabelNumberTicker)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    NEW_OBJ(LabelNumberTicker)
    OBJ_MEM_OVERLOAD(0x14)

    static void Init();

    void CountUp();
    void CountUpFromCurrentValue();
    void SnapToValue(int i);

    UILabel *Label() const { return mLabel; }
    void SetLabel(UILabel *);

protected:
    LabelNumberTicker();
    void UpdateDisplay();

    /** "label to be shrink wrapped" */
    ObjPtr<UILabel> mLabel; // 0x44
    int mDesiredValue; // 0x58
    float mAnimTime; // 0x5c
    float mAnimDelay; // 0x60
    Symbol mWrapperText; // 0x64
    float mAcceleration; // 0x68
    int unk6c; // 0x6c
    int unk70; // 0x70
    u32 unk74; // 0x74
    Timer mTimer; // 0x78
    ObjPtr<EventTrigger> mTickTrigger; // 0xa8
    int mTickEvery; // 0xbc

private:
    void SetDesiredValue(int);
};
