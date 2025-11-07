#include "ui/UISlider.h"
#include "UIComponent.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Draw.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

void UISlider::OldResourcePreload(BinStream &bs) {
    char buf[256];
    bs.ReadString(buf, 256);
    unk50.SetName(buf, true);
}

UISlider::UISlider() : unk50(this), mCurrent(0), mNumSteps(10), mVertical(0) {}

BEGIN_PROPSYNCS(UISlider)
    SYNC_PROP_MODIFY(slider_resource, unk50, Update())
    SYNC_PROP(vertical, mVertical)
    SYNC_SUPERCLASS(ScrollSelect)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(UISlider)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(UIComponent)
    bs << unk50;
    bs << mVertical;
END_SAVES

BEGIN_COPYS(UISlider)

END_COPYS

BEGIN_LOADS(UISlider)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void UISlider::SetTypeDef(DataArray *) {}

void UISlider::PreLoad(BinStream &bs) {}

void UISlider::PostLoad(BinStream &bs) {}

void UISlider::DrawShowing() {}

RndDrawable *UISlider::CollideShowing(const Segment &, float &, Plane &) {
    return nullptr;
}

int UISlider::CollidePlane(const Plane &pl) { return 1; }

void UISlider::Enter() {
    UIComponent::Enter();
    Reset();
}

void UISlider::SetCurrent(int i) {
    if (i < 0 || i >= mNumSteps) {
        MILO_FAIL("Can't set slider to %i (%i steps)", i, mNumSteps);
    } else
        mCurrent = i;
}

int UISlider::SelectedAux() const { return Current(); }

void UISlider::SetSelectedAux(int i) { SetCurrent(i); }

DataNode UISlider::OnMsg(const ButtonDownMsg &) { return NULL_OBJ; }

void UISlider::SyncSlider() {}

float UISlider::Frame() const {
    if (mNumSteps == 1)
        return 0.0f;
    else
        return (float)(mCurrent) / (float)(mNumSteps - 1);
}

void UISlider::SetNumSteps(int i) {
    if (i < 1)
        MILO_FAIL("Can't set num steps to %i (must be >= 1)", i);
    else
        mNumSteps = i;
}

void UISlider::SetFrame(float frame) {
    MILO_ASSERT(frame >= 0 && frame <= 1.0f, 0xe2);
    mCurrent = frame * (mNumSteps - 1) + 0.5f;
}

int UISlider::Current() const { return mCurrent; }

void UISlider::Init() {}

void UISlider::Update() {
    static Symbol mesh("mesh");
    static Symbol mats("mats");

    unk68 = 0;
    unk6c = 0;
    unk70 = 0;
    unk74 = 0;
    unk78 = 0;
    unk7c = 0;
}

BEGIN_HANDLERS(UISlider)

END_HANDLERS
