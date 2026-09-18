#include "ui/UIListDir.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "ui/UIComponent.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"

namespace {
    struct WidgetDrawSort {
        bool operator()(const UIListWidget *w1, const UIListWidget *w2) const {
            return w1->DrawOrder() < w2->DrawOrder();
        }
    };
}

UIListDir::UIListDir()
    : mOrientation(kUIListVertical), mFadeOffset(0), mElementSpacing(50.0f),
      mScrollHighlightChange(0.5f), mTestMode(0), mTestState(this, this),
      mTestNumData(100), mTestGapSize(0.0f), mTestComponentState(UIComponent::kFocused),
      mTestDisableElements(0), mDirection(0) {
    mTestState.SetNumDisplay(5, true);
    mTestState.SetGridSpan(1, true);
    mTestState.SetSelected(0, -1, true);
}

UIListDir::~UIListDir() { DeleteAll(unk270); }

BEGIN_HANDLERS(UIListDir)
    HANDLE_ACTION(test_scroll, mTestState.Scroll(_msg->Int(2), false))
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_PROPSYNCS(UIListDir)
    SYNC_PROP_SET(orientation, mOrientation, mOrientation = (UIListOrientation)_val.Int())
    SYNC_PROP(fade_offset, mFadeOffset)
    SYNC_PROP(element_spacing, mElementSpacing)
    SYNC_PROP(scroll_highlight_change, mScrollHighlightChange)
    SYNC_PROP(test_mode, mTestMode)
    SYNC_PROP(test_num_data, mTestNumData)
    SYNC_PROP(test_gap_size, mTestGapSize)
    SYNC_PROP_SET(
        test_num_display,
        mTestState.NumDisplay(),
        mTestState.SetNumDisplay(_val.Int(), true)
    )
    SYNC_PROP_SET(
        test_grid_span, mTestState.GridSpan(), mTestState.SetGridSpan(_val.Int(), true)
    )
    SYNC_PROP_SET(test_scroll_time, mTestState.Speed(), mTestState.SetSpeed(_val.Float()))
    SYNC_PROP_SET(
        test_list_state,
        mTestComponentState,
        mTestComponentState = (UIComponent::State)_val.Int()
    )
    SYNC_PROP_MODIFY(test_disable_elements, mTestDisableElements, Reset())
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(UIListDir)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(RndDir)
    bs << mOrientation << mFadeOffset;
    int numDisp = mTestState.NumDisplay();
    bs << mTestMode << numDisp << mElementSpacing << mTestState.Speed() << mTestNumData
       << mTestComponentState << mTestGapSize << mTestDisableElements
       << mScrollHighlightChange;
END_SAVES

BEGIN_COPYS(UIListDir)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY_AS(UIListDir, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(mOrientation)
        COPY_MEMBER(mFadeOffset)
        COPY_MEMBER(mElementSpacing)
        COPY_MEMBER(mScrollHighlightChange)
        COPY_MEMBER(mTestMode)
        mTestState.SetNumDisplay(c->mTestState.NumDisplay(), true);
        mTestState.SetGridSpan(c->mTestState.GridSpan(), true);
        mTestState.SetSpeed(c->mTestState.Speed());
        COPY_MEMBER(mTestNumData)
        COPY_MEMBER(mTestComponentState)
        COPY_MEMBER(mTestGapSize)
        COPY_MEMBER(mTestDisableElements)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

void UIListDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    RndDir::PreLoad(bs);
    d.PushRev(this);
}

void UIListDir::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    RndDir::PostLoad(bs);
    int orientation;
    d >> orientation;
    d >> mFadeOffset;
    mOrientation = (UIListOrientation)orientation;
    int numDisp;
    float speed;
    int state;
    d >> mTestMode >> numDisp >> mElementSpacing >> speed >> mTestNumData >> state
        >> mTestGapSize >> mTestDisableElements;
    if (d.rev > 0) {
        d >> mScrollHighlightChange;
    }
    mTestState.SetNumDisplay(numDisp, true);
    mTestState.SetSpeed(speed);
    mTestComponentState = (UIComponent::State)state;
}

