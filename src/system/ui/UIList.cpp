#include "ui/UIList.h"
#include "math/Geo.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "os/User.h"
#include "rndobj/Draw.h"
#include "rndobj/FontBase.h"
#include "ui/UIComponent.h"
#include "ui/UIListArrow.h"
#include "ui/UIListCustom.h"
#include "ui/UIListDir.h"
#include "ui/UIListHighlight.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "ui/UIListProvider.h"
#include "ui/UIListSlot.h"
#include "ui/UIListState.h"
#include "ui/UIListSubList.h"
#include "ui/UIListWidget.h"
#include "ui/UITransitionHandler.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

static bool gLoading = false;

UIList::UIList()
    : UITransitionHandler(this), mListDir(this), mListState(this, this), mDataProvider(0),
      mNumData(100), mPaginate(0), mUser(0), mParent(0), mExtendedLabelEntries(this),
      mExtendedMeshEntries(this), mExtendedCustomEntries(this), mAutoScrollPause(2),
      mAutoScrollSendMsgs(0), unk150(1), mAutoScrolling(0), unk158(-1), unk15c(0),
      unk15d(0), unk160(1), mAllowHighlight(1) {}

UIList::~UIList() {
    DeleteAll(mWidgets);
    RELEASE(mDataProvider);
}

