#pragma once
#include "UIListWidget.h"
#include "obj/Data.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "ui/UIColor.h"
#include "ui/UILabel.h"
#include "ui/UIListWidget.h"
#include "utl/Symbol.h"

class UIList;
class UIListLabel;
class UIListMesh;
class UIListSubList;

class UIListProvider {
public:
    UIListProvider() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual ~UIListProvider() {}
    virtual void Text(int showing, int data, UIListLabel *slot, UILabel *label) const;
    virtual RndMat *Mat(int showing, int data, UIListMesh *slot) const;
    virtual UIListProvider *Provider(int showing, int data, UIListSubList *slot) const {
        return nullptr;
    }
    virtual void
    Custom(int showing, int data, class UIListCustom *slot, Hmx::Object *object) const {}
    virtual void UpdateExtendedText(int showing, int data, UILabel *label) const;
    virtual void UpdateExtendedMesh(int showing, int data, RndMesh *mesh) const;
    virtual void UpdateExtendedCustom(int showing, int data, Hmx::Object *custom) const;
    virtual Symbol DataSymbol(int data) const { return gNullStr; }
    virtual bool CanSelect(int data) const { return true; }
    virtual int DataIndex(Symbol sym) const {
        int numdata = NumData();
        for (int i = 0; i < numdata; i++) {
            if (DataSymbol(i) == sym)
                return i;
        }
        return -1;
    }
    virtual int NumData() const = 0;
    virtual bool IsActive(int data) const { return true; }
    virtual bool IsHidden(int data) const { return false; }
    virtual void InitData(RndDir *dir) {}
    virtual float
    GapSize(int showing, int data, int selected_data, int scroll_direction) const {
        return 0;
    }
    virtual UIComponent::State ComponentStateOverride(
        int showing, int data, UIComponent::State component_state
    ) const {
        return component_state;
    }
    virtual UIListWidgetState
    ElementStateOverride(int showing, int data, UIListWidgetState state) const {
        return state;
    }
    virtual UIListWidgetState SlotElementStateOverride(
        int showing, int data, class UIListWidget *slot, UIListWidgetState state
    ) const {
        return state;
    }
    virtual UIColor *SlotColorOverride(
        int showing, int data, class UIListWidget *slot, UIColor *color
    ) const {
        return color;
    }
    virtual void PreDraw(int showing, int data, UIListSlot *slot) const {}
    virtual int SnappableAtOrBeforeData(int data) const { return -1; }
    virtual bool IsSnappableAtData(int data) const { return false; }
    virtual bool IsHeader(int data) { return false; }
    virtual void UnHighlightCurrent() {}
    virtual void ClearIconLabels() {}
};

class DataProvider : public UIListProvider {
public:
    DataProvider(
        DataArray *data,
        int offset,
        bool fluidWidth,
        bool subArraysAsTokenFmt,
        UIList *list
    )
        : mData(0), mOffset(offset), mFluidWidth(fluidWidth),
          mSubarraysAsTokenFmt(subArraysAsTokenFmt), mList(list) {
        SetData(data);
    }
    virtual ~DataProvider() { mData->Release(); }
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int idx) const {
        DataNode &node = mData->Node(idx + mOffset);
        if (node.Type() == kDataArray) {
            return node.Array()->Sym(0);
        } else
            return node.ForceSym();
    }
    virtual int NumData() const { return mData->Size() - mOffset; }
    virtual bool IsActive(int idx) const {
        return std::find(mDisabled.begin(), mDisabled.end(), DataSymbol(idx))
            == mDisabled.end();
    }
    virtual float GapSize(int, int, int, int) const;
    virtual UIListWidgetState
    ElementStateOverride(int, int idx, UIListWidgetState s) const {
        return std::find(mDimmed.begin(), mDimmed.end(), DataSymbol(idx)) != mDimmed.end()
            ? kUIListWidgetInactive
            : s;
    }

    void SetData(DataArray *data);
    void Enable(Symbol element);
    void Disable(Symbol element);
    void Dim(Symbol element);
    void UnDim(Symbol element);
    DataArray *Data() const { return mData; }

private:
    DataArray *mData; // 0x4
    int mOffset; // 0x8
    bool mFluidWidth; // 0xc
    bool mSubarraysAsTokenFmt; // 0xd
    std::list<Symbol> mDisabled; // 0x10
    std::list<Symbol> mDimmed; // 0x18
    std::vector<float> mWidths; // 0x20
    UIList *mList; // 0x2c
};
