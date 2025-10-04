#include "hamobj/HamNavList.h"
#include "HamListRibbon.h"
#include "HamNavList.h"
#include "HamScrollBehavior.h"
#include "gesture/GestureMgr.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Trans.h"
#include "synth/Sound.h"
#include "ui/UIComponent.h"
#include "utl/BinStream.h"
#include "utl/Std.h"

HamNavList::HamNavList()
    : mNavInputType(kNavInput_RightHand), mListState(this, this),
      mRibbonMode(HamListRibbon::kRibbonSlide), unkc8(0), mListRibbonResource(this),
      mHeaderRibbonResource(this), mListDirResource(this),
      mScrollSpeedIndicatorResource(this), mNavProvider(this), mScrollSpeedAnim(this),
      unk154(0), mSkipEnterAnim(0), mSuppressAutomaticEnter(0), unk157(0), unk158(0),
      unk15c(0, 10, 10), unk170(0, 10, 0), unk184(0), unk188(0), mSkeletonTrackingID(0),
      unk190(this, &mListState), mDisableSlideSound(0), mDisableSelectSound(0),
      mEnabled(1), unk1e7(1), mAlwaysUseActiveSkeleton(1), mOnlyUseWhenFocused(1),
      unk1ec(0), unk1f0(0), unk1f8(-1), unk1fd(0), unk1fe(0) {
    mListState.SetSpeed(0);
    mListState.SetSelected(0, -1, true);
    SetRate(k30_fps_ui);
}

HamNavList::~HamNavList() {
    DeleteAll(unk64);
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
    // delete unk184;
    // delete unk188;
    if (mListRibbonResource) {
        Sound *slideSound = mListRibbonResource->SlideSound();
        if (slideSound)
            slideSound->Stop(nullptr, false);
    }
}

bool HamNavList::Replace(ObjRef *ref, Hmx::Object *obj) {
    return RndTransformable::Replace(ref, obj);
}

