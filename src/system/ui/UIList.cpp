#include "ui/UIList.h"
#include "math/Geo.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "os/User.h"
#include "rndobj/Draw.h"
#include "rndobj/FontBase.h"
#include "ui/UI.h"
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
#include "ui/Utl.h"
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

BEGIN_COPYS(UIList)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(UIList)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mListDir)
        mListState.SetCircular(c->Circular(), true);
        mListState.SetNumDisplay(c->NumDisplay(), true);
        mListState.SetGridSpan(c->GridSpan(), true);
        mListState.SetSpeed(c->Speed());
        COPY_MEMBER(mPaginate)
        COPY_MEMBER(mSelectToScroll)
        mListState.SetMinDisplay(c->mListState.MinDisplay());
        mListState.SetScrollPastMinDisplay(c->mListState.ScrollPastMinDisplay());
        mListState.SetMaxDisplay(c->mListState.MaxDisplay());
        mListState.SetScrollPastMaxDisplay(c->mListState.ScrollPastMaxDisplay());
        COPY_MEMBER(mNumData)
        COPY_MEMBER(mAutoScrollPause)
        COPY_MEMBER(mAutoScrollSendMsgs)
        COPY_MEMBER(mExtendedLabelEntries)
        COPY_MEMBER(mExtendedMeshEntries)
        COPY_MEMBER(mExtendedCustomEntries)
        COPY_MEMBER(mLimitCircularDisplayNumToDataNum)
        COPY_MEMBER(unk160)
    END_COPYING_MEMBERS
    const UIList *l = dynamic_cast<const UIList *>(o);
    if (l) {
        CopyHandlerData(l);
    }
    Update();
END_COPYS

BEGIN_LOADS(UIList)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(0x15, 0)

void UIList::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0x15, 0)
    PreLoadWithRev(d);
}

void UIList::PreLoadWithRev(BinStreamRev &d) {
    if (d.rev > 0x15) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d",
            PathName(this),
            ClassName(),
            d.rev,
            gRev
        );
    }
    UIComponent::PreLoad(d.stream);
    if (d.rev >= 0x14) {
        d >> mListDir;
    }
    d.PushRev(this);
}

void UIList::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    UIComponent::PostLoad(d.stream);
    mListDir.PostLoad(nullptr);
    bool local_scrollpastmin = false; // 0x90
    bool local_scrollpastmax = true; // 0x8f
    bool b8e; // 0x8e
    int local_gridspan = 1; // 0x8c
    int i88; // 0x88
    int local_mindisplay = 0; // 0x84
    int local_maxdisplay = -1; // 0x80
    float i7c; // 0x7c
    if (d.rev < 0xF) {
        int x; // 0x78
        int y; // 0x74
        int z; // 0x70
        d >> x >> y;
        if (d.rev > 4) {
            if (d.rev > 6) {
                d >> z;
            } else {
                d >> b8e;
            }
        }
        if (d.rev > 6) {
            d >> b8e;
        }
        if (d.rev > 8) {
            d >> b8e;
        }
        int i6c; // 0x6c
        if (d.rev > 10) {
            d >> i6c;
        }
        int i68; // 0x68
        d >> i68;
    }
    d >> i88;
    if (d.rev > 0x11) {
        d >> local_gridspan;
    }
    d >> b8e;
    d >> i7c;
    if (d.rev > 0xC) {
        d >> local_scrollpastmin;
    }
    if (d.rev > 7) {
        d >> local_scrollpastmax;
    }
    if (d.rev > 2) {
        d >> mPaginate;
    }
    if (d.rev > 3) {
        d >> mSelectToScroll;
    }
    if (d.rev >= 10) {
        d >> local_mindisplay;
    }
    if (d.rev >= 6) {
        d >> local_maxdisplay;
    }
    gLoading = true;
    mListState.SetNumDisplay(i88, false);
    unk160 = i88;
    mListState.SetGridSpan(local_gridspan, false);
    mListState.SetCircular(b8e, false);
    mListState.SetSpeed(i7c);
    mListState.SetScrollPastMinDisplay(local_scrollpastmin);
    mListState.SetScrollPastMaxDisplay(local_scrollpastmax);
    mListState.SetMinDisplay(local_mindisplay);
    mListState.SetMaxDisplay(local_maxdisplay);
    if (d.rev == 1) {
        int i64, i60;
        d >> i64 >> i60;
    }
    if (d.rev >= 0xC) {
        d >> mNumData;
    }
    if (d.rev >= 0xE) {
        d >> mAutoScrollPause;
    }
    if (d.rev < 0x13) {
        mAutoScrollSendMsgs = true;
    } else {
        d >> mAutoScrollSendMsgs;
    }
    if (d.rev >= 0x10) {
        d >> mExtendedLabelEntries;
        d >> mExtendedMeshEntries;
        d >> mExtendedCustomEntries;
    }
    if (d.rev >= 0x11) {
        LoadHandlerData(d.stream);
    }
    if (d.rev >= 0x15) {
        d >> mLimitCircularDisplayNumToDataNum;
    } else {
        mLimitCircularDisplayNumToDataNum = false;
    }
    gLoading = false;
    Update();
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