void UIListDir::SyncObjects() {
    RndDir::SyncObjects();
    if (TheLoadMgr.EditMode()) {
        CreateElements(0, unk270, mTestState.NumDisplay());
        FillElements(mTestState, unk270);
    }
}

void UIListDir::DrawShowing() {
    if (mTestMode && TheLoadMgr.EditMode()) {
        UIListWidgetDrawState drawState;
        BuildDrawState(drawState, mTestState, mTestComponentState, 0, true);
        DrawWidgets(
            drawState, mTestState, unk270, WorldXfm(), mTestComponentState, nullptr, false
        );
    } else {
        RndDir::DrawShowing();
    }
}

void UIListDir::Poll() {
    if (TheLoadMgr.EditMode()) {
        RndDir::Poll();
        if (mTestMode) {
            mTestState.Poll(TheTaskMgr.Seconds(TaskMgr::kRealTime));
            PollWidgets(unk270);
        }
    }
}

int UIListDir::NumData() const { return mTestNumData; }

float UIListDir::GapSize(int, int, int, int) const { return mTestGapSize; }

bool UIListDir::IsActive(int i) const {
    if (mTestDisableElements)
        return !(i % 2);
    else
        return true;
}

void UIListDir::StartScroll(const UIListState &state, int i, bool b) {
    StartScroll(state, unk270, i, b);
}

void UIListDir::CompleteScroll(const UIListState &state) {
    CompleteScroll(state, unk270);
}

UIListOrientation UIListDir::Orientation() const { return mOrientation; }

float UIListDir::ElementSpacing() const { return mElementSpacing; }

UIList *UIListDir::SubList(int i, std::vector<UIListWidget *> &vec) {
    FOREACH (it, vec) {
        UIList *l = (*it)->SubList(i);
        if (l)
            return l;
    }
    return nullptr;
}

void UIListDir::DrawWidgets(
    UIListWidgetDrawState &drawState,
    const UIListState &listState,
    std::vector<UIListWidget *> &widgets,
    const Transform &xfm,
    UIComponent::State compState,
    Box *box,
    bool b7
) {
    bool scrolling = listState.IsScrolling();
    FOREACH (it, widgets) {
        bool focused = compState == UIComponent::kFocused;
        UIListWidget *cur = *it;
        UIListWidgetDrawType drawType = cur->DrawType();
        if ((drawType == 0) || (drawType == 3 && (b7 || focused))
            || (drawType == 1 && focused)) {
            cur->Draw(
                drawState,
                listState,
                xfm,
                compState,
                box,
                scrolling ? kExcludeFirst : kDrawAll
            );
        }
    }
    if (scrolling) {
        FOREACH (it, widgets) {
            UIListWidget *cur = *it;
            UIListWidgetDrawType drawType = cur->DrawType();
            if ((drawType == 0)
                || (drawType == 1 && compState == UIComponent::kFocused)) {
                cur->Draw(drawState, listState, xfm, compState, box, kDrawFirst);
            }
        }
    }
}

void UIListDir::PollWidgets(std::vector<UIListWidget *> &widgets) {
    FOREACH (it, widgets) {
        (*it)->Poll();
    }
}

void UIListDir::FillElement(
    UIListState const &state, std::vector<UIListWidget *> &vec, int i
) {
    int disp = state.Display2Data(i);
    if (disp != -1) {
        int snapped = state.SnappedDataForDisplay(i);
        if (snapped >= 0)
            disp = snapped;
        int disp2show = state.Display2Showing(i);
        bool isnegone = i == -1;
        i = Clamp(0, state.NumDisplay(), i);
        FOREACH (it, vec) {
            (*it)->Fill(*state.Provider(), i, disp2show, disp);
            if (isnegone && snapped >= 0) {
                (*it)->Fill(
                    *state.Provider(), 1, state.Display2Showing(0), state.Display2Data(0)
                );
            }
        }
    }
}

