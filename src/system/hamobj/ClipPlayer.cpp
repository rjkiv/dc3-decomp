#include "hamobj/ClipPlayer.h"
#include "HamRegulate.h"
#include "MoveMgr.h"
#include "char/CharClip.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamDriver.h"
#include "hamobj/SongUtl.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "rndobj/PropAnim.h"
#include "rndobj/PropKeys.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"

const char *ClipPlayer::sRestStepNames[4] = {
    "rest_step_left", "rest_step_right", "rest_step_fwd", "rest_step_back"
};

namespace {
    float ClipStart(CharClip *clip, float beat, float &start, float &end) {
        if (fmodf(beat, 1.0) != 0.0f && ceilf(beat) - beat < 0.0001f) {
            beat = ceil(beat);
        }
        float offset = 0.0f;
        float period = (clip->PlayFlags() >> 12) & 0xF;

        if (period != 0.0f) {
            offset = Mod(beat - clip->StartBeat(), period);
        }

        start = beat - offset;
        end = clip->EndBeat() - clip->StartBeat() + start;

        return clip->StartBeat() + offset;
    }
}

void Annotate(DataArray *a, float f, const char *cc) {
    a->Insert(a->Size(), DataArrayPtr(BeatToFrame(f), cc));
}

bool ClipPlayer::Init(RndPropAnim *anim) {
    mClipDir = TheHamDirector->ClipDir();
    if (anim) {
        PropKeys *clipKeys = anim->GetKeys(TheHamDirector, DataArrayPtr(Symbol("clip")));
        if (clipKeys) {
            mClipKeys = clipKeys->AsSymbolKeys();
        }
        PropKeys *clipCrossoverKeys =
            anim->GetKeys(TheHamDirector, DataArrayPtr(Symbol("clip_crossover")));
        if (clipCrossoverKeys) {
            mClipCrossoverKeys = clipCrossoverKeys->AsSymbolKeys();
        }
        PropKeys *masterKeys = TheHamDirector->GetMasterKeys("clip");
        if (masterKeys) {
            mMasterClipKeys = masterKeys->AsSymbolKeys();
        }
        if (mClipKeys && mMasterClipKeys && mClipDir) {
            Key<Symbol> *k1;
            Key<Symbol> *k2;
            if (TheHamDirector->GetPracticeFrames(k1, k2)) {
                unk20 = Round(FrameToBeat(k1->frame));
                unk24 = Round(FrameToBeat(k2->frame)) - 1.0f;
                String str(k1->value);
                str.ReplaceAll('*', '\0');
                mInClip =
                    mClipDir->Find<CharClip>(MakeString("%s_in", str.c_str()), false);
                k2--;
                str = k2->value;
                str.ReplaceAll('*', '\0');
                mOutClip =
                    mClipDir->Find<CharClip>(MakeString("%s_out", str.c_str()), false);
                mRestClip = mClipDir->Find<CharClip>("rest", false);
                for (int i = 0; i < 4; i++) {
                    mRestStepClips[i] =
                        mClipDir->Find<CharClip>(sRestStepNames[i], false);
                }
            }
            return true;
        }
    }
    return false;
}

bool ClipPlayer::Init(Difficulty d) {
    return Init(TheHamDirector->GetPropAnim(d, "song.anim", false));
}

bool ClipPlayer::Init(int x) { return Init(TheHamDirector->SongAnim(x)); }

bool ClipPlayer::CanUseRestStep() {
    CharClip *c;
    return (TheLoadMgr.EditMode() && TheHamDirector->NoTransitions())
        || (c = mOutClip, !(c && (ClipLength(c) != 3 || c->Flags() & 4)));
}

void ClipPlayer::PlayAnims(HamCharacter *c, float f1, float f2, int x) {
    unk48 = x;
    unkc = FrameToBeat(f1);
    unk10 = FrameToBeat(f2);
    unk1c = c->SongDriver();
    unk44 = 0;
    unk1c->Clear();
    HamRegulate *reg = c->Regulator();
    PlayNormal(-kHugeFloat, nullptr, "");
    reg->RegulateWay(c->GetWaypoint(), 8);
}

void ClipPlayer::PlayClip(CharClip *clip, float f1, float f2, HamDriver::LayerArray *arr) {
    if (clip) {
        float f50, f4c;
        ClipStart(clip, f1, f50, f4c);
        unk44++;
        if (!TheLoadMgr.EditMode() || (unk48 <= 0 || unk44 == unk48)) {
            HamDriver::LayerClip *layerClip = unk1c->NewLayerClip();
            layerClip->unk10 = clip;
            layerClip->unkc = f50 - unk50;
            layerClip->unk4 = f2 - unk50;
            arr->unk2c.push_front(layerClip);
            if (TheLoadMgr.EditMode() && unk48 > 0) {
                layerClip->unk4 = -kHugeFloat;
            }
        }
    }
}

bool ClipPlayer::PushExpertClip(int i1, HamDriver::LayerArray *arr) {
    if (i1 < 0)
        return false;
    else {
        Key<Symbol> &curKey = mClipKeys->at(i1);
        float beat = FrameToBeat(curKey.frame);
        bool b2 = false;
        if (unkc < beat + 1.0f) {
            b2 = PushExpertClip(i1 - 1, arr);
        }
        float f6 = b2 ? beat : -kHugeFloat;
        PlayClip(mClipDir->Find<CharClip>(curKey.value.Str(), false), beat, f6, arr);
        return true;
    }
}

