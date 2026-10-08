#include "hamobj/HamNavList.h"
#include "HamListRibbon.h"
#include "HamNavList.h"
#include "HamScrollBehavior.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DirectionGestureFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/HandHeightGestureFilter.h"
#include "gesture/HandsUpGestureFilter.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/HamNavProvider.h"
#include "macros.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Vec.h"
#include "meta/MetaMusicManager.h"
#include "meta_ham/ShellInput.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/Anim.h"
#include "rndobj/Overlay.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "stl/_vector.h"
#include "synth/Sound.h"
#include "ui/UI.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UIListProvider.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "ui/Utl.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <cstdio>

bool HamNavList::sForceDisengage;
bool HamNavList::sLastSelectInControllerMode;

float HamNavList::sSlideSmoothAmount = 10;
float HamNavList::sSlideTrendAmount = 10;

const int HamNavList::sListStateMinDisplay = 7;

HamNavList::HamNavList()
    : mNavInputType(kNavInput_RightHand), mListState(this, this),
      mRibbonMode(HamListRibbon::kRibbonSlide), mFiltersUpdated(0), mListRibbon(this),
      mHeaderRibbon(this), mListDir(this),
      mScrollSpeedIndicator(this), mNavProvider(this), mScrollSpeedAnim(this),
      mPlayEnterAnim(0), mSkipEnterAnim(0), mSuppressAutomaticEnter(0), mSuppressingEnter(0), mHandHeight(0),
      mSlideAmount(0, 10, 10), mDisengageAmount(0, 10, 0), mDirectionFilter(0), mHandHeightFilter(0), mSkeletonTrackingID(0),
      mScrollBehavior(this, &mListState), mDisableSlideSound(0), mDisableSelectSound(0),
      mEnabled(1), mEnableSelection(1), mAlwaysUseActiveSkeleton(1), mOnlyUseWhenFocused(1),
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
    delete mDirectionFilter;
    delete mHandHeightFilter;
    if (mListRibbon) {
        mListRibbon->StopSlideSound();
    }
}

bool HamNavList::Replace(ObjRef *ref, Hmx::Object *obj) {
    return RndTransformable::Replace(ref, obj);
}

BEGIN_HANDLERS(HamNavList)
    HANDLE_ACTION(set_provider, SetProvider(_msg->Obj<UIListProvider>(2)))
    HANDLE_ACTION(set_highlight, SetHighlight(_msg->Int(2)))
    HANDLE_ACTION(set_selected, SetSelected(_msg->Int(2)))
    HANDLE_ACTION(set_swelling, SetSwelling())
    HANDLE_ACTION(set_sliding, SetSliding(_msg->Float(2)))
    HANDLE_ACTION(set_selecting, SetSelecting(false))
    HANDLE_EXPR(get_selected, mListState.Selected())
    HANDLE_EXPR(get_selected_sym, GetSelectedSym())
    HANDLE_EXPR(is_scrolling_settled, IsScrollingSettled())
    HANDLE_ACTION(scroll_to_index, ScrollToIndex(_msg->Int(2), _msg->Int(3)))
    HANDLE_EXPR(get_top_index, mListState.FirstShowing())
    HANDLE_ACTION(refresh, unk1f0 = true)
    HANDLE_ACTION(set_controller_focus, SetControllerFocus(_msg->Int(2)))
    HANDLE_ACTION(play_enter_anim, PlayEnterAnim())
    HANDLE_ACTION(enable_navigation, mEnabled = true)
    HANDLE_ACTION(disable_navigation, mEnabled = false)
    HANDLE_ACTION(enable_selection, mEnableSelection = true)
    HANDLE_ACTION(disable_selection, mEnableSelection = false)
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
    SYNC_PROP_MODIFY(list_ribbon_resource, mListRibbon, Update())
    SYNC_PROP_MODIFY(header_ribbon_resource, mHeaderRibbon, Update())
    SYNC_PROP_MODIFY(list_dir_resource, mListDir, Update())
    SYNC_PROP_MODIFY(
        scroll_speed_indicator_resource, mScrollSpeedIndicator, Update()
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
    bs << mListRibbon;
    bs << mListDir;
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
    bs << mHeaderRibbon;
    bs << mScrollSpeedIndicator;
    bs << mSkipEnterAnim;
END_SAVES

BEGIN_COPYS(HamNavList)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(HamNavList)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mListDir)
        COPY_MEMBER(mNavProvider)
        COPY_MEMBER(mListRibbon)
        COPY_MEMBER(mDisableSelectSound)
        COPY_MEMBER(mDisableSlideSound)
        COPY_MEMBER(mEnabled)
        COPY_MEMBER(mAlwaysUseActiveSkeleton)
        COPY_MEMBER(mOnlyUseWhenFocused)
        COPY_MEMBER(mNavInputType)
        COPY_MEMBER(mScrollSpeedAnim)
        COPY_MEMBER(mSuppressAutomaticEnter)
        COPY_MEMBER(mBigElements)
        COPY_MEMBER(mHeaderRibbon)
        COPY_MEMBER(mScrollSpeedIndicator)
        COPY_MEMBER(mSkipEnterAnim)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(HamNavList)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(10, 0)

void HamNavList::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(10, 0)
    UIComponent::PreLoad(d.stream);
    if (d.rev >= 2) {
        LOAD_SUPERCLASS(RndAnimatable)
    }
    if (d.rev >= 1) {
        d >> mListRibbon;
        d >> mListDir;
    } else {
        char buf[0x100];
        d.stream.ReadString(buf, 0x100);
        mListDir.SetName(buf, true);
    }
    d >> mNavProvider;
    SetNavProvider(mNavProvider);
    if (d.rev >= 3) {
        d >> mDisableSelectSound;
        d >> mDisableSlideSound;
        d >> mEnabled;
        d >> mAlwaysUseActiveSkeleton;
        d >> (BinStreamEnum<NavInputType> &)mNavInputType;
    }
    if (d.rev >= 5) {
        d >> mOnlyUseWhenFocused;
    }
    if (d.rev >= 4) {
        d >> mScrollSpeedAnim;
    }
    if (d.rev >= 6) {
        d >> mSuppressAutomaticEnter;
    }
    if (d.rev >= 7) {
        d >> mBigElements;
    }
    if (d.rev >= 8) {
        d >> mHeaderRibbon;
    }
    if (d.rev >= 9) {
        d >> mScrollSpeedIndicator;
    }
    if (d.rev >= 10) {
        d >> mSkipEnterAnim;
    }
    d.PushRev(this);
}