BEGIN_HANDLERS(UIList)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE(selected_sym, OnSelectedSym)
    HANDLE_EXPR(selected_pos, SelectedPos())
    HANDLE_EXPR(selected_data, SelectedData())
    HANDLE_EXPR(num_display, NumDisplay())
    HANDLE_EXPR(first_showing, FirstShowing())
    HANDLE_ACTION(set_provider, SetProvider(_msg->Obj<UIListProvider>(2)))
    HANDLE(set_data, OnSetData)
    HANDLE_EXPR(num_data, NumProviderData())
    HANDLE_ACTION(disable_data, DisableData(_msg->Sym(2)))
    HANDLE_ACTION(enable_data, EnableData(_msg->Sym(2)))
    HANDLE_ACTION(dim_data, DimData(_msg->Sym(2)))
    HANDLE_ACTION(undim_data, UnDimData(_msg->Sym(2)))
    HANDLE(set_selected, OnSetSelected)
    HANDLE(set_selected_simulate_scroll, OnSetSelectedSimulateScroll)
    HANDLE_ACTION(set_scroll_user, mUser = _msg->Obj<LocalUser>(2))
    HANDLE_ACTION(refresh, Refresh(true))
    HANDLE_ACTION(set_draw_manually_controlled_widgets, unk15d = _msg->Int(2))
    HANDLE(scroll, OnScroll)
    HANDLE_EXPR(is_scrolling, IsScrolling())
    HANDLE_EXPR(is_scrolling_down, mListState.CurrentScroll() > 0)
    HANDLE_ACTION(store, Store())
    HANDLE_ACTION(undo, RevertScrollSelect(this, _msg->Obj<LocalUser>(2), nullptr))
    HANDLE_ACTION(confirm, Reset())
    HANDLE_ACTION(set_num_display, SetNumDisplay(_msg->Int(2)))
    HANDLE_ACTION(set_grid_span, SetGridSpan(_msg->Int(2)))
    HANDLE_ACTION(auto_scroll, AutoScroll())
    HANDLE_ACTION(stop_auto_scroll, mAutoScrolling = false)
    HANDLE_EXPR(parent_list, mParent)
    HANDLE_ACTION(allow_highlight, mAllowHighlight = _msg->Int(2))
    HANDLE_SUPERCLASS(ScrollSelect)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(UIList)
    SYNC_PROP_MODIFY(list_resource, mListDir, Update())
    SYNC_PROP_SET(display_num, NumDisplay(), SetNumDisplay(_val.Int()))
    SYNC_PROP_SET(grid_span, GridSpan(), SetGridSpan(_val.Int()))
    SYNC_PROP_SET(circular, Circular(), SetCircular(_val.Int()))
    SYNC_PROP_SET(scroll_time, Speed(), SetSpeed(_val.Float()))
    SYNC_PROP(paginate, mPaginate)
    SYNC_PROP_SET(
        min_display, mListState.MinDisplay(), mListState.SetMinDisplay(_val.Int())
    )
    SYNC_PROP_SET(
        scroll_past_min_display,
        mListState.ScrollPastMinDisplay(),
        mListState.SetScrollPastMinDisplay(_val.Int())
    )
    SYNC_PROP_SET(
        scroll_past_min_display,
        mListState.ScrollPastMinDisplay(),
        mListState.SetScrollPastMinDisplay(_val.Int())
    )
    SYNC_PROP_SET(
        max_display, mListState.MaxDisplay(), mListState.SetMaxDisplay(_val.Int())
    )
    SYNC_PROP_SET(
        scroll_past_max_display,
        mListState.ScrollPastMaxDisplay(),
        mListState.SetScrollPastMaxDisplay(_val.Int())
    )
    SYNC_PROP_MODIFY(num_data, mNumData, Update())
    SYNC_PROP(auto_scroll_pause, mAutoScrollPause)
    SYNC_PROP(auto_scroll_send_messages, mAutoScrollSendMsgs)
    SYNC_PROP(extended_label_entries, mExtendedLabelEntries)
    SYNC_PROP(extended_mesh_entries, mExtendedMeshEntries)
    SYNC_PROP(extended_custom_entries, mExtendedCustomEntries)
    SYNC_PROP_SET(in_anim, GetInAnim(), SetInAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_SET(out_anim, GetOutAnim(), SetOutAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_SET(
        limit_circular_display_num_to_data_num,
        mLimitCircularDisplayNumToDataNum,
        LimitCircularDisplay(_val.Int())
    )
    SYNC_SUPERCLASS(ScrollSelect)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(UIList)
    SAVE_REVS(0x15, 0)
    SAVE_SUPERCLASS(UIComponent)
    bs << mListDir;
    bs << NumDisplay();
    bs << GridSpan();
    bs << Circular();
    bs << Speed();
    bs << mListState.ScrollPastMinDisplay();
    bs << mListState.ScrollPastMaxDisplay();
    bs << mPaginate;
    bs << mSelectToScroll;
    bs << mListState.MinDisplay();
    bs << mListState.MaxDisplay();
    bs << mNumData;
    bs << mAutoScrollPause;
    bs << mAutoScrollSendMsgs;
    bs << mExtendedLabelEntries;
    bs << mExtendedMeshEntries;
    bs << mExtendedCustomEntries;
    SaveHandlerData(bs);
    bs << mLimitCircularDisplayNumToDataNum;
END_SAVES

BEGIN_LOADS(UIList)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

BEGIN_COPYS(UIList)

END_COPYS

UIListDir *UIList::GetUIListDir() const { return mListDir; }

int UIList::SelectedPos() const { return mListState.Selected(); }

bool UIList::IsScrolling() const { return mListState.IsScrolling(); }

void UIList::SetSpeed(float speed) { mListState.SetSpeed(speed); }

float UIList::Speed() const { return mListState.Speed(); }

void UIList::SetParent(UIList *uilist) { mParent = uilist; }

void UIList::CalcBoundingBox(Box &box) {}

Symbol UIList::SelectedSym(bool fail) const {
    Symbol sym = mListState.Provider()->DataSymbol(mListState.SelectedData());
    if (fail) {
        if (sym == gNullStr)
            MILO_FAIL("DataSymbol() not implemented in UIList provider");
    }
    return sym;
}

void UIList::Scroll(int i) {
    unk15c = true;
    mListState.Scroll(i, false);
}

void UIList::StopAutoScroll() { mAutoScrolling = false; }

int UIList::NumProviderData() const {
    UIListProvider *p = mListState.Provider();
    if (p)
        return p->NumData();
    else
        return NumData();
}

int UIList::SelectedAux() const { return mListState.Selected(); }

bool UIList::IsEmptyValue() const { return SelectedData() == -1; }

void UIList::AutoScroll() {
    UIListProvider *prov = mListState.Provider();
    if (!prov)
        prov = this;
    if (prov->NumData() <= NumDisplay()) {
        StopAutoScroll();
    } else {
        mAutoScrolling = true;
        unk150 = 1;
        unk158 = mAutoScrollPause + TheTaskMgr.UISeconds();
    }
}

void UIList::Enter() {
    UIComponent::Enter();
    Reset();
    mListDir->ListEntered();
}

void UIList::Poll() {
    UIComponent::Poll();
    if (mAutoScrolling) {
        if (unk158 >= 0.0f && TheTaskMgr.UISeconds() >= unk158) {
            Scroll(unk150);
            unk158 = -1.0f;
        }
    }
    mListState.Poll(TheTaskMgr.UISeconds());
    mListDir->PollWidgets(mWidgets);
    unk15c = false;
    UpdateHandler();
}

int UIList::CollidePlane(std::vector<Vector3> const &vec, Plane const &p) { return 0; }

void UIList::StartScroll(UIListState const &, int, bool) {}

void UIList::HandleSelectionUpdated() { UITransitionHandler::StartValueChange(); }

void UIList::UpdateExtendedEntries(UIListState const &) {}

DataNode UIList::OnScroll(DataArray *) { return NULL_OBJ; }

DataNode UIList::OnSelectedSym(DataArray *) { return NULL_OBJ; }

void UIList::FinishValueChange() {}

void UIList::PreLoadWithRev(BinStreamRev &) {}

void UIList::SetSelected(int i, int j) {
    mListDir->CompleteScroll(mListState, mWidgets);
    mListState.SetSelected(i, j, true);
    Refresh(false);
    mListDir->Poll();
}

bool UIList::SetSelected(Symbol sym, bool b, int i) {
    int index = mListState.Provider()->DataIndex(sym);
    if (index == -1) {
        if (b) {
            MILO_NOTIFY("Couldn't find %s in UIList provider", sym);
        }
        return false;
    } else {
        SetSelected(index, i);
        return true;
    }
}

void UIList::Refresh(bool b) {
    mListDir->FillElements(mListState, mWidgets);
    if (b) {
        int nowrap = mListState.SelectedNoWrap();
        if (nowrap >= NumProviderData() && nowrap != 0)
            SetSelected(NumProviderData() - 1, -1);
        else {
            if (!mListState.Provider()->IsActive(mListState.SelectedData())
                && !mListState.IsScrolling()) {
                SetSelected(nowrap, -1);
            }
        }
    }
}

void UIList::EnableData(Symbol s) {
    MILO_ASSERT(mDataProvider, 0x382);
    mDataProvider->Enable(s);
    Refresh(false);
}

void UIList::DisableData(Symbol s) {
    MILO_ASSERT(mDataProvider, 0x389);
    mDataProvider->Disable(s);
    Refresh(false);
    if (!mDataProvider->IsActive(SelectedData())) {
        mListState.SetSelected(0, -1, true);
    }
}

void UIList::DimData(Symbol s) {
    MILO_ASSERT(mDataProvider, 0x396);
    mDataProvider->Dim(s);
    Refresh(false);
}

void UIList::UnDimData(Symbol s) {
    MILO_ASSERT(mDataProvider, 0x39d);
    mDataProvider->UnDim(s);
    Refresh(false);
}

void UIList::SetSelectedAux(int i) { SetSelected(i, -1); }

void UIList::CompleteScroll(UIListState const &) {}

DataNode UIList::OnSetSelected(DataArray *) { return NULL_OBJ; }

void UIList::PreLoad(BinStream &) {}

void UIList::PostLoad(BinStream &) {}

void UIList::SetSelectedSimulateScroll(int) {}

bool UIList::SetSelectedSimulateScroll(Symbol, bool) { return false; }

void UIList::Update() {
    if (!gLoading) {
        MILO_ASSERT(mListDir, 0x238);
        mListDir->CreateElements(this, mWidgets, mListState.NumDisplay());

        if (TheLoadMgr.EditMode())
            Refresh(false);
    }
}

DataNode UIList::OnMsg(const ButtonDownMsg &msg) { return NULL_OBJ; }

DataNode UIList::OnSetSelectedSimulateScroll(DataArray *) { return NULL_OBJ; }

void UIList::OldResourcePreload(BinStream &bs) {}

void UIList::SetNumDisplay(int i) {
    mListState.SetNumDisplay(i, gLoading == 0);
    Update();
}

void UIList::SetGridSpan(int i) {
    mListState.SetGridSpan(i, gLoading == 0);
    Update();
}

void UIList::SetCircular(bool b) {
    mListState.SetCircular(b, gLoading == 0);
    Update();
    if (!gLoading)
        Refresh(false);
}

void UIList::LimitCircularDisplay(bool b) {
    if (&mListState) {
        int val;
        mLimitCircularDisplayNumToDataNum = b;
        if (b) {
            int numprov = NumProviderData();
            int i = unk160;
            if (numprov < i)
                val = numprov;
            else
                val = 1;

        } else {
            val = unk160;
        }
        SetNumDisplay(val);
        Refresh(false);
    }
}

void UIList::SetProvider(UIListProvider *prov) {}

DataNode UIList::OnSetData(DataArray *) { return NULL_OBJ; }

void UIList::DrawShowing() {}

float UIList::GetDistanceToPlane(const Plane &p, Vector3 &v) {
    float ret = 0;
    bool first = true;
    Box box;
    CalcBoundingBox(box);
    Vector3 boxVecs[8] = { Vector3(box.mMin.x, box.mMin.y, box.mMin.z),
                           Vector3(box.mMax.x, box.mMin.y, box.mMin.z),
                           Vector3(box.mMax.x, box.mMax.y, box.mMin.z),
                           Vector3(box.mMin.x, box.mMax.y, box.mMin.z),
                           Vector3(box.mMin.x, box.mMin.y, box.mMax.z),
                           Vector3(box.mMax.x, box.mMin.y, box.mMax.z),
                           Vector3(box.mMax.x, box.mMax.y, box.mMax.z),
                           Vector3(box.mMin.x, box.mMax.y, box.mMax.z) };
    for (int i = 0; i < 8; i++) {
        float dot = p.Dot(boxVecs[i]);
        if (first || (std::fabs(dot) < std::fabs(ret))) {
            ret = dot;
            v = boxVecs[i];
            first = false;
        }
    }
    return ret;
}

void UIList::Init() {
    Register();
    REGISTER_OBJ_FACTORY(UIListArrow)
    REGISTER_OBJ_FACTORY(UIListCustom)
    REGISTER_OBJ_FACTORY(UIListDir)
    REGISTER_OBJ_FACTORY(UIListHighlight)
    REGISTER_OBJ_FACTORY(UIListLabel)
    REGISTER_OBJ_FACTORY(UIListMesh)
    REGISTER_OBJ_FACTORY(UIListSlot)
    REGISTER_OBJ_FACTORY(UIListSubList)
    REGISTER_OBJ_FACTORY(UIListWidget)
}

void UIList::BoundingBoxTriangles(std::vector<std::vector<Vector3> > &) {}

RndDrawable *UIList::CollideShowing(const Segment &, float &, Plane &) { return nullptr; }

int UIList::CollidePlane(const Plane &) { return 1; }