CharClip *ClipPlayer::GetTransitionBefore(Key<Symbol> *key) {
    if (key) {
        if (key != &mClipKeys->at(0)
            && (!TheLoadMgr.EditMode() || !TheHamDirector->NoTransitions())) {
            char name[256];
            strcpy(name, (key - 1)->value.Str());
            strcat(name, "_");
            strcat(name, key->value.Str());
            return mClipDir->Find<CharClip>(name, false);
        }
    }
    return nullptr;
}

CharClip *ClipPlayer::GetRoutineTransition(const char *cc, Key<Symbol> *key) {
    if (key) {
        if (key != &mClipKeys->at(0)
            && (!TheLoadMgr.EditMode() || !TheHamDirector->NoTransitions())) {
            char name[256];
            strcpy(name, cc);
            strcat(name, "_");
            strcat(name, key->value.Str());
            return mClipDir->Find<CharClip>(name, false);
        }
    }
    return nullptr;
}

void ClipPlayer::GetRoutineCrossoverClips(
    float f1, const char *cc, CharClip **c1, CharClip **c2
) {
    if (TheMoveMgr->HasRoutine()) {
        const std::pair<const MoveVariant *, const MoveVariant *> *moveVars =
            TheMoveMgr->GetRoutineMeasure(unk14, Round(f1 / 4.0f));
        if (moveVars) {
            if (moveVars->first) {
                *c1 = mClipDir->Find<CharClip>(moveVars->first->Name().Str(), false);
            }
            if (moveVars->second) {
                *c2 = mClipDir->Find<CharClip>(moveVars->second->Name().Str(), false);
            }
        }
    }
    if (!*c1) {
        *c1 = *c2;
        if (!*c1) {
            *c1 = mClipDir->Find<CharClip>(cc, false);
            if (!*c1) {
                *c1 = mClipDir->Find<CharClip>(mMasterClipKeys->at(0).value.Str(), false);
            }
        }
    }
    if (!*c2) {
        *c2 = *c1;
    }
}

void ClipPlayer::PlayNormal(float f1, HamDriver::LayerArray *arr, const char *cc) {
    HamDriver::LayerArray *newArr;
    if (arr) {
        newArr = new HamDriver::LayerArray();
        arr->unk2c.push_front(newArr);
        strncpy(newArr->unkc, cc, 0x1F);
    } else {
        newArr = &unk1c->Layers();
    }
    newArr->unk4 = f1 - unk50;
    if (!mClipKeys) {
        if (TheLoadMgr.EditMode()) {
            {
                static DebugNotifyOncer _dw;
                _dw << MakeStringNotInlined(
                    "No 'clips' keyframes in your song.anim.  Please don't save this song!"
                );
            }
        }
    } else {
        static Symbol merge_moves("merge_moves");
        if (TheHamProvider->Property(merge_moves, true)->Int() != 0) {
            int keyIdx = mClipKeys->KeyLessEq(BeatToFrame(unkc));
            PushRoutineBuilderClip(keyIdx, newArr);
        } else if (mClipKeys == mMasterClipKeys) {
            PushExpertClip(mClipKeys->KeyLessEq(BeatToFrame(unkc)), newArr);
        } else {
            PushClip(mClipKeys->KeyGreaterEq(BeatToFrame(unkc)), newArr);
        }
    }
}

float ClipPlayer::ClipLength(CharClip *clip) {
    float end = floor(clip->EndBeat());
    float start = ceil(clip->StartBeat());
    return end - start;
}

bool ClipPlayer::GetClipRange(
    const char *c1, const char *c2, float f1, float &f2, float &f3, float &f4
) {
    auto clip = mClipDir->Find<CharClip>(c1, false);
    if (clip) {
        float clipStart = ClipStart(clip, f1, f2, f3);
        f4 = 1e+30;
        auto clip2 = mClipDir->Find<CharClip>(c2, false);
        if (clip2 != nullptr) {
            auto node = clip->FindLastNode(clip2, clipStart);
            if (node != nullptr) {
                f4 = (node->curBeat - clip->StartBeat()) + f2;
            }
        }
        return true;
    }
    return false;
}

CharClip *ClipPlayer::GetPrevRoutineTransition(int idx) {
    if (idx <= 0) {
        return nullptr;
    } else {
        MinEq<int>(idx, mClipKeys->size() - 1);
        if (TheLoadMgr.EditMode() && TheHamDirector->NoTransitions()) {
            return nullptr;
        } else {
            Key<Symbol> &key = mClipKeys->at(idx);
            Key<Symbol> &keyPrev = mClipKeys->at(idx - 1);

            float f8 = FrameToBeat(keyPrev.frame);
            if (f8 > 0) {
                f8 += 1;
            }
            CharClip *c1 = nullptr;
            CharClip *c2 = nullptr;
            GetRoutineCrossoverClips(f8, keyPrev.value.Str(), &c1, &c2);
            return GetRoutineTransition(c2->Name(), &key);
        }
    }
}
