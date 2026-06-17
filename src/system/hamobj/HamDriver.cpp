#include "hamobj/HamDriver.h"

#include "char/CharClipDisplay.h"
#include "math/Easing.h"
#include "obj/Task.h"
#include "stl/_pair.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/TimeConversion.h"
#include "char/Char.h"
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "utl/BinStream.h"

HamDriver::HamDriver() : mBones(this), unk78(-kHugeFloat) {}

HamDriver::~HamDriver() { Clear(); }

BEGIN_HANDLERS(HamDriver)
    HANDLE_SUPERCLASS(CharPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamDriver)
    SYNC_PROP(bones, mBones)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(CharPollable)
END_PROPSYNCS

BEGIN_SAVES(HamDriver)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mBones;
END_SAVES

BEGIN_COPYS(HamDriver)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(HamDriver)
    BEGIN_COPYING_MEMBERS
        mBones = (CharBonesObject *)c->mBones;
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(HamDriver)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(1, 0)

void HamDriver::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    d >> mBones;
}

void HamDriver::Enter() { Clear(); }

void HamDriver::Highlight() {
    if (gCharHighlightY == -1) {
        CharDeferHighlight(this);
    } else {
        gCharHighlightY = Display(gCharHighlightY);
    }
}

void HamDriver::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mBones);
}

bool HamDriver::Replace(ObjRef *ref, Hmx::Object *obj) {
    mLayers.Replace(ref, obj);
    bool replaced = CharWeightable::Replace(ref, obj);
    return replaced;
}

float HamDriver::Display(float f1) {
    float scaledHeight = TheRnd.Height() * f1;
    auto pathName = PathName(this);
    Hmx::Color color(1.0f, 1.0f, 1.0f, 1.0f);
    Vector2 screenPos(CharClipDisplay::GetSEm(), scaledHeight);
    TheRnd.DrawString(
        MakeString("%s beat: %.2f", pathName, unk78), screenPos, color, true
    );
    CharClipDisplay::Init(this->Dir());
    float lineSpacing = CharClipDisplay::LineSpacing() + scaledHeight;
    if (mBones && Weight() != 0) {
        FOREACH (it, mLayers.unk2c) {
            lineSpacing = DisplayRecurse(*it, 0, lineSpacing);
        }
    }
    return lineSpacing / TheRnd.Height();
}

void HamDriver::Poll() {
    if (mBones && Weight() > 0) {
        mLayers.Eval(Weight());
        mBones->ScaleDown(*mBones, 1.0f - mLayers.unk8);
        mLayers.Play(*mBones);
        unk78 = TheTaskMgr.Beat();
    }
}

void HamDriver::SetClipWeightMap() {
    unk7c.clear();
    SetClipMapRecurse(&mLayers);
    float total = 0;
    FOREACH (it, unk7c) {
        total += it->second;
    }

    if (total > 0) {
        FOREACH (it, unk7c) {
            it->second *= (1.0f / total);
        }
    }
}

float HamDriver::DisplayRecurse(Layer *layer, int i, float f) {
    LayerArray *array = dynamic_cast<LayerArray *>(layer);
    if (array) {
        if (array->unk8 != 0) {
            float sem = i * CharClipDisplay::GetSEm();
            CharClipDisplay display;
            display.unk10 = f;
            display.unk1c = unk78;
            display.unk64 = sem;
            display.SetText(MakeString("(%s)", array->unkc));
            display.SetStartEnd(unk78 - 4.0f, unk78 + 4.0f, true);
            display.unk20 = array->unk8;
            display.DrawTrack();
            display.DrawBlend(array->unk4, 1.0f);
            display.DrawCursor();
            f += CharClipDisplay::LineSpacing();
            int x = i + 1;
            FOREACH (it, array->unk2c) {
                f = DisplayRecurse(*it, x, f);
            }
        }
    } else {
        LayerClip *clip = dynamic_cast<LayerClip *>(layer);
        if (clip && clip->unk8 != 0) {
            float sem = i * CharClipDisplay::GetSEm();
            CharClipDisplay display;
            float beat = (unk78 - clip->unkc) + clip->unk10->StartBeat();
            display.unk64 = sem;
            display.unk1c = beat;
            display.unk20 = clip->unk8;
            display.SetClip(clip->unk10, true);
            display.unk18 = f;
            display.DrawTrack();
            float beat2 = (clip->unk10->StartBeat() + clip->unk4) - clip->unkc;
            display.DrawBlend(beat2, 1.0f);
            display.DrawCursor();
            f += CharClipDisplay::LineSpacing();
        }
    }
    return f;
}

