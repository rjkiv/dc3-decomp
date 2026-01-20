#include "hamobj/HamDriver.h"

#include "char/CharClipDisplay.h"
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
    Hmx::Color color(1.0, 1.0, 1.0, 1.0);
    Vector2 screenPos(CharClipDisplay::GetSEm(), scaledHeight);
    auto stringDisplay = MakeString("%s beat: %.2f", pathName, unk78);
    TheRnd.DrawString(stringDisplay, screenPos, color, true);
    CharClipDisplay::Init(this->Dir());
    float lineSpacing = CharClipDisplay::LineSpacing() + scaledHeight;
    for (auto it = mLayers.unk2c.begin(); it != mLayers.unk2c.end() && mWeight != 0.0; ++it) {
        lineSpacing = DisplayRecurse(*it, 0, lineSpacing);
    }
    return lineSpacing / TheRnd.Height();
}

void HamDriver::Clear() { mLayers.Clear(); }
HamDriver::LayerClip *HamDriver::NewLayerClip() { return new LayerClip(this); }
void HamDriver::OffsetSec(float f) { return mLayers.OffsetSec(f); }
CharClip *HamDriver::FirstClip() { return mLayers.FirstClip(); }

#pragma region HamDriver::Layer

void HamDriver::Layer::OffsetSec(float f1) {
    unk4 = SecondsToBeat(BeatToSeconds(unk4) + f1);
}

#pragma endregion

#pragma region HamDriver::LayerClip

HamDriver::LayerClip::LayerClip(Hmx::Object *obj) : unk10(obj) {}

HamDriver::LayerClip::~LayerClip() {}

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
    if ((ObjRef *)unk10.Ptr() == ref && unk10.SetObj(obj) == nullptr) {
        delete unk10;
        return true;
    }
    return false;
}
#pragma endregion

#pragma region HamDriver::LayerArray

void HamDriver::LayerArray::Clear() {
    FOREACH(it, unk2c) {
        delete *it;
    }
    unk2c.clear();
}

bool HamDriver::LayerArray::Replace(ObjRef *ref, Hmx::Object *obj) {
    FOREACH(it, unk2c) {
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
        FOREACH(it, unk2c) {
            (*it)->Play(bones);
        }
    }
}

CharClip *HamDriver::LayerArray::FirstClip() {
    CharClip *clip;
    FOREACH(it, unk2c) {
        clip = (*it)->FirstClip();
        if (clip != nullptr) {
            break;
        }
    }
    return clip;
}

void HamDriver::LayerArray::OffsetSec(float f1) {
    Layer::OffsetSec(f1);
    FOREACH(it, unk2c) {
        (*it)->OffsetSec(f1);
    }
}


#pragma endregion