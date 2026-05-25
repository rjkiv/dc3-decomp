#pragma once
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Joypad.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class InlineHelp : public UIComponent {
public:
    struct ActionElement { // From RB3 decomp
        JoypadAction mAction; // 0x0
        Symbol mPrimaryToken; // 0x4
        Symbol mSecondaryToken; // 0x8
        String mPrimaryStr; // 0xc
        String mSecondaryStr; // 0x14

        ~ActionElement();
        ActionElement();
        ActionElement(JoypadAction);

        bool HasSecondaryStr() const { return !mSecondaryStr.empty(); }
        void SetToken(Symbol, bool);
        void SetString(char const *, bool);
        void SetConfig(DataNode &, bool);
        Symbol GetToken(bool) const;
        const char *GetText(bool) const;
    };

    // Hmx::Object
    virtual ~InlineHelp();
    OBJ_CLASSNAME(InlineHelp);
    OBJ_SET_TYPE(InlineHelp);
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
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    NEW_OBJ(InlineHelp)
    OBJ_MEM_OVERLOAD(0x16)

    static void Init();

    void UpdateTextColors();
    void ClearActionToken(JoypadAction);
    void SetActionToken(JoypadAction, DataNode &);
    void SetLabelRotationPcts(float f);
    DataNode OnSetConfig(DataArray const *);

private:
    static float sLastUpdatedTime;
    static float sRotationTime;
    static float sLabelRot;
    static bool sHasFlippedTextThisRotation;
    static bool sNeedsTextUpdate;
    static bool sRotated;
    static const float sRotateDelay;
    static const float sRotateDuration;

protected:
    InlineHelp();

    void Update();
    void UpdateLabelText();

    static void ResetRotation();

    std::vector<Symbol> mIconTypes; // 0x44
    std::vector<ActionElement> mConfig; // 0x50
    std::vector<UILabel *> mTextLabels; // 0x5c
    bool mUseConnectedControllers; // 0x68
    bool mHorizontal; // 0x69
    float mSpacing; // 0x6c
    ResourceDirPtr<ObjectDir> mResourceDir; // 0x70
    UILabel *unk88; // 0x88 - text label?
    ObjPtr<UIColor> mTextColor; // 0x8c

    virtual void OldResourcePreload(BinStream &);
    virtual void SyncLabelsToConfig();
    virtual void UpdateIconTypes(bool);
    virtual String GetIconStringFromAction(int);
};
