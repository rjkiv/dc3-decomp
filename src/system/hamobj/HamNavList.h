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

class HamNavList : public UIComponent,
                   public RndAnimatable,
                   public UIListProvider,
                   public UIListStateCallback,
                   public SkeletonCallback {
public:
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
    virtual void OldResourcePreload(BinStream &);
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

private:
    void SetRibbonMode(HamListRibbon::RibbonMode);
    void SetHighlight(int);
    void SetSliding(float);
    void SetSelecting(bool);

    DataNode OnMsg(const ButtonDownMsg &);

protected:
    HamNavList();

    void Update();
    void SetSwelling();
    void SetControllerFocus(int);

    int mNavInputType; // 0x60
    std::vector<UIListWidget *> unk64; // 0x64
    UIListState mListState; // 0x70
    std::vector<HamListRibbonDrawState> unkb8; // 0xb8
    HamListRibbon::RibbonMode mRibbonMode; // 0xc4
    bool unkc8; // 0xc8
    ResourceDirPtr<HamListRibbon> mListRibbonResource; // 0xcc
    ResourceDirPtr<HamListRibbon> mHeaderRibbonResource; // 0xe4
    ResourceDirPtr<UIListDir> mListDirResource; // 0xfc
    ResourceDirPtr<HamScrollSpeedIndicator> mScrollSpeedIndicatorResource; // 0x114
    ObjPtr<HamNavProvider> mNavProvider; // 0x12c
    ObjPtr<RndAnimatable> mScrollSpeedAnim; // 0x140
    bool unk154; // 0x154
    bool mSkipEnterAnim; // 0x155
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
    bool mAlwaysUseActiveSkeleton; // 0x1e8
    bool mOnlyUseWhenFocused; // 0x1e9
    float unk1ec; // 0x1ec
    bool unk1f0; // 0x1f0
    Symbol unk1f4; // 0x1f4
    int unk1f8;
    bool unk1fc;
    bool unk1fd;
    bool unk1fe;
    std::vector<Symbol> mBigElements; // 0x200
    std::vector<int> unk20c; // 0x20c
};
