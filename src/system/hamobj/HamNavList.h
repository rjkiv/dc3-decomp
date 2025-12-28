#pragma once
#include "HamListRibbon.h"
#include "HamNavProvider.h"
#include "HamScrollBehavior.h"
#include "gesture/Skeleton.h"
#include "hamobj/HamScrollSpeedIndicator.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Anim.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIComponent.h"
#include "ui/UIListDir.h"
#include "ui/UIListProvider.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "utl/MemMgr.h"

/** "List of navigation actions controlled by a single hand with gestures" */
class HamNavList : public UIComponent,
                   public RndAnimatable,
                   public UIListProvider,
                   public UIListStateCallback,
                   public SkeletonCallback {
public:
    enum NavInputType {
        kNavInput_RightHand = 0,
        kNavInput_LeftHand = 1
    };

    // Hmx::Object
    virtual ~HamNavList();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(HamNavList);
    OBJ_SET_TYPE(HamNavList);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual bool CanHaveFocus();
    // RndAnimatable
    virtual float StartFrame();
    virtual float EndFrame();
    // UIListProvider
    virtual int NumData() const;
    // UIListStateCallback
    virtual void StartScroll(const UIListState &, int, bool);
    virtual void CompleteScroll(const UIListState &);
    // SkeletonCallback
    virtual void Clear();
    virtual void Update(const struct SkeletonUpdateData &) {}
    virtual void PostUpdate(const struct SkeletonUpdateData *);
    virtual void Draw(const BaseSkeleton &, class SkeletonViz &);

    OBJ_MEM_OVERLOAD(0x26)
    NEW_OBJ(HamNavList)
    void Refresh();
    void HandleHighlightChanged(int);
    void PlayScrollSound();
    void StopScrollSound();
    void SetScrollSoundFrame(float);
    void SetNavProvider(HamNavProvider *);
    Symbol GetSelectedSym() const;
    void ScrollToIndex(int, int);
    void PlayEnterAnim();
    void ScrollSubList(int, int);
    void ScrollSubListToIndex(int, int);
    bool IsDataHeader(int);
    void SetProvider(UIListProvider *);
    void PushBackBigElement(Symbol);
    void EraseBigElement(int);
    void SetHighButtonMode(bool);
    void AddRibbonSinks(Hmx::Object *, Symbol);
    void RemoveRibbonSinks(Hmx::Object *, Symbol);
    void DoSelectFor(int);
    void SendHighlightMsg(int);
    void SendHighlightSettledMsg(int);
    void ClearBigElements();
    void HideItem(int, bool);
    void SetProviderNavItemLabels(int, DataArray *);
    void Enable() { mEnabled = true; }

    HamNavProvider *GetHelpbarProvider() { return mNavProvider; }

    static void Init();
    static bool sLastSelectInControllerMode;

private:
    void SetRibbonMode(HamListRibbon::RibbonMode);
    void SetHighlight(int);
    void SetSliding(float);
    void SetSelecting(bool);
    bool SkipPoll() const;
    void RealRefresh();
    void SetSwelling();
    bool ShouldSkipSelectAnim(DataNode &) const;
    bool ShouldSkipSelectSound(DataNode &) const;
    int NumItems() const;
    int GetDisabledCount(int) const;
    int GetHighlightItem(void) const;
    void DetermineHighlightedItem();

    static float sSlideSmoothAmount;
    static float sSlideTrendAmount;

    DataNode OnMsg(const ButtonDownMsg &);

protected:
    virtual void OldResourcePreload(BinStream &);

    HamNavList();

    void Update();
    void SetControllerFocus(int);

    NavInputType mNavInputType; // 0x60
    std::vector<UIListWidget *> unk64; // 0x64
    UIListState mListState; // 0x70
    std::vector<HamListRibbonDrawState> mRibbonDrawStates; // 0xb8
    /** "Mode for animations" */
    HamListRibbon::RibbonMode mRibbonMode; // 0xc4
    bool unkc8; // 0xc8
    /** "HamListRibbon resource file" */
    ResourceDirPtr<HamListRibbon> mListRibbonResource; // 0xcc
    /** "HamListRibbon resource file" */
    ResourceDirPtr<HamListRibbon> mHeaderRibbonResource; // 0xe4
    /** "UIListDir resource file" */
    ResourceDirPtr<UIListDir> mListDirResource; // 0xfc
    /** "HamScrollSpeedIndicator resource file" */
    ResourceDirPtr<HamScrollSpeedIndicator> mScrollSpeedIndicatorResource; // 0x114
    ObjPtr<HamNavProvider> mNavProvider; // 0x12c
    ObjPtr<RndAnimatable> mScrollSpeedAnim; // 0x140
    bool unk154; // 0x154
    /** "Skip the enter anim altogether" */
    bool mSkipEnterAnim; // 0x155
    /** "Don't automatically play the enter anim when this component enters" */
    bool mSuppressAutomaticEnter; // 0x156
    bool unk157; // 0x157
    float unk158; // 0x158
    DoubleExponentialSmoother unk15c; // 0x15c
    DoubleExponentialSmoother unk170; // 0x170
    int unk184;
    int unk188;
    int mSkeletonTrackingID; // 0x18c
    HamScrollBehavior unk190;
    bool mDisableSlideSound; // 0x1e4
    bool mDisableSelectSound; // 0x1e5
    bool mEnabled; // 0x1e6
    bool unk1e7; // 0x1e7
    /** "Automatically tie this navlist to the active skeleton" */
    bool mAlwaysUseActiveSkeleton; // 0x1e8
    /** "This list can only be used when it is focused" */
    bool mOnlyUseWhenFocused; // 0x1e9
    float unk1ec; // 0x1ec
    bool unk1f0; // 0x1f0
    Symbol unk1f4; // 0x1f4
    int unk1f8;
    bool unk1fc;
    bool unk1fd;
    bool unk1fe;
    /** "Elements that match these will be bigger than the other elements" */
    std::vector<Symbol> mBigElements; // 0x200
    std::vector<int> unk20c; // 0x20c
};