void UIListDir::StartScroll(
    UIListState const &state, std::vector<UIListWidget *> &widgets, int i, bool b
) {
    mDirection = i;
    MILO_ASSERT(mDirection, 0x1ED);
    FOREACH (it, widgets) {
        (*it)->StartScroll(mDirection, b);
    }
    if (b) {
        FillElement(state, widgets, mDirection > 0 ? state.NumDisplay() : -1);
    }
}

void UIListDir::CompleteScroll(
    UIListState const &state, std::vector<UIListWidget *> &widgets
) {
    FOREACH (it, widgets) {
        (*it)->CompleteScroll(state, mDirection);
    }
    if (mDirection == 1 && state.SnappedDataForDisplay(0) >= 0) {
        FillElement(state, widgets, 0);
    }
}

void UIListDir::FillElements(UIListState const &state, std::vector<UIListWidget *> &vec) {
    int num = state.NumDisplayWithData();
    for (int i = 0; i < num; i++) {
        FillElement(state, vec, i);
    }
}

void UIListDir::ListEntered() {
    static Message start("start");
    Handle(start, false);
}

void UIListDir::BuildDrawState(
    UIListWidgetDrawState &drawState,
    const UIListState &listState,
    UIComponent::State compState,
    float f4,
    bool b5
) const {
    int numDisplay = listState.NumDisplay();
    int numDisplayWithData = listState.NumDisplayWithData();
    int i198 = Min(mFadeOffset, numDisplay / 2);
    int i194 = i198;
    if (mFadeOffset != 0) {
        if (listState.Circular()) {
            MinEq(i194, listState.SelectedDisplay());
            MinEq(i198, numDisplay - listState.SelectedDisplay() - 1);
        } else {
            int i19c = listState.FirstShowing();
            if (listState.ScrollPastMinDisplay()) {
                i19c -= listState.MinDisplay();
            }
            MaxEq(i19c, 0);
            MinEq(i194, i19c);
            int i1a8 = i19c + numDisplay;
            i1a8 = listState.Provider()->NumData() - i1a8;
            MinEq(i198, i1a8);
        }
    }
    float f27 = (float)i194 * mElementSpacing;
    float f26 = (float)(numDisplay - 1 - i198) * mElementSpacing;
    int i1 = listState.CurrentScroll() > 0 ? 1 : -1;
    int selected = listState.Selected();
    int selectedData = listState.SelectedData();
    int selectedDisplay = listState.SelectedDisplay();
    drawState.mHighlightDisplay = selectedDisplay;
    if (listState.IsScrolling()) {
        if (listState.StepPercent() > mScrollHighlightChange) {
            selected += i1;
            drawState.mHighlightDisplay += i1;
        }
        numDisplayWithData++;
    }
    drawState.mElements.clear();
    drawState.mElements.reserve(numDisplayWithData);
    drawState.mHighlightElementState = kUIListWidgetActive;
    float f24 = 0;
    float f18 = 0;
    float f23 = 0;
    float f25 = 0;
    float f19 = listState.StepPercent() * (float)i1;
    int i7 = 0;
    for (int i14 = 0; i14 < numDisplayWithData; i14++) {
        int i15 = i14;
        if (listState.IsScrolling() && i1 == -1) {
            i15 = i14 - 1;
        }
        int d2d = listState.Display2Data(i15);
        if (d2d == -1) {
            UIListElementDrawState newState;
            newState.mDraw = false;
            drawState.mElements.push_back(newState);
        } else if (listState.Circular() || i7 <= d2d) {
            int i9 = listState.Display2Showing(i15);
            i7 = d2d;
            int i10 = listState.SnappedDataForDisplay(i15);
            if (i10 >= 0) {
                i7 = i10;
            }
            float f20 = listState.Provider()->GapSize(i9, i7, selectedData, i1);
            if (i14 == 0) {
                f25 = f20;
            }
            float f21;
            Vector3 v190;
            if (listState.ShouldHoldDisplayInPlace(i15)) {
                if (i1 == -1) {
                    f21 = SetElementPos(
                        v190, (float)i15 + 1.0f, listState.GridSpan(), f23, 0
                    );
                } else {
                    f21 = SetElementPos(v190, (float)i15, listState.GridSpan(), f23, 0);
                }
            } else {
                f21 = SetElementPos(
                    v190, (float)i15 - f19, listState.GridSpan(), -(f25 * f19 - f23), 0
                );
            }
            float f22 = 1;
            if (!listState.ShouldHoldDisplayInPlace(i15)) {
                f21 = f21 - -(f25 * f19 - f23);
                if (f21 < f27) {
                    f22 -= (f27 - f21) / ((float)(i194 + 1) * mElementSpacing);
                } else if (f21 > f26) {
                    f22 -= (f21 - f26) / ((float)(i198 + 1) * mElementSpacing);
                }
            }
            int i16;
            if (!listState.Provider()->IsActive(i7)) {
                i16 = 2;
            } else if (selected != i9 || !b5) {
                i16 = 0;
            } else {
                i16 = 1;
            }
            UIListWidgetState ws = listState.Provider()->ElementStateOverride(
                selected, i7, (UIListWidgetState)i16
            );
            if (selected == i9) {
                drawState.mHighlightElementState = ws;
            }
            UIListProvider *prov = listState.Provider();
            UIListElementDrawState newState;
            newState.mDraw = true;
            newState.mPos = v190;
            newState.mAlpha = f22;
            newState.unk14.Set(0, 0, 0);
            newState.mElementState = ws;
            newState.mComponentState =
                prov->ComponentStateOverride(selected, i7, compState);
            newState.mDisplay = i15;
            newState.mShowing = selected;
            newState.mData = i7;
            drawState.mElements.push_back(newState);
            f23 += f20;
            if (i15 > 0 && i15 < numDisplay - 1) {
                f18 += f20;
            }
            if (i15 < selectedDisplay) {
                f24 += f20;
            }
        } else {
            break;
        }
    }
    SetElementPos(drawState.mFirstPos, 0, listState.GridSpan(), 0, 0);
    SetElementPos(
        drawState.mLastPos, listState.NumDisplay() - 1, listState.GridSpan(), f18, 0
    );
    SetElementPos(drawState.mHighlightPos, selectedDisplay, listState.GridSpan(), f24, f4);
}