void HamNavList::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    UIComponent::PostLoad(bs);
    mListDir.PostLoad(nullptr);
    mListRibbon.PostLoad(nullptr);
    mHeaderRibbon.PostLoad(nullptr);
    mScrollSpeedIndicator.PostLoad(nullptr);
    Update();
}

void HamNavList::DrawShowing() {
    if (mListDir && !unk1f0) {
        unk1ec = TheTaskMgr.UISeconds();
        UIListWidgetDrawState drawState;
        mListDir->BuildDrawState(
            drawState, mListState, kFocused, 0, !mScrollBehavior.IsScrolling()
        );
        LinkRibbonDrawState(mRibbonDrawStates, drawState);
        if (mListState.ScrollPastMinDisplay()) {
            for (int i = 0; i < mRibbonDrawStates.size(); i++) {
                if (i < mListState.MinDisplay()
                    || i >= mListState.MinDisplay() + HamListRibbon::sNumListSelectable) {
                    mRibbonDrawStates[i].unk0.SetParams(0, 0, 0);
                }
            }
        }
        if (mListRibbon) {
            mListRibbon->SetFrame(GetFrame(), 1);
            mListRibbon->SetScrollAnimFrame(mScrollBehavior.GetUnk20());
            mListRibbon->SetDisengageFrame(mDisengageAmount.Level());
            mListRibbon->SetMode(mRibbonMode);
            mListRibbon->Draw(WorldXfm(), mRibbonDrawStates, false, false);
        }
        if (mHeaderRibbon) {
            mHeaderRibbon->SetFrame(GetFrame(), 1);
            mHeaderRibbon->SetScrollAnimFrame(mScrollBehavior.GetUnk20());
            mHeaderRibbon->SetDisengageFrame(mDisengageAmount.Level());
            mHeaderRibbon->SetMode(mRibbonMode);
            mHeaderRibbon->Draw(WorldXfm(), mRibbonDrawStates, true, false);
        }
        for (int i = 0; i < mRibbonDrawStates.size(); i++) {
            if (mRibbonDrawStates[i].unk1c && mRibbonDrawStates[i].unk18) {
                mRibbonDrawStates[i].unk18->mAlpha = 0;
            }
        }
        mListDir->DrawWidgets(
            drawState, mListState, unk64, WorldXfm(), GetState(), nullptr, false
        );
        if (mScrollSpeedIndicator) {
            mScrollSpeedIndicator->Draw(WorldXfm());
        }
    }
}

