#pragma once
#include "UIListDir.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "os/User.h"
#include "rndobj/Mesh.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIComponent.h"
#include "ui/UILabel.h"
#include "ui/UIListProvider.h"
#include "ui/ScrollSelect.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "ui/UITransitionHandler.h"
#include "utl/Symbol.h"

/**
 * @brief A UI Object representing a list element.
 * Original _objects description:
 * "Component for displaying 1- or 2-dimensional lists of data.
 * Can be oriented horizontally or vertically, can scroll normally or
 * circularly, and can have any number of visible elements (even just
 * one, a.k.a. a spin button)."
 */
class UIList : public UIComponent,
               public UIListProvider,
               public ScrollSelect,
               public UIListStateCallback,
               public UITransitionHandler {
public:
    // Hmx::Object
    virtual ~UIList();
    OBJ_CLASSNAME(UIList)
    OBJ_SET_TYPE(UIList)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // UIListProvider
    virtual int NumData() const;
    // RndDrawable
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    // UIListStateCallback
    virtual void StartScroll(const UIListState &, int, bool);
    virtual void CompleteScroll(const UIListState &);

    // unsure where these go
    virtual void Enter();
    virtual void Poll();

    static void Init();

    void SetNumDisplay(int);
    void SetGridSpan(int);
    void SetCircular(bool);
    void SetSpeed(float speed); // { mListState.SetSpeed(speed); }
    void SetParent(UIList *);
    void LimitCircularDisplay(bool);
    void SetProvider(UIListProvider *);
    int NumProviderData() const;
    void DisableData(Symbol);
    void EnableData(Symbol);
    void DimData(Symbol);
    void UnDimData(Symbol);
    void Refresh(bool);
    void AutoScroll();
    void StopAutoScroll();
    void SetSelected(int, int);
    bool SetSelected(Symbol, bool, int);
    int SelectedPos() const;
    void Scroll(int);
    void CalcBoundingBox(Box &);
    Symbol SelectedSym(bool) const;
    void SetSelectedSimulateScroll(int);
    bool SetSelectedSimulateScroll(Symbol, bool);
    UIListDir *GetUIListDir() const;

    int NumDisplay() const { return mListState.NumDisplay(); }
    int GridSpan() const { return mListState.GridSpan(); }
    bool Circular() const { return mListState.Circular(); }
    float Speed() const;
    int SelectedData() const { return mListState.SelectedData(); }
    int FirstShowing() const { return mListState.FirstShowing(); }
    bool IsScrolling() const;

private:
    void Update();
    void OldResourcePreload(BinStream &);

protected:
    // ScrollSelect
    virtual int SelectedAux() const;
    virtual void SetSelectedAux(int);
    // UITransitionHandler
    virtual void FinishValueChange();
    virtual bool IsEmptyValue() const;

    UIList();

    DataNode OnMsg(const ButtonDownMsg &);
    DataNode OnSelectedSym(DataArray *);
    DataNode OnSetData(DataArray *);
    DataNode OnSetSelected(DataArray *);
    DataNode OnSetSelectedSimulateScroll(DataArray *);
    DataNode OnScroll(DataArray *);

    int CollidePlane(std::vector<Vector3> const &, Plane const &);
    void HandleSelectionUpdated();
    void UpdateExtendedEntries(UIListState const &);
    void PreLoadWithRev(BinStreamRev &);
    void BoundingBoxTriangles(std::vector<std::vector<Vector3> > &);

    ResourceDirPtr<UIListDir> mListDir; // 0x8c
    std::vector<UIListWidget *> mWidgets; // 0xa4
    UIListState mListState; // 0xb0
    DataProvider *mDataProvider; // 0xf8
    /** "Num data to show (only for milo)". Ranges from 1 to 1000. */
    int mNumData; // 0xfc
    /** "Allow scrolling by pages?" */
    bool mPaginate; // 0x100
    LocalUser *mUser; // 0x104
    UIList *mParent; // 0x108
    /** "labels to be filled in by list provider at runtime" */
    ObjPtrList<UILabel> mExtendedLabelEntries; // 0x10c
    /** "meshes to be filled in by list provider at runtime" */
    ObjPtrList<RndMesh> mExtendedMeshEntries; // 0x120
    /** "custom objects to be filled in by list provider at runtime" */
    ObjPtrList<Hmx::Object> mExtendedCustomEntries; // 0x134
    /** "Time to pause when auto scroll changes directions (seconds)".
        Ranges from 0 to 100. */
    float mAutoScrollPause; // 0x148
    /** "Should this list send UIComponentScroll* messages while auto-scrolling?" */
    bool mAutoScrollSendMsgs; // 0x14c
    int unk150;
    bool mAutoScrolling; // 0x154
    float unk158;
    bool unk15c;
    bool unk15d;
    /** "Allow multiple instances of same option to be displayed?" */
    bool mLimitCircularDisplayNumToDataNum; // 0x15e
    int unk160;
    bool mAllowHighlight; // 0x164
};

class UIListCustomTemplate {
public:
    virtual ~UIListCustomTemplate() {}
    virtual void SetAlphaColor(float, class UIColor *) = 0;
    virtual void GrowBoundingBox(Box &) const = 0;
};