void HamDriver::Clear() { mLayers.Clear(); }
HamDriver::LayerClip *HamDriver::NewLayerClip() { return new LayerClip(this); }
void HamDriver::OffsetSec(float f) { return mLayers.OffsetSec(f); }
CharClip *HamDriver::FirstClip() { return mLayers.FirstClip(); }

#pragma region HamDriver::Layer

void HamDriver::Layer::OffsetSec(float f1) {
    unk4 = SecondsToBeat(BeatToSeconds(unk4) + f1);
}

void HamDriver::SetClipMapRecurse(HamDriver::Layer *layer) {
    LayerArray *array = dynamic_cast<LayerArray *>(layer);
    if (array) {
        if (array->unk8 != 0) {
            FOREACH (it, array->unk2c) {
                SetClipMapRecurse(*it);
            }
        }
    } else {
        LayerClip *clip = dynamic_cast<LayerClip *>(layer);
        if (clip && clip->unk8 != 0) {
            CharClip *c = clip->unk10;
            auto it = unk7c.find(c);
            if (it != unk7c.end()) {
                it->second += clip->unk8;
            } else {
                unk7c.insert(std::pair<CharClip *, float>(c, clip->unk8));
            }
        }
    }
}

#pragma endregion

#pragma region HamDriver::LayerClip

HamDriver::LayerClip::LayerClip(Hmx::Object *obj) : unk10(obj) {}

void HamDriver::LayerClip::OffsetSec(float f1) {
    Layer::OffsetSec(f1);
    unkc = SecondsToBeat(BeatToSeconds(unkc) + f1);
}

void HamDriver::LayerClip::Eval(float f1) {
    float beat = TheTaskMgr.Beat();
    auto clamped = Clamp(0.0f, 1.0f, beat - unk4);
    unk8 = EaseSigmoid(clamped, 0.0, 0.0) * f1;
}

bool HamDriver::LayerClip::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (&unk10 == ref && !unk10.SetObj(obj)) {
        if (this) {
            delete this;
        }
        return true;
    }
    return false;
}

void HamDriver::LayerClip::Play(CharBones &bones) {
    if (unk8 > 0) {
        float startbeat = unk10->StartBeat();
        float val = (TheTaskMgr.Beat() - unkc) + startbeat;
        bones.ScaleAdd(unk10, unk8, val, TheTaskMgr.DeltaBeat());
    }
}

#pragma endregion

#pragma region HamDriver::LayerArray

void HamDriver::LayerArray::Clear() {
    FOREACH (it, unk2c) {
        delete *it;
    }
    unk2c.clear();
}

bool HamDriver::LayerArray::Replace(ObjRef *ref, Hmx::Object *obj) {
    FOREACH (it, unk2c) {
        if (it == unk2c.end()) {
            return false;
        }
        bool replaced = (*it)->Replace(ref, obj);
        if (replaced) {
            unk2c.erase(it);
            break;
        }
    }
    return false;
}

void HamDriver::LayerArray::Play(CharBones &bones) {
    if (unk8 > 0.0) {
        FOREACH (it, unk2c) {
            (*it)->Play(bones);
        }
    }
}

CharClip *HamDriver::LayerArray::FirstClip() {
    CharClip *clip;
    FOREACH (it, unk2c) {
        clip = (*it)->FirstClip();
        if (clip != nullptr) {
            return clip;
        }
    }
    return nullptr;
}

void HamDriver::LayerArray::OffsetSec(float f1) {
    Layer::OffsetSec(f1);
    FOREACH (it, unk2c) {
        (*it)->OffsetSec(f1);
    }
}

void HamDriver::LayerArray::Eval(float f) {
    unk8 = 0;
    if (f > 0) {
        float elapsed = TheTaskMgr.Beat() - unk4;
        if (elapsed > 0) {
            float val = (elapsed - 1.0f < 0) ? elapsed : 1.0f;
            float sigmoid = EaseSigmoid(val, 0, 0) * f;
            FOREACH (it, unk2c) {
                (*it)->Eval(sigmoid);
                float val8 = (*it)->unk8;
                float val2 = (val8 - sigmoid < 0) ? val8 : sigmoid;
                unk8 += val2;
                sigmoid -= val2;
            }
        }
    }
}

#pragma endregion
