#pragma once
#include "UIComponent.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/FontBase.h"
#include "rndobj/Group.h"
#include "ui/UIColor.h"
#include "ui/UIFontImporter.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

/** "Top-level resource object for UILabels" */
class UILabelDir : public RndDir, public UIFontImporter {
public:
    // Hmx::Object
    OBJ_CLASSNAME(UILabelDir);
    OBJ_SET_TYPE(UILabelDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    OBJ_MEM_OVERLOAD(0x19);

    bool AllowEditText() const;
    RndFontBase *FontObj(Symbol) const;
    UIColor *GetStateColor(UIComponent::State) const;
    static DataNode GetMatVariations(UILabelDir *);
    static void Init();

protected:
    UILabelDir();

    /** "color to use when no other color is defined for a state" */
    ObjPtr<UIColor> mDefaultColor; // 0x2f0
    /** The colors to use depending on the label's state.
     * This vector is expected to be of size UIComponent::kNumStates.
     * Original _objects descriptions:
     * "color when label is normal"
     * "color when label is focused"
     * "color when label is disabled"
     * "color when label is selecting"
     * "color when label is selected"
     */
    std::vector<ObjPtr<UIColor> > mColors; // 0x304
    ObjPtr<RndAnimatable> mFocusAnim; // 0x310
    ObjPtr<RndAnimatable> mPulseAnim; // 0x324
    ObjPtr<RndGroup> mFocusedBackgroundGroup; // 0x338
    ObjPtr<RndGroup> mUnfocusedBackgroundGroup; // 0x34c
    /** "allow non-localized text with this resource?" */
    bool mAllowEditText; // 0x360
};
