#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/MemMgr.h"

class HamNavList;

class HamNavProvider : public Hmx::Object, public UIListProvider {
public:
    struct NavItem {
        NavItem()
            : mLabel(gNullStr), mCheckbox(0), unkc(0), unk10(1), unk11(0), unk14(0),
              unk24(0) {}
        ~NavItem() {
            if (unk14) {
                unk14->Release();
                unk14 = nullptr;
            }
        }

        Symbol mLabel; // 0x0
        int mCheckbox; // 0x4
        int unk8;
        int unkc;
        bool unk10;
        bool unk11;
        DataArray *unk14;
        std::vector<Symbol> mLabels; // 0x18
        DataProvider *unk24;
    };
    // Hmx::Object
    virtual ~HamNavProvider();
    OBJ_CLASSNAME(HamNavProvider);
    OBJ_SET_TYPE(HamNavProvider);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const { return nullptr; }
    virtual UIListProvider *Provider(int, int, UIListSubList *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return mNavItems.size(); }
    virtual bool IsActive(int) const;
    virtual bool IsHidden(int) const;

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(HamNavProvider)
    static void Init();

    void Refresh();
    void SetChecked(Symbol, bool, bool);
    void SelectRadioButton(Symbol);
    void SetStars(Symbol, int, bool);
    void SetLabel(int, Symbol);

    DataNode OnSetHidden(const DataArray *);

protected:
    HamNavProvider();

    void CreateSubListProvider(int);

    DataNode OnSetEnabled(const DataArray *);
    DataNode OnSetFormatArgs(const DataArray *);

    std::vector<NavItem> mNavItems; // 0x30
    HamNavList *mNavList; // 0x3c
};