void UIListDir::CreateElements(UIList *uilist, std::vector<UIListWidget *> &vec, int i) {
    DeleteAll(vec);
    for (ObjDirItr<UIListWidget> it(this, true); it != nullptr; ++it) {
        Hmx::Object *obj = Hmx::Object::NewObject(it->ClassName());
        UIListWidget *widget = dynamic_cast<UIListWidget *>(obj);
        widget->ResourceCopy(it);
        widget->SetParentList(uilist);
        vec.push_back(widget);
    }
    std::sort(vec.begin(), vec.end(), WidgetDrawSort());
    FOREACH (it, vec) {
        (*it)->CreateElements(uilist, i);
    }
}

float UIListDir::SetElementPos(Vector3 &v, float f1, int i2, float f3, float f4) const {
    v.Zero();
    int floored = floorf(f1);
    float f3toset =
        mElementSpacing * ((f1 - (float)floored) + (float)(floored / i2)) + f3;
    float f2toset = mElementSpacing * (float)(floored % i2) + f4;
    if (mOrientation == kUIListVertical) {
        v.z -= f3toset;
        v.x += f2toset;
    } else {
        v.x += f3toset;
        v.z -= f2toset;
    }
    return f3toset;
}

void UIListDir::Reset() {
    mTestState.SetSelected(0, -1, true);
    FillElements(mTestState, unk270);
}
