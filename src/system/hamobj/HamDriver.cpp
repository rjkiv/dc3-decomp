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

HamDriver::HamDriver() : mBones(this), mOldBeat(-kHugeFloat) {}

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

float HamDriver::Display(float top) {
    float scaledHeight = TheRnd.Height() * top;
    auto pathName = PathName(this);
    Hmx::Color color(1.0f, 1.0f, 1.0f, 1.0f);
    Vector2 screenPos(CharClipDisplay::Em(), scaledHeight);
    TheRnd.DrawString(
        MakeString("%s beat: %.2f", pathName, mOldBeat), screenPos, color, true
    );
    CharClipDisplay::Init(this->Dir());
    float lineSpacing = CharClipDisplay::LineSpacing() + scaledHeight;
    if (mBones && Weight() != 0) {
        FOREACH (it, mLayers.layers) {
            lineSpacing = DisplayRecurse(*it, 0, lineSpacing);
        }
    }
    return lineSpacing / TheRnd.Height();
}

void HamDriver::Poll() {
    if (mBones && Weight() > 0) {
        mLayers.Eval(Weight());
        mBones->ScaleDown(*mBones, 1.0f - mLayers.usedWeight);
        mLayers.Play(*mBones);
        mOldBeat = TheTaskMgr.Beat();
    }
}

void HamDriver::SetClipWeightMap() {
    mClipWeights.clear();
    SetClipMapRecurse(&mLayers);
    float total = 0;
    FOREACH (it, mClipWeights) {
        total += it->second;
    }

    if (total > 0) {
        FOREACH (it, mClipWeights) {
            it->second *= (1.0f / total);
        }
    }
}

float HamDriver::DisplayRecurse(Layer *layer, int indent, float top) {
    LayerArray *array = dynamic_cast<LayerArray *>(layer);
    if (array) {
        if (array->usedWeight != 0) {
            float sem = indent * CharClipDisplay::Em();
            CharClipDisplay display;
            display.mTop = top;
            display.mBeat = mOldBeat;
            display.mIndent = sem;
            display.SetText(MakeString("(%s)", array->name));
            display.SetStartEnd(mOldBeat - 4.0f, mOldBeat + 4.0f, true);
            display.mWeight = array->usedWeight;
            display.DrawTrack();
            display.DrawBlend(array->blendStart, 1.0f);
            display.DrawCursor();
            top += CharClipDisplay::LineSpacing();
            int x = indent + 1;
            FOREACH (it, array->layers) {
                top = DisplayRecurse(*it, x, top);
            }
        }
    } else {
        LayerClip *clip = dynamic_cast<LayerClip *>(layer);
        if (clip && clip->usedWeight != 0) {
            float sem = indent * CharClipDisplay::Em();
            CharClipDisplay display;
            float beat = (mOldBeat - clip->startBeat) + clip->clip->StartBeat();
            display.mIndent = sem;
            display.mBeat = beat;
            display.mWeight = clip->usedWeight;
            display.SetClip(clip->clip, true);
            display.mTop = top;
            display.DrawTrack();
            float beat2 = (clip->clip->StartBeat() + clip->blendStart) - clip->startBeat;
            display.DrawBlend(beat2, 1.0f);
            display.DrawCursor();
            top += CharClipDisplay::LineSpacing();
        }
    }
    return top;
}

void HamDriver::Clear() { mLayers.Clear(); }
HamDriver::LayerClip *HamDriver::NewLayerClip() { return new LayerClip(this); }
void HamDriver::OffsetSec(float f) { return mLayers.OffsetSec(f); }
CharClip *HamDriver::FirstClip() { return mLayers.FirstClip(); }

#pragma region HamDriver::Layer

void HamDriver::Layer::OffsetSec(float sec) {
    blendStart = SecondsToBeat(BeatToSeconds(blendStart) + sec);
}

void HamDriver::SetClipMapRecurse(HamDriver::Layer *layer) {
    LayerArray *array = dynamic_cast<LayerArray *>(layer);
    if (array) {
        if (array->usedWeight != 0) {
            FOREACH (it, array->layers) {
                SetClipMapRecurse(*it);
            }
        }
    } else {
        LayerClip *clip = dynamic_cast<LayerClip *>(layer);
        if (clip && clip->usedWeight != 0) {
            CharClip *c = clip->clip;
            auto it = mClipWeights.find(c);
            if (it != mClipWeights.end()) {
                it->second += clip->usedWeight;
            } else {
                mClipWeights.insert(std::pair<CharClip *, float>(c, clip->usedWeight));
            }
        }
    }
}

#pragma endregion

#pragma region HamDriver::LayerClip

HamDriver::LayerClip::LayerClip(Hmx::Object *obj) : clip(obj) {}

void HamDriver::LayerClip::OffsetSec(float sec) {
    Layer::OffsetSec(sec);
    startBeat = SecondsToBeat(BeatToSeconds(startBeat) + sec);
}

void HamDriver::LayerClip::Eval(float weight) {
    float beat = TheTaskMgr.Beat();
    auto clamped = Clamp(0.0f, 1.0f, beat - blendStart);
    usedWeight = EaseSigmoid(clamped, 0.0, 0.0) * weight;
}

bool HamDriver::LayerClip::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (&clip == ref && !clip.SetObj(obj)) {
        if (this) {
            delete this;
        }
        return true;
    }
    return false;
}

void HamDriver::LayerClip::Play(CharBones &bones) {
    if (usedWeight > 0) {
        float startbeat = clip->StartBeat();
        float val = (TheTaskMgr.Beat() - startBeat) + startbeat;
        bones.ScaleAdd(clip, usedWeight, val, TheTaskMgr.DeltaBeat());
    }
}

#pragma endregion

#pragma region HamDriver::LayerArray

void HamDriver::LayerArray::Clear() {
    FOREACH (it, layers) {
        delete *it;
    }
    layers.clear();
}

bool HamDriver::LayerArray::Replace(ObjRef *ref, Hmx::Object *obj) {
    FOREACH (it, layers) {
        if (it == layers.end()) {
            return false;
        }
        bool replaced = (*it)->Replace(ref, obj);
        if (replaced) {
            layers.erase(it);
            break;
        }
    }
    return false;
}

void HamDriver::LayerArray::Play(CharBones &bones) {
    if (usedWeight > 0.0) {
        FOREACH (it, layers) {
            (*it)->Play(bones);
        }
    }
}

CharClip *HamDriver::LayerArray::FirstClip() {
    CharClip *clip;
    FOREACH (it, layers) {
        clip = (*it)->FirstClip();
        if (clip != nullptr) {
            return clip;
        }
    }
    return nullptr;
}

void HamDriver::LayerArray::OffsetSec(float delta) {
    Layer::OffsetSec(delta);
    FOREACH (it, layers) {
        (*it)->OffsetSec(delta);
    }
}

void HamDriver::LayerArray::Eval(float weight) {
    usedWeight = 0;
    if (weight > 0) {
        float elapsed = TheTaskMgr.Beat() - blendStart;
        if (elapsed > 0) {
            float val = (elapsed - 1.0f < 0) ? elapsed : 1.0f;
            float sigmoid = EaseSigmoid(val, 0, 0) * weight;
            FOREACH (it, layers) {
                (*it)->Eval(sigmoid);
                float val8 = (*it)->usedWeight;
                float val2 = (val8 - sigmoid < 0) ? val8 : sigmoid;
                usedWeight += val2;
                sigmoid -= val2;
            }
        }
    }
}

#pragma endregion
