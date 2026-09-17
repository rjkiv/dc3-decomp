#pragma once
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UIListState.h"
#include "utl/MemMgr.h"

class UIList;
class UIListProvider;

enum UIListWidgetState {
    kUIListWidgetActive,
    kUIListWidgetHighlight,
    kUIListWidgetInactive,
    kNumUIListWidgetStates
};

enum UIListWidgetDrawType {
    kUIListWidgetDrawAlways,
    kUIListWidgetDrawOnlyFocused,
    kUIListWidgetDrawFocusedOrManual,
    kNumUIListWidgetDrawTypes
};

enum DrawCommand {
    kDrawAll,
    kDrawFirst,
    kExcludeFirst
};

// size 0x3c
struct UIListElementDrawState {
    int unk0, unk4, unk8, unkc;
    int unk10, unk14, unk18, unk1c;
    int unk20, unk24;
    UIComponent::State mComponentState; // 0x28
    int unk2c;
    int unk30, unk34, unk38;
};

struct UIListWidgetDrawState {
    Vector3 mFirstPos; // 0x0
    Vector3 mLastPos; // 0x10
    Vector3 mHighlightPos; // 0x20
    int mHighlightDisplay; // 0x30
    UIListWidgetState mHighlightElementState; // 0x34
    std::vector<UIListElementDrawState> mElements; // 0x38
};

class UIListWidget : public Hmx::Object {
public:
    // Hmx::Object
    OBJ_CLASSNAME(UIListWidget)
    OBJ_SET_TYPE(UIListWidget)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // UIListWidget
    virtual UIList *SubList(int) { return nullptr; }
    virtual void ResourceCopy(const UIListWidget *w);
    virtual void CreateElements(UIList *, int) {}
    virtual void Draw(
        const UIListWidgetDrawState &,
        const UIListState &,
        const Transform &,
        UIComponent::State,
        Box *,
        DrawCommand
    ) {}
    virtual void Fill(const UIListProvider &, int, int, int) {}
    virtual void StartScroll(int, bool) {}
    virtual void CompleteScroll(const UIListState &, int) {}
    virtual void Poll() {}

    float DrawOrder() const;
    void SetParentList(UIList *);
    void SetColor(UIListWidgetState, UIComponent::State, UIColor *);
    UIList *ParentList();

    UIListWidgetDrawType DrawType() const { return mWidgetDrawType; }

    NEW_OBJ(UIListWidget)
    OBJ_MEM_OVERLOAD(0x48)

protected:
    UIListWidget();

    void CalcXfm(const Transform &, const Vector3 &, Transform &);
    UIColor *DisplayColor(UIListWidgetState, UIComponent::State) const;
    void
    DrawMesh(RndMesh *, UIListWidgetState, UIComponent::State, Transform const &, Box *);

    float mDrawOrder; // 0x2c
    float mDisabledAlphaScale; // 0x30
    ObjPtr<UIColor> mDefaultColor; // 0x34
    std::vector<std::vector<ObjPtr<UIColor> > > mColors; // 0x48 - a vector of vectors of
                                                         // ObjPtrs...wonderful
    UIListWidgetDrawType mWidgetDrawType; // 0x54
    UIList *mParentList; // 0x58
};
