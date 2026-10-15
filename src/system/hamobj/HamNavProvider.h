#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/MemMgr.h"

class HamNavList;

/** "List of data for HamNavList" */
class HamNavProvider : public Hmx::Object, public UIListProvider {
public:
    enum CheckboxMode {
        kCheckbox_None = 0,
        kCheckbox_Disabled = 1,
        kCheckbox_Enabled = 2
    };

    enum ScoreDisplayMode {
        kScoreDisplay_None = 0,
        kScoreDisplay_Perform = 1,
        kScoreDisplay_Practice = 2,
        kScoreDisplay_Multiplayer = 3,
    };

    struct NavItem {
        NavItem()
            : mLabel(gNullStr), mCheckboxState(kCheckbox_None),
              mScoreDisplayMode(kScoreDisplay_None), mEnabled(true), mHidden(false),
              mFmtArgs(0), mLabelProvider(0) {}
        NavItem(const NavItem &other)
            : mLabel(other.mLabel), mCheckboxState(other.mCheckboxState),
              mSongID(other.mSongID), mScoreDisplayMode(other.mScoreDisplayMode),
              mEnabled(other.mEnabled), mHidden(other.mHidden), mFmtArgs(other.mFmtArgs),
              mLabels(other.mLabels), mLabelProvider(other.mLabelProvider) {
            if (mFmtArgs) {
                mFmtArgs->AddRef();
            }
        }
        ~NavItem() {
            if (mFmtArgs) {
                mFmtArgs->Release();
                mFmtArgs = nullptr;
            }
        }

        /** "used for a list entry with a single label" */
        Symbol mLabel; // 0x0
        CheckboxMode mCheckboxState; // 0x4
        int mSongID; // 0x8
        ScoreDisplayMode mScoreDisplayMode; // 0xc - some sort of state/enum?
        bool mEnabled; // 0x10
        bool mHidden; // 0x11
        DataArray *mFmtArgs;
        /** "used for a list entry with a list of labels" */
        std::vector<Symbol> mLabels; // 0x18
        DataProvider *mLabelProvider; // 0x24
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

    void SetChecked(Symbol, bool, bool);
    void SelectRadioButton(Symbol);
    void SetStars(Symbol, int, bool);
    void SetLabel(int, Symbol);
    Symbol DataSymbol(int, int) const;
    void SetLabel(int, int, Symbol);
    void SetLabels(int, DataArray *);
    void ResetLabelProvider(int);
    void SetEnabled(int, bool);
    bool IsEnabled(int) const;
    void SetHidden(int, bool);

    void SetNavList(HamNavList *l) { mNavList = l; }
    std::vector<NavItem> &Items() { return mNavItems; }

    DataNode OnSetHidden(const DataArray *);

protected:
    HamNavProvider();

    void CreateSubListProvider(int);
    int FindLabel(Symbol);

    DataNode OnSetEnabled(const DataArray *);
    DataNode OnSetFormatArgs(const DataArray *);

    std::vector<NavItem> mNavItems; // 0x30
    HamNavList *mNavList; // 0x3c
};
