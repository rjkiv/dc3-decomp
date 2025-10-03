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
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual UIListProvider *Provider(int, int, UIListSubList *) const { return nullptr; }
    virtual void Custom(int, int, class UIListCustom *, Hmx::Object *) const {}
    virtual void UpdateExtendedText(int, int, UILabel *) const;
    virtual void UpdateExtendedMesh(int, int, RndMesh *) const;
    virtual void UpdateExtendedCustom(int, int, Hmx::Object *) const;
    virtual Symbol DataSymbol(int) const { return gNullStr; }
    virtual bool CanSelect(int) const { return true; }
    virtual int DataIndex(Symbol s) const {
        int numdata = NumData();
        for (int i = 0; i < numdata; i++) {
            if (DataSymbol(i) == s)
                return i;
        }
        return -1;
    }
    virtual int NumData() const = 0;
    virtual bool IsActive(int) const { return true; }
    virtual bool IsHidden(int) const { return false; }
    virtual void InitData(RndDir *) {}
    virtual float GapSize(int, int, int, int) const { return 0; }
    virtual UIComponent::State
    ComponentStateOverride(int, int, UIComponent::State s) const {
        return s;
    }
    virtual UIListWidgetState ElementStateOverride(int, int, UIListWidgetState s) const {
        return s;
    }
    virtual UIListWidgetState
    SlotElementStateOverride(int, int, class UIListWidget *, UIListWidgetState s) const {
        return s;
    }
    virtual UIColor *SlotColorOverride(int, int, class UIListWidget *, UIColor *c) const {
        return c;
    }
    virtual void PreDraw(int, int, UIListSlot *) const {}
    virtual int SnappableAtOrBeforeData(int) const { return -1; }
    virtual bool IsSnappableAtData(int) const { return false; }
    virtual bool IsHeader(int) { return false; }
    virtual void UnHighlightCurrent() {}
    virtual void ClearIconLabels() {}
};

class DataProvider : public UIListProvider {
public:
    DataProvider(DataArray *arr, int i, bool b1, bool b2, UIList *ul)
        : mData(0), mOffset(i), mFluidWidth(b1), unkd(b2), mList(ul) {
        SetData(arr);
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

    void SetData(DataArray *);
    void Enable(Symbol);
    void Disable(Symbol);
    void Dim(Symbol);
    void UnDim(Symbol);
    DataArray *Data() const { return mData; }

protected:
    DataArray *mData; // 0x4
    int mOffset; // 0x8
    bool mFluidWidth; // 0xc
    bool unkd; // 0xd
    std::list<Symbol> mDisabled; // 0x10
    std::list<Symbol> mDimmed; // 0x18
    std::vector<float> mWidths; // 0x20
    UIList *mList; // 0x2c
};