void UIList::DrawShowing() {
    if (unk15c) {
        mListState.Poll(TheTaskMgr.UISeconds());
        unk15c = false;
    }
    bool u7 = unk15d;
    if (mParent) {
        if (mParent->mListDir->SubList(
                mParent->mListState.SelectedDisplay(), mParent->mWidgets
            )
            == this) {
            u7 = mParent->unk15d;
        }
    }
    float f9;

    UIList *sublist = ChildList();
    if (sublist) {
        UIListDir *dir = sublist->mListDir;
        f9 = dir->ElementSpacing() * sublist->mListState.SelectedDisplay();
    } else {
        f9 = 0;
    }
    UIListWidgetDrawState drawState;
    mListDir->BuildDrawState(drawState, mListState, DrawState(this), f9, mAllowHighlight);
    mListDir->DrawWidgets(
        drawState, mListState, mWidgets, WorldXfm(), DrawState(this), nullptr, u7
    );
}

RndDrawable *UIList::CollideShowing(const Segment &s, float &fl, Plane &pl) {
    RndDrawable *ret = nullptr;
    std::vector<std::vector<Vector3> > vectors;
    BoundingBoxTriangles(vectors);
    Segment segment = s;
    bool b1 = false;
    fl = 1;
    FOREACH (it, vectors) {
        auto &curVector = *it;
        Triangle triangle;
        triangle.Set(curVector[0], curVector[1], curVector[2]);
        float fd0;
        if (Intersect(segment, triangle, false, fd0)) {
            b1 = true;
            Interp(segment.start, segment.end, fd0, segment.end);
            fl *= fd0;
            pl.a = triangle.frame.z.x;
            pl.b = triangle.frame.z.y;
            pl.c = triangle.frame.z.z;
            pl.d = -pl.Dot(triangle.origin);
        }
    }
    if (b1) {
        ret = this;
    }
    return ret;
}

int UIList::CollidePlane(const Plane &pl) {
    std::vector<std::vector<Vector3> > vectors;
    BoundingBoxTriangles(vectors);
    int collided = CollidePlane(vectors[0], pl);
    if (collided == 0) {
        return 0;
    } else {
        FOREACH (it, vectors) {
            if (collided != CollidePlane(*it, pl)) {
                return 0;
            }
        }
    }
    return collided;
}

void UIList::StartScroll(const UIListState &state, int i2, bool b3) {
    mListDir->StartScroll(state, mWidgets, i2, b3);
    if (state.Provider()->IsActive(state.SelectedData())
        && (!mAutoScrolling || mAutoScrollSendMsgs)) {
        TheUI->Handle(UIComponentScrollStartMsg(this, mUser), false);
    }
}

void UIList::CompleteScroll(const UIListState &state) {
    mListDir->CompleteScroll(state, mWidgets);
    if (mAutoScrolling) {
        int i4 = mNumData;
        state.Provider();
        int i2 = unk150 > 0 ? mListState.MaxFirstShowing() : 0;
        if (i4 == i2) {
            unk150 = -unk150;
            unk158 = TheTaskMgr.UISeconds() + mAutoScrollPause;
        } else {
            unk15c = true;
            mListState.Scroll(unk150, false);
        }
    }
    if (mListState.Provider()->IsActive(mListState.SelectedData())) {
        if (!mAutoScrolling || mAutoScrollSendMsgs) {
            TheUI->Handle(UIComponentScrollMsg(this, mUser), false);
        }
        HandleSelectionUpdated();
    }
}

void UIList::OldResourcePreload(BinStream &bs) {
    char buf[256];
    bs.ReadString(buf, 256);
    mListDir.SetName(buf, true);
}