void HamNavList::SetControllerFocus(int i1) {
    if (InControllerMode()) {
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

bool HamNavList::SkipPoll() const {
    float uiSeconds = (float)TheTaskMgr.UISeconds();
    if (unk1ec < uiSeconds - 0.5f) {
        return true;
    } else {
        return mNavInputType == kNavInput_RightHand && mOnlyUseWhenFocused
            && TheUI->FocusComponent() != this;
    }
    return false;
}

void HamNavList::Refresh() { unk1f0 = true; }

void HamNavList::SetHighButtonMode(bool b) {
    unk1fe = b;
    if (mDirectionFilter) {
        mDirectionFilter->SetHighButtonMode(b);
    }
}

int HamNavList::NumData() const { return 18; }

void HamNavList::SetSwelling() {
    if (unk1f0)
        RealRefresh();

    if (mRibbonMode != HamListRibbon::kRibbonSelect) {
        if (mRibbonMode == HamListRibbon::kRibbonDisengaged) {
            SetHighlight(mListState.Selected());
        }
        SetRibbonMode(HamListRibbon::kRibbonSwell);
        float uiSeconds = TheTaskMgr.DeltaUISeconds();
        mSlideAmount.Smooth(0.0f, uiSeconds);
    }
}

bool HamNavList::CanHaveFocus() { return mNavInputType == kNavInput_RightHand; }

bool HamNavList::ShouldSkipSelectAnim(DataNode &node) const {
    UIListProvider *provider = mListState.Provider();
    if (!provider || 1 < mListState.NumShowing()) {
        if (node.Type() != kDataSymbol)
            return false;

        static Symbol skip_select_anim("skip_select_anim");
        static Symbol skip_select_anim_and_sound("skip_select_anim_and_sound");
        if (node.Sym(0) != skip_select_anim) {
            if (node.Sym(0) != skip_select_anim_and_sound)
                return false;
        }
    }
    return true;
}

bool HamNavList::ShouldSkipSelectSound(DataNode &node) const {
    if (node.Type() != kDataSymbol) {
        return false;
    } else {
        static Symbol skip_select_sound("skip_select_sound");
        static Symbol skip_select_anim_and_sound("skip_select_anim_and_sound");
        if (node.Sym(0) != skip_select_sound) {
            if (node.Sym(0) != skip_select_anim_and_sound)
                return false;
        }
        return true;
    }
}

void HamNavList::AddRibbonSinks(Hmx::Object *o, Symbol s) {
    if (mListRibbon && o)
        o->AddSink(mListRibbon, s);
    if (mHeaderRibbon && o)
        o->AddSink(mHeaderRibbon, s);
}

void HamNavList::RemoveRibbonSinks(Hmx::Object *o, Symbol s) {
    if (mListRibbon && o)
        o->RemoveSink(mListRibbon, s);
    if (mHeaderRibbon && o)
        o->RemoveSink(mHeaderRibbon, s);
}

void HamNavList::DoSelectFor(int i) {
    if (unk1f0)
        RealRefresh();
    mListState.SetSelected(i, mListState.FirstShowing(), true);
    sLastSelectInControllerMode = true;
    SetSelecting(true);
}

void HamNavList::HandleHighlightChanged(int i) {
    if (0 <= i && i < mListState.NumShowing()) {
        SendHighlightMsg(i);
        bool sendMsg = mScrollBehavior.GetFirstVal() <= 0.0f;
        if (sendMsg) {
            SendHighlightSettledMsg(i);
        }
        if (TheGestureMgr->GetBool4271() && mListRibbon) {
            mListRibbon->PlayHighlightSound(i);
        }
    }
}

void HamNavList::OldResourcePreload(BinStream &bs) {
    char name[256];
    bs.ReadString(name, 0x100);
    mListRibbon.SetName(name, true);
}

void HamNavList::HideItem(int index, bool b) {
    if (unk1f0)
        RealRefresh();
    MILO_ASSERT_RANGE(index, 0, mRibbonDrawStates.size(), 0x527);
    mRibbonDrawStates[index].unk1c = b;
    if (mNavProvider)
        mNavProvider->SetEnabled(index, b == false);
}

bool HamNavList::IsDataHeader(int i) {
    if (mListState.Provider()) {
        UIListProvider *p = mListState.Provider();
        return p->IsHeader(i);
    } else {
        return false;
    }
}

void HamNavList::ScrollSubList(int i, int j) {
    if (unk1f0)
        RealRefresh();

    UIList *list = mListDir->SubList(i, unk64);
    if (list)
        list->Scroll(j);
}

void HamNavList::ScrollSubListToIndex(int i, int j) {
    if (unk1f0)
        RealRefresh();

    UIList *list = mListDir->SubList(i, unk64);
    if (list)
        list->SetSelected(j, j);
}

int HamNavList::NumItems() const {
    if (mListState.ScrollPastMinDisplay()) {
        if (mScrollBehavior.AtTop() || mScrollBehavior.AtBottom()) {
            return HamListRibbon::sNumListSelectable + 1;
        } else {
            return HamListRibbon::sNumListSelectable + 2;
        }
    } else {
        return mListState.NumShowing() - GetDisabledCount(mListState.NumShowing());
    }
}

float HamNavList::StartFrame() {
    if (mListRibbon) {
        mListRibbon->StartFrame();
    } else {
        return 0.0f;
    }
}

float HamNavList::EndFrame() {
    if (mListRibbon) {
        mListRibbon->EndFrame();
    } else {
        return 0.0f;
    }
}

void HamNavList::SendHighlightSettledMsg(int i) {
    UIListProvider *provider = mListState.Provider();
    MILO_ASSERT(provider, 0x327);
    bool canSelect = provider->CanSelect(i);

    if (provider->NumData() != 0) {
        NavHighlightSettledMsg msg(provider->DataSymbol(i), i, this, canSelect);

        TheUI->Handle(msg, false);
        Export(msg, true);
        TheHamProvider->Handle(msg, false);
    }
}

void HamNavList::SetProvider(UIListProvider *p) {
    UIListProvider *provider = mListState.Provider();
    if (p == provider) {
        RealRefresh();
    } else {
        if (mListState.ScrollPastMinDisplay()) {
            mScrollBehavior.Exit();
        }
        mListState.SetProvider(p, mListDir);
        RealRefresh();
        mListState.SetSelected(0, -1, true);
        if (mListState.ScrollPastMinDisplay())
            mScrollBehavior.Enter();
    }
}

void HamNavList::SetProviderNavItemLabels(int i, DataArray *d) {
    mNavProvider->SetLabels(i, d);
}

void HamNavList::StartScroll(UIListState const &state, int i, bool b) {
    if (mListDir) {
        mListDir->StartScroll(state, unk64, i, b);
    }
}

Symbol HamNavList::GetSelectedSym() const {
    UIListProvider *provider = mListState.Provider();
    if (provider) {
        Symbol s = provider->DataSymbol(mListState.SelectedData());
        if (s == gNullStr) {
            MILO_FAIL("DataSymbol() not implemented in UIList provider");
        }
        return s;
    } else {
        return gNullStr;
    }
}

void HamNavList::SendHighlightMsg(int i) {
    if (unk1f0)
        RealRefresh();
    UIListProvider *provider = mListState.Provider();
    MILO_ASSERT(provider, 0x339);
    bool canSel = provider->CanSelect(i);

    NavHighlightMsg msg(provider->DataSymbol(i), i, this, canSel);
    TheUI->Handle(msg, false);
    Export(msg, true);
    TheHamProvider->Handle(msg, false);
}

int HamNavList::GetHighlightItem() const {
    if (mListRibbon) {
        int numShowing = mListState.NumShowing();
        if (mListRibbon->IsScrollable(numShowing)) {
            int selDisplay = mListState.SelectedDisplay();
            int minDisplay = mListState.MinDisplay();
            return selDisplay - minDisplay;
        } else {
            return mListState.SelectedDisplay()
                - GetDisabledCount(mListState.SelectedDisplay());
        }
    } else
        return 0;
}

void HamNavList::SetSliding(float f) {
    if (unk1f0)
        RealRefresh();

    if (mRibbonMode != HamListRibbon::kRibbonSelect) {
        float f1 = 0.0f;
        if (mRibbonMode != HamListRibbon::kRibbonSlide) {
            mSlideAmount.Reset();
            SetRibbonMode(HamListRibbon::kRibbonSlide);
        }
        if (sSlideSmoothAmount == 0) {
            mSlideAmount.SetParams(f, f, 0);
        } else {
            mSlideAmount.Smooth(f, TheTaskMgr.DeltaUISeconds());
        }
        SetFrame(mSlideAmount.Level(), 1.0f);
    }
}

void HamNavList::Draw(const BaseSkeleton &baseSkeleton, SkeletonViz &skeletonViz) {
    const Skeleton *skeleton = dynamic_cast<const Skeleton *>(&baseSkeleton);
    MILO_ASSERT(skeleton, 0x5a3);
    mDirectionFilter->Draw(*skeleton, skeletonViz);
}

void HamNavList::SetHighlight(int i) {
    if (unk1f0)
        RealRefresh();
    UIListProvider *provider = mListState.Provider();
    if (provider && (0 <= i) && (i < mListState.NumShowing())) {
        mDirectionFilter->ResetHoverTimer();
        mListState.SetSelected(i, mListState.FirstShowing(), true);
        HandleHighlightChanged(i);
    }
}

void HamNavList::Update() {
    static float sFloat = 0.1f;
    delete mDirectionFilter;
    delete mHandHeightFilter;
    if (mNavInputType == kNavInput_RightHand) {
        if (!TheGestureMgr->InDoubleUserMode()) {
            mDirectionFilter = new DirectionGestureFilterSingleUser(
                kSkeletonRight, kSkeletonLeft, sFloat, -0.2f
            );
        } else {
            mDirectionFilter = new DirectionGestureFilterDoubleUser(
                kSkeletonRight, kSkeletonLeft, sFloat, -0.2f
            );
        }
        mHandHeightFilter = new HandHeightGestureFilter(kSkeletonRight);
    } else {
        if (!TheGestureMgr->InDoubleUserMode()) {
            mDirectionFilter = new DirectionGestureFilterSingleUser(
                kSkeletonLeft, kSkeletonRight, sFloat, -0.1f
            );
        } else {
            mDirectionFilter = new DirectionGestureFilterDoubleUser(
                kSkeletonLeft, kSkeletonRight, sFloat, -0.1f
            );
        }
        mHandHeightFilter = new HandHeightGestureFilter(kSkeletonLeft);
    }

    unk1fd = TheGestureMgr->InDoubleUserMode();
    if (mDirectionFilter) {
        mDirectionFilter->SetHighButtonMode(unk1fe);
    }

    if (mNavInputType == kNavInput_RightHand) {
        mListState.SetNumDisplay(10, true);
    } else {
        mListState.SetNumDisplay(2, true);
    }

    int numDisplay = mListState.NumDisplay();
    mRibbonDrawStates.resize(numDisplay);

    if (mListDir) {
        int numShowing = mListState.NumDisplay();
        mListDir->CreateElements(nullptr, unk64, numShowing);
    }
    unk1f0 = true;
}

void HamNavList::PostUpdate(SkeletonUpdateData const *data) {
    if (data && !SkipPoll()) {
        mFiltersUpdated = true;
        for (int i = 0; i < 6; i++) {
            const Skeleton *skeleton = data->unk4[i];
            if (skeleton->TrackingID() == mSkeletonTrackingID) {
                int elapsedMs = skeleton->ElapsedMs();
                mDirectionFilter->Update(*skeleton, 0 < elapsedMs ? elapsedMs : 0);
                elapsedMs = skeleton->ElapsedMs(); // idk why i have to say this again but
                                                   // it matches
                mHandHeightFilter->Update(*skeleton, 0 < elapsedMs ? elapsedMs : 0);
                return;
            }
        }
    }
}

void HamNavList::Clear() {
    mDirectionFilter->Clear();
    mHandHeightFilter->Clear();
}

void HamNavList::SetNavProvider(HamNavProvider *provider) {
    mNavProvider = provider;
    if (provider) {
        provider->SetNavList(this);
        SetProvider(provider);
    } else {
        SetProvider(this);
    }
}

void HamNavList::SetRibbonMode(HamListRibbon::RibbonMode mode) {
    if (mRibbonMode != mode) {
        if ((mNavInputType != kNavInput_RightHand || !TheMetaMusicManager)
            && mNavInputType == kNavInput_LeftHand) {
            if (!InControllerMode()) {
                if (mode == HamListRibbon::kRibbonDisengaged) {
                    static LeftHandListEngagementMsg leftHandListDisengaged(false);
                    TheUI->Handle(leftHandListDisengaged, false);
                }
                if (mRibbonMode == HamListRibbon::kRibbonDisengaged) {
                    static LeftHandListEngagementMsg leftHandListEngaged(true);
                    TheUI->Handle(leftHandListEngaged, false);
                }
            }
        }
        mRibbonMode = mode;
        if (mListRibbon) {
            mListRibbon->SetMode(mode);
        }
        if (mHeaderRibbon) {
            mHeaderRibbon->SetMode(mode);
        }
    }
}

void HamNavList::ClearBigElements() {
    mBigElements.clear();
    unk20c.clear();
}

void HamNavList::Exit() {
    UIComponent::Exit();
    SkeletonUpdateHandle updateHandle = SkeletonUpdate::InstanceHandle();
    if (updateHandle.HasCallback(this)) {
        updateHandle.RemoveCallback(this);
    }
    if (mListRibbon) {
        mListRibbon->StopSlideSound();
    }
    if (mScrollSpeedIndicator) {
        mScrollSpeedIndicator->HandleExit();
    }
    mScrollBehavior.Exit();
    unk1f4 = gNullStr;
    unk1f8 = -1;
}

void HamNavList::Enter() {
    UIComponent::Enter();
    SkeletonUpdateHandle updateHandle = SkeletonUpdate::InstanceHandle();
    if (!updateHandle.HasCallback(this)) {
        updateHandle.AddCallback(this);
    }

    if (!mDisableSlideSound && mListRibbon) {
        mListRibbon->PlaySlideSound();
    }
    mFiltersUpdated = false;
    if (mSuppressAutomaticEnter) {
        mSuppressingEnter = true;
    } else {
        mPlayEnterAnim = true;
    }

    if (mListRibbon) {
        mListRibbon->HandleEnter();
    }
    if (mHeaderRibbon) {
        mHeaderRibbon->HandleEnter();
    }
    if (mScrollSpeedIndicator) {
        mScrollSpeedIndicator->HandleEnter();
    }
    unk1ec = TheTaskMgr.UISeconds();
    RealRefresh();

    static Symbol cheat_focus_restart("cheat_focus_restart");
    static Symbol pausecommand_restart("pausecommand_restart");

    if (mNavProvider && DataVariable(cheat_focus_restart).IntValue()) {
        int index = mNavProvider->DataIndex(pausecommand_restart);
        if (index != -1) {
            SetHighlight(index);
        }
    }
}

void HamNavList::Disengage() {
    mDirectionFilter->ClearSwipe();
    if ((!InControllerMode() || !CanHaveFocus())
        && mRibbonMode != HamListRibbon::kRibbonSelect) {
        SetRibbonMode(HamListRibbon::kRibbonDisengaged);
    }
}

void HamNavList::CompleteScroll(const UIListState &state) {
    if (mListDir) {
        mListDir->CompleteScroll(state, unk64);
    }
}

void HamNavList::ScrollToIndex(int i, int j) {
    if (GesturingWithVoice() && mListState.IsScrolling()) {
        mScrollBehavior.Exit();
    }
    mListState.SetSelected(i, j, true);
    unk1f0 = true;
    SetHighlight(i);
}

void HamNavList::PlayEnterAnim() {
    mSuppressingEnter = false;
    if (mListRibbon) {
        if (mListRibbon->EnterAnim()) {
            mListRibbon->SetTestEntering(true);
            if (mSkipEnterAnim) {
                mListRibbon->SetFrame(mListRibbon->EndFrame(), 1.0f);
                mListRibbon->SetTestEntering(false);
            }
        }
    }
    if (mHeaderRibbon) {
        if (mHeaderRibbon->EnterAnim()) {
            mHeaderRibbon->SetTestEntering(true);
            if (mSkipEnterAnim) {
                mHeaderRibbon->SetFrame(mHeaderRibbon->EndFrame(), 1.0f);
                mHeaderRibbon->SetTestEntering(false);
            }
        }
    }
    if ((mListRibbon && mListRibbon->TestEntering())
        || (mHeaderRibbon && mHeaderRibbon->TestEntering())) {
        Animate(0, false, 0);
    }
}

void HamNavList::Poll() {
    UIComponent::Poll();
    if (unk1f0) {
        RealRefresh();
    }

    if (mListDir) {
        mListDir->PollWidgets(unk64);
    }

    if (SkipPoll()) {
        if (mListRibbon) {
            mListRibbon->SetSlideSoundFrame(0);
        }
        mDirectionFilter->ClearSwipe();
        return;
    }

    if (TheGestureMgr && !TheLoadMgr.EditMode()) {
        if (TheGestureMgr->InDoubleUserMode() != unk1fd) {
            Update();
        }
        if (mAlwaysUseActiveSkeleton) {
            mSkeletonTrackingID = TheGestureMgr->ActiveSkeletonTrackingId();
        }

        const Skeleton *skel =
            TheGestureMgr->GetSkeletonByTrackingID(mSkeletonTrackingID);
        if (skel && skel->IsValid() && !skel->IsSideways() && !sForceDisengage) {
            UpdateGestures(skel);
            if (mScrollSpeedIndicator) {
                if ((mRibbonMode == HamListRibbon::kRibbonDisengaged
                     || !mListState.ScrollPastMinDisplay())
                    && mScrollSpeedIndicator->GetUnk1FC()) {
                    mScrollSpeedIndicator->Show(false);
                } else if (
                    mRibbonMode == HamListRibbon::kRibbonSwell
                    && !mScrollSpeedIndicator->GetUnk1FC()
                    && mListState.ScrollPastMinDisplay()
                ) {
                    mScrollSpeedIndicator->Show(true);
                } else {
                    mScrollSpeedIndicator->Update(
                        mHandHeightFilter->GetUnk10(),
                        HamScrollBehavior::mScrollUpCap,
                        HamScrollBehavior::mScrollDownCap
                    );
                }
            }
        } else if (!InVoiceMode()) {
            Disengage();
            if (mScrollSpeedIndicator
                && mScrollSpeedIndicator->GetUnk1FC()) {
                mScrollSpeedIndicator->Show(false);
            }
        }
    }

    if (mRibbonMode != HamListRibbon::kRibbonDisengaged) {
        RndOverlay::Find("swipe_direction")->SetCallback(mDirectionFilter);
    }

    if (mPlayEnterAnim) {
        mPlayEnterAnim = false;
        PlayEnterAnim();
    }

    if (mRibbonMode == HamListRibbon::kRibbonSwell) {
        if (!InControllerMode() && !InVoiceMode() && !TheLoadMgr.EditMode()) {
            DetermineHighlightedItem();
        }
    }

    if (GetRibbonMode() == HamListRibbon::kRibbonDisengaged || InControllerMode()
        || InVoiceMode()) {
        mScrollBehavior.SetUnk30(0);
    }
    if (GetRibbonMode() == HamListRibbon::kRibbonDisengaged) {
        if (InControllerMode()) {
            SetRibbonMode(HamListRibbon::kRibbonSwell);
        }
    }

    static bool sUnknownBool;

    if (mListRibbon && mListState.Provider()
        && mListRibbon->IsScrollable(mListState.NumShowing()) && !sUnknownBool) {
        mScrollBehavior.Update(mHandHeightFilter->GetUnk10());
    }

    if (mListRibbon) {
        if (mRibbonMode == HamListRibbon::kRibbonSlide
            && !mListRibbon->TestEntering()) {
            mListRibbon->SetSlideSoundFrame(mSlideAmount.Level());
        } else {
            mListRibbon->SetSlideSoundFrame(0);
        }
    }

    for (int i = 0; i < mRibbonDrawStates.size(); i++) {
        mRibbonDrawStates[i].unk0.Smooth(
            GetTargetSwellAmount(i), TheTaskMgr.DeltaUISeconds()
        );
    }

    if (mRibbonMode == HamListRibbon::kRibbonDisengaged) {
        mDisengageAmount.Smooth(1, TheTaskMgr.DeltaUISeconds());
    } else {
        mDisengageAmount.Smooth(0, TheTaskMgr.DeltaUISeconds());
    }

    if (mRibbonMode == HamListRibbon::kRibbonSelect && !IsAnimating()
        && !TheUI->InTransition() && !TheLoadMgr.EditMode()) {
        SetRibbonMode(HamListRibbon::kRibbonSwell);
        for (int i = 0; i < mRibbonDrawStates.size(); i++) {
            mRibbonDrawStates[i].unk0.SetParams(0, 0, 0);
        }

        if (unk1f8 != -1) {
            UIListProvider *provider = mListState.Provider();
            MILO_ASSERT(provider, 0x185);

            static Message navSelectDoneMsg("nav_select_done", 0, 0, 0, 0);
            navSelectDoneMsg[0] = unk1f4;
            navSelectDoneMsg[1] = unk1f8;
            navSelectDoneMsg[2] = this;
            navSelectDoneMsg[3] = unk1fc;

            TheUI->Handle(navSelectDoneMsg, false);
            TheHamProvider->Handle(navSelectDoneMsg, false);

            unk1f8 = -1;
        }

        if (mListRibbon) {
            mListRibbon->OnSelectDone();
        }

        if (mHeaderRibbon) {
            mHeaderRibbon->OnSelectDone();
        }
    }

    if (mListRibbon) {
        if (mSuppressingEnter) {
            mListRibbon->SetTestEntering(true);
            SetFrame(0, 1.0f);
        } else if (mListRibbon->TestEntering() && !IsAnimating()) {
            mListRibbon->SetTestEntering(false);
            SetFrame(0, 1.0f);
        }
    }

    if (mHeaderRibbon) {
        if (mSuppressingEnter) {
            mHeaderRibbon->SetTestEntering(true);
            SetFrame(0, 1.0f);
        } else if (mHeaderRibbon->TestEntering() && !IsAnimating()) {
            mHeaderRibbon->SetTestEntering(false);
            SetFrame(0, 1.0f);
        }
    }
}

void HamNavList::SetSelecting(bool b) {
    if (mEnableSelection) {
        sLastSelectInControllerMode = b;
        UIListProvider *provider = mListState.Provider();
        MILO_ASSERT(provider, 0x491);
        int selected = mListState.Selected();
        UIList *sublist = mListDir->SubList(selected, unk64);
        Symbol s;
        if (sublist) {
            HamNavProvider *navProvider = mNavProvider; // -_-
            int selectedPos = sublist->SelectedPos() + 1;
            int wrapShowing = sublist->GetListState().WrapShowing(selectedPos);
            s = navProvider->DataSymbol(selected, wrapShowing);
        } else {
            s = provider->DataSymbol(selected);
        }
        SetRibbonMode(HamListRibbon::kRibbonSelect);
        if (TheGestureMgr && TheGestureMgr->GesturingWithVoice()) {
            TheGestureMgr->SetGesturingWithVoice(false);
            if (mListState.IsScrolling()) {
                mScrollBehavior.Enter();
            }
        }
        UIComponent::SendSelect(nullptr);
        bool canSelect = provider->CanSelect(selected);
        unk1fc = canSelect;
        unk1f4 = s;
        unk1f8 = selected;

        NavSelectMsg msg(s, selected, this, canSelect);
        TheHamProvider->Handle(msg, false);
        DataNode handle = TheUI->Handle(msg, false);
        Export(msg, true);

        if (!mDisableSelectSound) {
            if (!ShouldSkipSelectSound(handle) && mListRibbon) {
                mListRibbon->PlaySelectSound(selected);
            }
        }

        if (mListRibbon) {
            mListRibbon->SetSlideSoundFrame(1);
            mListRibbon->SetUnk26C(ShouldSkipSelectAnim(handle));
        }

        if (mHeaderRibbon) {
            mHeaderRibbon->SetSlideSoundFrame(1);
            mHeaderRibbon->SetUnk26C(ShouldSkipSelectAnim(handle));
        }

        RndAnimatable::Animate(0, 0, 0);
    }
}

void HamNavListGlitchCB(float ms, void *refresh) {
    MILO_LOG(
        "HamNavList::Refresh %s took %f ms on frame %d\n",
        PathName(static_cast<HamNavList *>(refresh)),
        ms,
        TheRnd.GetFrameID()
    );
}

void HamNavList::RealRefresh() {
    AutoGlitchReport report(15.0f, HamNavListGlitchCB, this);
    unk1f0 = false;
    if (mListRibbon) {
        UIListProvider *provider = mListState.Provider();
        if (provider) {
            int numShowing = mListState.NumShowing();
            bool isScrollable = mListRibbon->IsScrollable(numShowing);
            if (isScrollable) {
                int maxDisplay = sListStateMaxDisplay - 4;
                int minDisplay = 3;
                if (numShowing <= maxDisplay) {
                    minDisplay = (numShowing - maxDisplay) + 2;
                }
                mListState.SetMinDisplay(minDisplay);
                mListState.SetScrollPastMinDisplay(true);
                mListState.SetMaxDisplay(maxDisplay);
                mListState.SetScrollPastMaxDisplay(maxDisplay < numShowing);
            } else {
                mListState.SetScrollPastMinDisplay(false);
                mListState.SetSelected(mListState.Selected(), -1, true);
            }
        }
    }

    if (mListDir) {
        mListState.Provider()->UnHighlightCurrent();
        mListState.Provider()->ClearIconLabels();
        mListDir->FillElements(mListState, unk64);
    }

    if (mNavInputType == kNavInput_RightHand) {
        UIListProvider *provider = mListState.Provider();
        if (provider) {
            static Message eqShiftEvenMsg("eq_shift_even");
            static Message eqShiftOddMsg("eq_shift_odd");
            int numShowing = mListState.NumShowing();
            if (numShowing % 2 == 0 || mListState.ScrollPastMinDisplay()) {
                TheHamProvider->Handle(eqShiftEvenMsg, false);
            } else {
                TheHamProvider->Handle(eqShiftOddMsg, false);
            }
        }
    }
}

int HamNavList::GetDisabledCount(int amount) const {
    int count = 0;
    for (int i = 0; i < amount; i++) {
        UIListProvider *provider = mListState.Provider();
        if (!provider->IsActive(i))
            count++;
    }
    for (int i = amount; i < mListState.NumShowing(); i++) {
        UIListProvider *provider = mListState.Provider();
        if (provider->IsActive(i))
            break;
        count++;
    }
    MILO_ASSERT(!IsScrollable() || count == 0, 0x313);
    return count;
}

float HamNavList::GetTargetSwellAmount(int i) {
    if (TheLoadMgr.EditMode()) {
        if (i == mListState.SelectedDisplay()) {
            if (mRibbonMode == HamListRibbon::RibbonMode::kRibbonSwell) {
                return GetFrame();
            }
            return 1.0f;
        }
    } else {
        if (!mListRibbon->TestEntering()) {
            if (!mScrollBehavior.IsScrolling()) {
                if (InControllerMode()) {
                    if (i == mListState.SelectedDisplay()
                        && TheUI->FocusComponent() == this) {
                        return 1.0f;
                    }
                } else if (mRibbonMode != 3) {
                    if (i == mListState.SelectedDisplay() && mScrollBehavior.GetUnk30() == 0) {
                        return 1.0f;
                    }
                    if (mRibbonMode == 0 && i < mListState.NumShowing()) {
                        UIListProvider *provider = mListState.Provider();
                        if (provider->IsActive(i) && mSkeletonTrackingID != -1) {
                            int disabledCount = GetDisabledCount(i);
                            disabledCount = i - disabledCount;
                            if (mListState.ScrollPastMinDisplay()) {
                                disabledCount -= mListState.MinDisplay();
                                if (!mScrollBehavior.AtTop()) {
                                    disabledCount++;
                                }
                            }
                            return CalculateSwell(disabledCount);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

bool HamNavList::IsElementBig(int element) const {
    int idx;
    if (mListRibbon->IsScrollable(mListState.NumShowing())) {
        idx = element + mListState.FirstShowing() - mListState.MinDisplay();
    } else {
        idx = element;
    }
    if (idx >= 0 && idx < mListState.NumShowing()) {
        for (int i = 0; i < mBigElements.size(); i++) {
            if (mListState.Provider()->DataSymbol(idx) == mBigElements[i]) {
                return true;
            }
        }
        for (int i = 0; i < unk20c.size(); i++) {
            if (idx == unk20c[i]) {
                return true;
            }
        }
    }
    return false;
}

void HamNavList::DrawDebug() const {
    static bool sBool;
    static float sFloat1 = 0.03f; // 82f0c790
    static float sFloat2 = 0.37f; // 82f0c794
    static float sFloat3 = 0.1f; // 82f0c798
    static float sFloat4 = 0.8f; // 82f0c79c
    static float sFloat5 = 0.1f; // 82f0c7a0
    static float sFloat6 = 0.25f; // 82f0c7a4
    if (sBool) {
        float handVal = mHandHeightFilter->GetUnk10();
        UtilDrawLine(
            Vector2(0, handVal), Vector2(1, handVal), Hmx::Color(0.0f, 1.0f, 0.0f, 1.0f)
        );
        static Hmx::Color color2(0.2f, 0.2f, 0.2f, 0.7f);
        static Hmx::Color color3(1.0f, 1.0f, 1.0f, 1.0f);

        TheRnd.DrawRectScreen(
            Hmx::Rect(sFloat3, sFloat5 - 0.05f, sFloat4, sFloat6 + 0.05f),
            color2,
            nullptr,
            nullptr,
            nullptr
        );
        for (int i = 0; i < 5; i++) {
            char buf[50];
            sprintf_s<50>(buf, "");
            switch (i) {
            case 0:
                sprintf_s<50>(buf, "Hand height %f", mHandHeight);
                break;
            case 1:
                sprintf_s<50>(
                    buf, "ListState SelectedDisplay: %d", mListState.SelectedDisplay()
                );
                break;
            case 2:
                sprintf_s<50>(
                    buf, "ListState FirstShowing: %d", mListState.FirstShowing()
                );
                break;
            case 3:
                sprintf_s<50>(buf, "ListState Selected: %d", mListState.Selected());
                break;
            case 4:
                sprintf_s<50>(buf, "Num selectable items: %d", NumItems());
                break;
            }
            Vector2 vec2C(sFloat2 * 0 + sFloat3, i * sFloat1 + sFloat5);
            TheRnd.DrawStringScreen(buf, vec2C, color3, true);
        }
    }
}

void HamNavList::LinkRibbonDrawState(
    std::vector<HamListRibbonDrawState> &ribbonDrawStates,
    UIListWidgetDrawState &widgetDrawStates
) {
    int numUIElements = widgetDrawStates.mElements.size();
    if (numUIElements != ribbonDrawStates.size()) {
        ribbonDrawStates.resize(numUIElements);
    }
    for (int i = 0; i < numUIElements; i++) {
        ribbonDrawStates[i].unk14 = i == mListState.SelectedDisplay();
        if (mListRibbon->IsScrollable(mListState.NumShowing())) {
            ribbonDrawStates[i].unk24 = mListState.Provider()->IsHeader(
                mListState.FirstShowing() + i - mListState.MinDisplay()
            );
        } else {
            ribbonDrawStates[i].unk24 = mListState.Provider()->IsHeader(i);
        }
        ribbonDrawStates[i].unk20 = IsElementBig(i);
        ribbonDrawStates[i].unk18 = &widgetDrawStates.mElements[i];
        ribbonDrawStates[i].unk18->mComponentState = mState;
        if (ribbonDrawStates[i].unk18->mElementState == kUIListWidgetHighlight) {
            if (!mListRibbon->TestEntering()
                && mRibbonMode != HamListRibbon::kRibbonDisengaged) {
                if (TheUI->FocusComponent() == this || !InControllerMode()) {
                    continue;
                }
            }
            ribbonDrawStates[i].unk18->mElementState = kUIListWidgetActive;
        }
    }
}

float HamNavList::CalculateSwell(int i1) const {
    static float sFloat = 0.8f;
    int numItems = NumItems();
    float f2 = Clamp(0.0f, 1.0f, mHandHeight);
    f2 = sqrtf(fabsf(f2 - (float)i1 / (float)(numItems - 1))) * sqrtf((float)numItems)
        * sFloat;
    return 1 - Clamp(0.0f, 1.0f, f2);
}

void HamNavList::DetermineHighlightedItem() {
    MILO_ASSERT(!InControllerMode(), 0x2B7);
    MILO_ASSERT(!TheLoadMgr.EditMode(), 0x2B8);
    static float sFloat = 0.1f;

    int i2 = NumItems();
    int i7 = i2 - 1;
    float f8 = -(i7 * sFloat - 1);
    f8 = f8 / (float)i2;
    i2 = GetHighlightItem();
    if (mListState.ScrollPastMinDisplay() && !mScrollBehavior.AtTop()
        && (i2 != 0 || mScrollBehavior.GetUnk30() != 1)) {
        if (i2 == HamListRibbon::sNumListSelectable - 1 && mScrollBehavior.GetUnk30() == 2) {
            i2 = HamListRibbon::sNumListSelectable + 1;
        } else {
            i2++;
        }
    }

    int i6 = i7 * mHandHeight + 0.5f;
    float f10 = (float)i2 / i7;

    if (i6 <= i7) {
        i7 = Max(i6, 0);
    }

    if (mListState.ScrollPastMinDisplay()) {
        if (!mScrollBehavior.AtTop()) {
            i7 = i7 - 1;
        }
        if (i7 == -1) {
            mScrollBehavior.SetUnk30(1);
        } else if (i7 == HamListRibbon::sNumListSelectable) {
            mScrollBehavior.SetUnk30(2);
        } else {
            mScrollBehavior.SetUnk30(0);
        }
        i7 = Clamp(0, HamListRibbon::sNumListSelectable - 1, i7);
    }

    if (fabsf(mHandHeight - f10) < f8 / 2 + sFloat) {
        if (mScrollBehavior.AtBottom()) {
            mScrollBehavior.SetUnk30(0);
        }
    } else {
        int i3 = GetDisabledCount(i7) + mListState.FirstShowing() + i7;
        if (i3 != mListState.Selected()) {
            SetHighlight(i3);
        }
    }
}

void HamNavList::UpdateGestures(const Skeleton *skeleton) {
    if (unk1f0) {
        RealRefresh();
    }
    if (!InControllerMode() && !GesturingWithVoice() && mEnabled) {
        if (mRibbonMode == HamListRibbon::kRibbonSelect) {
            if (IsAnimating()) {
                return;
            }
            if (GetFrame() == EndFrame()) {
                return;
            }
        }
        if (InVoiceMode()) {
            TheGestureMgr->SetInVoiceMode(false);
        }

        if (mRibbonMode == HamListRibbon::kRibbonDisengaged) {
            mDirectionFilter->Clear();
            mDirectionFilter->SetEngaged(false);
        } else {
            mDirectionFilter->SetEngaged(true);
        }
        if (mScrollBehavior.IsScrolling()) {
            mDirectionFilter->Clear();
            mDirectionFilter->ResetHoverTimer();
        }
        if (NumItems() == 1) {
            mDirectionFilter->ResetHoverTimer();
            mDirectionFilter->SetAllowAboveShoulder(false);
        } else {
            mDirectionFilter->SetAllowAboveShoulder(true);
        }
        int first = mListState.FirstShowing();
        bool scroll = mListState.ScrollPastMinDisplay();
        if (mListState.Selected() + (scroll != false) - first == NumItems() - 1) {
            mDirectionFilter->ResetHoverTimer();
        }

        bool b6 = skeleton && skeleton->IsValid() && mDirectionFilter->IsHandValid(*skeleton);
        bool b8 =
            mListState.ScrollPastMinDisplay() && mDirectionFilter->IsValidScrollPos(*skeleton);
        if (!b6 && !b8 && mFiltersUpdated && !TheLoadMgr.EditMode()) {
            Disengage();
        }

        if (b8 && mRibbonMode == HamListRibbon::kRibbonDisengaged) {
            SetSwelling();
        }

        if (mListRibbon->TestEntering()) {
            return;
        }

        mHandHeight = mHandHeightFilter->GetUnk10();
        if (mDirectionFilter->HasDirection() && mEnableSelection) {
            mDirectionFilter->ClearSwipe();
            static Message forceLetterboxOff("force_letterbox_off");
            TheUI->Handle(forceLetterboxOff, false);
            SetSelecting(false);
            return;
        }

        if (b6) {
            bool cmp =
                !mDirectionFilter->IsLockedIn() && !mDirectionFilter->HasDirection() || mScrollBehavior.GetUnk30();
            if (cmp) {
                SetSwelling();
                return;
            }

            if (!mEnableSelection) {
                return;
            }
            SetSliding(mDirectionFilter->GetPercentPulled());
            return;
        }
        mDirectionFilter->ClearSwipe();

    } else {
        mDirectionFilter->ClearSwipe();
    }
}

DataNode HamNavList::OnMsg(const ButtonDownMsg &msg) {
    if (unk1f0) {
        RealRefresh();
    }

    if ((InControllerMode() || TheLoadMgr.EditMode()) && !IsAnimating() && mEnabled) {
        if (!GesturingWithVoice() && TheUI->FocusComponent() == this) {
            int direction = ScrollDirection(msg, false, true, 1);
            if (direction != 0) {
                int selected = mListState.Selected();
                do { // actual do while loop??
                    selected += direction;
                    if (selected < 0 || selected >= mListState.NumShowing()) {
                        return 0;
                    }
                } while (!mListState.Provider()->IsActive(selected));

                if (mListState.ScrollPastMinDisplay()) {
                    int firstShowing = mListState.FirstShowing();
                    if (selected < firstShowing) {
                        mScrollBehavior.ScrollUp(false);
                    } else if (selected >= HamListRibbon::sNumListSelectable + firstShowing) {
                        mScrollBehavior.ScrollDown(false);
                    } else {
                        SetHighlight(selected);
                    }
                } else {
                    SetHighlight(selected);
                }
                return 0;
            }

            if (msg.GetAction() == kAction_Confirm) {
                if (mEnableSelection) {
                    SetSelecting(true);
                }
                return 0;
            }
        }
    }
    return DATA_UNHANDLED;
}