BEGIN_HANDLERS(HamNavList)
    HANDLE_ACTION(set_provider, SetProvider(_msg->Obj<UIListProvider>(2)))
    HANDLE_ACTION(set_highlight, SetHighlight(_msg->Int(2)))
    HANDLE_ACTION(set_selected, mListState.SetSelected(_msg->Int(2), -1, true))
    HANDLE_ACTION(set_swelling, SetSwelling())
    HANDLE_ACTION(set_sliding, SetSliding(_msg->Float(2)))
    HANDLE_ACTION(set_selecting, SetSelecting(false))
    HANDLE_EXPR(get_selected, mListState.Selected())
    HANDLE_EXPR(get_selected_sym, GetSelectedSym())
    HANDLE_EXPR(is_scrolling_settled, unk1ec <= 0)
    HANDLE_ACTION(scroll_to_index, ScrollToIndex(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(get_top_index, mListState.FirstShowing())
    HANDLE_ACTION(refresh, unk1f0 = true)
    HANDLE_ACTION(set_controller_focus, SetControllerFocus(_msg->Int(2)))
    HANDLE_ACTION(play_enter_anim, PlayEnterAnim())
    HANDLE_ACTION(enable_navigation, mEnabled = true)
    HANDLE_ACTION(disable_navigation, mEnabled = false)
    HANDLE_ACTION(enable_selection, unk1e7 = true)
    HANDLE_ACTION(disable_selection, unk1e7 = false)
    HANDLE_ACTION(scroll_sublist, ScrollSubList(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(
        scroll_sublist_to_index, ScrollSubListToIndex(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_ACTION(push_back_big_element, PushBackBigElement(_msg->Sym(2)))
    HANDLE_ACTION(pop_back_big_element, mBigElements.pop_back())
    HANDLE_ACTION(erase_big_element, EraseBigElement(_msg->Int(2)))
    HANDLE_ACTION(push_back_big_element_index, unk20c.push_back(_msg->Int(2)))
    HANDLE_ACTION(pop_back_big_element_index, unk20c.pop_back())
    HANDLE_EXPR(is_data_header, IsDataHeader(_msg->Int(2)))
    HANDLE_EXPR(get_num_display, mListState.NumDisplay())
    HANDLE_EXPR(data_index, mListState.Provider()->DataIndex(_msg->Sym(2)))
    HANDLE_EXPR(data_symbol, mListState.Provider()->DataSymbol(_msg->Int(2)))
    HANDLE_EXPR(index_enabled, mListState.Provider()->IsActive(_msg->Int(2)))
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(UIComponent)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamNavList)
    SYNC_PROP_MODIFY(list_ribbon_resource, mListRibbonResource, Update())
    SYNC_PROP_MODIFY(header_ribbon_resource, mHeaderRibbonResource, Update())
    SYNC_PROP_MODIFY(list_dir_resource, mListDirResource, Update())
    SYNC_PROP_MODIFY(
        scroll_speed_indicator_resource, mScrollSpeedIndicatorResource, Update()
    )
    SYNC_PROP_SET(mode, mRibbonMode, SetRibbonMode((HamListRibbon::RibbonMode)_val.Int()))
    SYNC_PROP_SET(
        nav_provider, mNavProvider.Ptr(), SetNavProvider(_val.Obj<HamNavProvider>())
    )
    SYNC_PROP(disable_select_sound, mDisableSelectSound)
    SYNC_PROP(disable_slide_sound, mDisableSlideSound)
    SYNC_PROP(skeleton_tracking_id, mSkeletonTrackingID)
    SYNC_PROP(enabled, mEnabled)
    SYNC_PROP(always_use_active_skeleton, mAlwaysUseActiveSkeleton)
    SYNC_PROP(only_use_when_focused, mOnlyUseWhenFocused)
    SYNC_PROP_SET(nav_input_type, mNavInputType, mNavInputType = (NavInputType)_val.Int())
    SYNC_PROP(scroll_speed_anim, mScrollSpeedAnim)
    SYNC_PROP(suppress_automatic_enter, mSuppressAutomaticEnter)
    SYNC_PROP(big_elements, mBigElements)
    SYNC_PROP(skip_enter_anim, mSkipEnterAnim)
    SYNC_SUPERCLASS(UIComponent)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamNavList)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(UIComponent)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mListRibbonResource;
    bs << mListDirResource;
    bs << mNavProvider;
    bs << mDisableSelectSound;
    bs << mDisableSlideSound;
    bs << mEnabled;
    bs << mAlwaysUseActiveSkeleton;
    bs << mNavInputType;
    bs << mOnlyUseWhenFocused;
    bs << mScrollSpeedAnim;
    bs << mSuppressAutomaticEnter;
    bs << mBigElements;
    bs << mHeaderRibbonResource;
    bs << mScrollSpeedIndicatorResource;
    bs << mSkipEnterAnim;
END_SAVES

BEGIN_COPYS(HamNavList)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(HamNavList)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mListDirResource)
        COPY_MEMBER(mNavProvider)
        COPY_MEMBER(mListRibbonResource)
        COPY_MEMBER(mDisableSelectSound)
        COPY_MEMBER(mDisableSlideSound)
        COPY_MEMBER(mEnabled)
        COPY_MEMBER(mAlwaysUseActiveSkeleton)
        COPY_MEMBER(mOnlyUseWhenFocused)
        COPY_MEMBER(mNavInputType)
        COPY_MEMBER(mScrollSpeedAnim)
        COPY_MEMBER(mSuppressAutomaticEnter)
        COPY_MEMBER(mBigElements)
        COPY_MEMBER(mHeaderRibbonResource)
        COPY_MEMBER(mScrollSpeedIndicatorResource)
        COPY_MEMBER(mSkipEnterAnim)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(HamNavList)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void HamNavList::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(10, 0)
    UIComponent::PreLoad(bs);
    if (gRev >= 2) {
        LOAD_SUPERCLASS(RndAnimatable)
    }
    if (gRev >= 1) {
        bs >> mListRibbonResource;
        bs >> mListDirResource;
    } else {
        char buf[0x100];
        bs.ReadString(buf, 0x100);
        mListDirResource.SetName(buf, true);
    }
    bs >> mNavProvider;
    SetNavProvider(mNavProvider);
    if (gRev >= 3) {
        bsrev >> mDisableSelectSound;
        bsrev >> mDisableSlideSound;
        bsrev >> mEnabled;
        bsrev >> mAlwaysUseActiveSkeleton;
        bsrev >> (int &)mNavInputType; // should be BinStreamEnum read
    }
    if (gRev >= 5) {
        bsrev >> mOnlyUseWhenFocused;
    }
    if (gRev >= 4) {
        bs >> mScrollSpeedAnim;
    }
    if (gRev >= 6) {
        bsrev >> mSuppressAutomaticEnter;
    }
    if (gRev >= 7) {
        bsrev >> mBigElements;
    }
    if (gRev >= 8) {
        bs >> mHeaderRibbonResource;
    }
    if (gRev >= 9) {
        bs >> mScrollSpeedIndicatorResource;
    }
    if (gRev >= 10) {
        bsrev >> mSkipEnterAnim;
    }
    bsrev.PushRev(this);
}

void HamNavList::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    UIComponent::PostLoad(bs);
    mListDirResource.PostLoad(nullptr);
    mListRibbonResource.PostLoad(nullptr);
    mHeaderRibbonResource.PostLoad(nullptr);
    mScrollSpeedIndicatorResource.PostLoad(nullptr);
    Update();
}

void HamNavList::SetControllerFocus(int i1) {
    if (TheGestureMgr && TheGestureMgr->InControllerMode()) {
        SetHighlight(i1);
    }
}

void HamNavList::Init() {
    REGISTER_OBJ_FACTORY(HamNavList);
    DataArray *cfg = SystemConfig("ui");
    cfg->FindData("slide_smooth_amount", sSlideSmoothAmount, false);
    cfg->FindData("slide_trend_amount", sSlideTrendAmount, false);
    HamScrollBehavior::Init();
}

void HamNavList::PushBackBigElement(Symbol element) { mBigElements.push_back(element); }
void HamNavList::EraseBigElement(int idx) {
    mBigElements.erase(mBigElements.begin() + idx);
}