int UIList::SelectedAux() const { return mListState.Selected(); }
void UIList::SetSelectedAux(int i) { SetSelected(i, -1); }
void UIList::FinishValueChange() {
    UpdateExtendedEntries(mListState);
    UITransitionHandler::FinishValueChange();
}
bool UIList::IsEmptyValue() const { return SelectedData() == -1; }

UIListDir *UIList::GetUIListDir() const { return mListDir; }

int UIList::Selected() const { return mListState.Selected(); }
int UIList::SelectedPos() const { return mListState.Selected(); }
UIListState &UIList::GetListState() { return mListState; }

int UIList::FirstShowing() const { return mListState.FirstShowing(); }

bool UIList::IsScrolling() const { return mListState.IsScrolling(); }

void UIList::SetSpeed(float speed) { mListState.SetSpeed(speed); }

float UIList::Speed() const { return mListState.Speed(); }

void UIList::SetParent(UIList *uilist) { mParent = uilist; }

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

// int UIList::CollidePlane(std::vector<Vector3> const &vec, Plane const &p) { return 0; }

UIList *UIList::ChildList() {
    return mListDir->SubList(mListState.SelectedDisplay(), mWidgets);
}

void UIList::HandleSelectionUpdated() {
    UITransitionHandler::StartValueChange();
    if (ChildList()) {
        ChildList()->HandleSelectionUpdated();
    }
}

void UIList::UpdateExtendedEntries(const UIListState &state) {
    UIListProvider *prov = state.Provider();
    if (prov && prov->NumData() > 0) {
        UIList *pMainList = mParent ? mParent : this;
        MILO_ASSERT(pMainList, 0x3EF);
        FOREACH (it, pMainList->mExtendedLabelEntries) {
            UILabel *label = *it;
            MILO_ASSERT(label, 0x3F6);
            prov->UpdateExtendedText(state.SelectedDisplay(), state.SelectedData(), label);
        }
        FOREACH (it, pMainList->mExtendedMeshEntries) {
            RndMesh *mesh = *it;
            MILO_ASSERT(mesh, 0x401);
            prov->UpdateExtendedMesh(state.SelectedDisplay(), state.SelectedData(), mesh);
        }
        FOREACH (it, pMainList->mExtendedCustomEntries) {
            Hmx::Object *custom = *it;
            MILO_ASSERT(custom, 0x40C);
            prov->UpdateExtendedCustom(
                state.SelectedDisplay(), state.SelectedData(), custom
            );
        }
    }
}

void UIList::SetSelected(int i, int j) {
    mListDir->CompleteScroll(mListState, mWidgets);
    mListState.SetSelected(i, j, true);
    Refresh(false);
    mListDir->Poll();
    UIList *sublist = ChildList();
    if (sublist) {
        sublist->Poll();
    }
    HandleSelectionUpdated();
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

void UIList::SetSelectedSimulateScroll(int i1) {
    mListDir->CompleteScroll(mListState, mWidgets);
    mListState.SetSelectedSimulateScroll(i1);
    Refresh(false);
    mListDir->Poll();
    if (ChildList()) {
        Poll();
    }
}

bool UIList::SetSelectedSimulateScroll(Symbol s, bool warn) {
    int scroll = mListState.Provider()->DataIndex(s);
    if (scroll == -1) {
        if (warn) {
            MILO_NOTIFY("Couldn't find %s in UIList provider", s);
        }
        return false;
    } else {
        SetSelectedSimulateScroll(scroll);
        return true;
    }
}

void UIList::Update() {
    if (!gLoading) {
        MILO_ASSERT(mListDir, 0x238);
        mListDir->CreateElements(this, mWidgets, mListState.NumDisplay());

        if (TheLoadMgr.EditMode())
            Refresh(false);
    }
}

int UIList::NumDisplay() const { return mListState.NumDisplay(); }

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
    if (Circular()) {
        mLimitCircularDisplayNumToDataNum = b;
        if (b) {
            SetNumDisplay(Max(Min(unk160, NumProviderData()), 1));
        } else {
            SetNumDisplay(unk160);
        }
        Refresh(false);
    }
}

void UIList::SetProvider(UIListProvider *prov) {
    if (prov == mListState.Provider()) {
        LimitCircularDisplay(mLimitCircularDisplayNumToDataNum);
        Refresh(true);
    } else {
        mListState.SetProvider(prov, mListDir);
        LimitCircularDisplay(mLimitCircularDisplayNumToDataNum);
        SetSelected(0, -1);
    }
    if (ChildList()) {
        Poll();
    }
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

void UIList::CalcBoundingBox(Box &box) {
    box.Set(WorldXfm().v, WorldXfm().v);
    UIListWidgetDrawState drawState;
    mListDir->DrawWidgets(
        drawState, mListState, mWidgets, WorldXfm(), DrawState(this), &box, unk15d
    );
}

const std::vector<UIListWidget *> &UIList::GetWidgets() const { return mWidgets; }

void UIList::BoundingBoxTriangles(std::vector<std::vector<Vector3> > &vec) {
    vec.clear();
    Box box;
    CalcBoundingBox(box);
    std::vector<Vector3> locVec;
    for (int i = 0; i < 2; i++) {
        float f;
        if (i != 0)
            f = box.mMin.x;
        else
            f = box.mMax.x;
        locVec.clear();
        locVec.push_back(Vector3(f, box.mMin.y, box.mMin.z));
        locVec.push_back(Vector3(f, box.mMin.y, box.mMax.z));
        locVec.push_back(Vector3(f, box.mMax.y, box.mMax.z));
        vec.push_back(locVec);
        locVec.clear();
        locVec.push_back(Vector3(f, box.mMin.y, box.mMin.z));
        locVec.push_back(Vector3(f, box.mMax.y, box.mMin.z));
        locVec.push_back(Vector3(f, box.mMax.y, box.mMax.z));
        vec.push_back(locVec);
    }
    for (int i = 0; i < 2; i++) {
        float f;
        if (i != 0)
            f = box.mMin.y;
        else
            f = box.mMax.y;
        locVec.clear();
        locVec.push_back(Vector3(box.mMin.x, f, box.mMin.z));
        locVec.push_back(Vector3(box.mMin.x, f, box.mMax.z));
        locVec.push_back(Vector3(box.mMax.x, f, box.mMax.z));
        vec.push_back(locVec);
        locVec.clear();
        locVec.push_back(Vector3(box.mMin.x, f, box.mMin.z));
        locVec.push_back(Vector3(box.mMax.x, f, box.mMin.z));
        locVec.push_back(Vector3(box.mMax.x, f, box.mMax.z));
        vec.push_back(locVec);
    }
    for (int i = 0; i < 2; i++) {
        float f;
        if (i != 0)
            f = box.mMin.z;
        else
            f = box.mMax.z;
        locVec.clear();
        locVec.push_back(Vector3(box.mMin.x, box.mMin.y, f));
        locVec.push_back(Vector3(box.mMin.x, box.mMax.y, f));
        locVec.push_back(Vector3(box.mMax.x, box.mMax.y, f));
        vec.push_back(locVec);
        locVec.clear();
        locVec.push_back(Vector3(box.mMin.x, box.mMin.y, f));
        locVec.push_back(Vector3(box.mMax.x, box.mMin.y, f));
        locVec.push_back(Vector3(box.mMax.x, box.mMax.y, f));
        vec.push_back(locVec);
    }
}

DataNode UIList::OnScroll(DataArray *da) {
    int scroll = da->Int(2);
    mUser = da->Size() > 3 ? da->Obj<LocalUser>(3) : 0;
    Scroll(scroll);
    return 1;
}

DataNode UIList::OnSelectedSym(DataArray *da) {
    if (da->Size() > 2) {
        return SelectedSym(da->Int(2));
    } else
        return SelectedSym(true);
}

DataNode UIList::OnSetSelected(DataArray *da) {
    DataNode node = da->Evaluate(2);
    int i6 = -1;
    if (node.Type() == kDataInt) {
        if (da->Size() == 4)
            i6 = da->Int(3);
        SetSelected(node.Int(), i6);
        return 1;
    } else if (node.Type() == kDataSymbol || node.Type() == kDataString) {
        bool i3 = da->Size() == 4 ? da->Int(3) : true;
        if (da->Size() == 5)
            i6 = da->Int(4);
        return SetSelected(node.ForceSym(), i3, i6);
    } else {
        MILO_FAIL("bad arg to set_selected");
        return 0;
    }
}

DataNode UIList::OnMsg(const ButtonDownMsg &msg) {
    mUser = msg.GetUser();
    Symbol cntType = JoypadControllerTypePadNum(msg.GetPadNum());

    if (CanScroll()) {
        int gridspan = mListState.GridSpan();
        UIList *childList = ChildList();
        UIListOrientation o = mListDir->Orientation();
        bool b1 = false;

        if (childList) {
            if (childList->Handle(msg, false) != DATA_UNHANDLED) {
                return 1;
            }

            int scrollDir = ScrollDirection(
                msg,
                cntType,
                childList->GetUIListDir()->Orientation() == 0,
                childList->GridSpan()
            );
            if ((scrollDir == 1
                 && childList->SelectedData() == childList->NumProviderData() - 1)
                || (scrollDir == -1 && childList->SelectedData() == 0)) {
                o = childList->GetUIListDir()->Orientation();
                b1 = true;
            }
        }

        int scrollDir = ScrollDirection(msg, cntType, o == 0, gridspan);
        if (scrollDir != 0) {
            if (gridspan == 1 || (scrollDir != 1 && scrollDir != -1)
                || ((scrollDir == 1 && (mListState.SelectedDisplay() + 1) % gridspan)
                    || (scrollDir == -1 && mListState.SelectedDisplay() % gridspan))) {
                int oldSelData = SelectedData();
                Scroll(scrollDir);
                if (oldSelData == SelectedData() && !IsScrolling() && !mSelectToScroll) {
                    return DATA_UNHANDLED;
                }

                int oldNextFill = UIListSubList::sNextFillSelection;
                if (childList) {
                    UIList *curChild = ChildList();
                    bool b2 = false;
                    if (curChild == childList) {
                        int dispFill = scrollDir + mListState.SelectedDisplay();
                        if (dispFill < 0 || dispFill >= NumDisplay())
                            b2 = true;
                        else {
                            curChild = mListDir->SubList(dispFill, mWidgets);
                        }
                    }
                    // oldNextFill = UIListSubList::sNextFillSelection;
                    if (curChild) {
                        if (b1) {
                            if (scrollDir > 0)
                                oldNextFill = 0;
                            else if (b2)
                                oldNextFill = 1000000;
                            else
                                oldNextFill = curChild->NumProviderData() - 1;
                        } else {
                            oldNextFill =
                                Min(curChild->NumProviderData() - 1,
                                    childList->SelectedData());
                        }
                        if (b2)
                            UIListSubList::sNextFillSelection = oldNextFill;
                        else
                            curChild->SetSelectedSimulateScroll(oldNextFill);
                    }
                }

                return 1;
            }

            return 1;
        }

        int pageDir = PageDirection(msg.GetAction());
        if (pageDir != 0) {
            if (mPaginate) {
                mListState.PageScroll(pageDir);
                return 1;
            }
        } else if (pageDir == 0) {
            if (CatchNavAction(msg.GetAction()))
                return 1;
        }
    }

    if (!IsScrolling()) {
        if (msg.GetAction() == kAction_Confirm) {
            if (SelectScrollSelect(this, mUser))
                return 1;
            SendSelect(mUser);
            return 1;
        }

        if (msg.GetAction() == kAction_Cancel && RevertScrollSelect(this, mUser, 0)) {
            return 1;
        }
    }

    return DATA_UNHANDLED;
}

DataNode UIList::OnSetSelectedSimulateScroll(DataArray *da) {
    DataNode node = da->Evaluate(2);
    if (node.Type() == kDataInt) {
        SetSelectedSimulateScroll(node.Int());
        return DataNode(1);
    } else if (node.Type() == kDataSymbol || node.Type() == kDataString) {
        bool b3 = da->Size() == 4 ? da->Int(3) : true;
        return SetSelectedSimulateScroll(node.ForceSym(), b3);
    } else {
        MILO_FAIL("bad arg to set_selected_simulate_scroll");
        return 0;
    }
}

DataNode UIList::OnSetData(DataArray *da) {
    DataArray *arr = da->Array(2);
    int i3 = da->Size() > 3 ? da->Int(3) : 0;
    bool i4 = da->Size() > 4 ? da->Int(4) : 0;
    bool i5 = da->Size() > 5 ? da->Int(5) : 0;
    if (mDataProvider)
        mDataProvider->SetData(arr);
    else
        mDataProvider = new DataProvider(arr, i3, i4, i5, this);
    SetProvider(mDataProvider);
    return 1;
}
