#include "char/CharDriver.h"
#include "char/Char.h"
#include "char/CharClip.h"
#include "char/CharClipDisplay.h"
#include "char/CharClipDriver.h"
#include "char/CharClipGroup.h"
#include "char/CharWeightable.h"
#include "macros.h"
#include "math/Color.h"
#include "math/Easing.h"
#include "math/Geo.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "utl/FilePath.h"
#include "utl/Symbol.h"
#include "obj/Utl.h"

CharClip *MyFindClip(const DataNode &n, ObjectDir *dir) {
    const DataNode &node = n.Evaluate();
    Hmx::Object *obj;
    if (node.Type() == kDataObject) {
        obj = node.ObjectValue();
    } else {
        MILO_ASSERT(node.Type() == kDataSymbol || node.Type() == kDataString, 0x12C);
        obj = dir->FindObject(node.LiteralStr(), false, true);
    }
    if (!obj) {
        return nullptr;
    } else {
        CharClip *clip = dynamic_cast<CharClip *>(obj);
        if (clip) {
            return clip;
        }
        CharClipGroup *group = dynamic_cast<CharClipGroup *>(obj);
        if (!group) {
            MILO_NOTIFY_ONCE(
                "%s: MyFindClip %s bad object type, not CharClip or CharClipGroup",
                PathName(dir),
                PathName(obj)
            );
            return nullptr;
        } else {
            return group->GetClip(0);
        }
    }
}

CharDriver::CharDriver()
    : mBones(this), mClips(this), mFirst(nullptr), mTestClip(this), mDefaultClip(this),
      mLastPlayedGroup(this), mDefaultPlayStarved(false), mOldBeat(kHugeFloat),
      mRealign(false), mBeatScale(1.0f), mBlendWidth(1.0f), mApply(kApplyBlend),
      mInternalBones(), mPlayMultipleClips(false) {}

CharDriver::~CharDriver() {
    if (mFirst)
        mFirst->DeleteStack();
    delete mInternalBones;
}

bool CharDriver::Replace(ObjRef *from, Hmx::Object *to) {
    bool deleted = false;
    if (mFirst != nullptr) {
        mFirst = mFirst->DeleteRef(from, deleted);
    }
    if (deleted != false) {
        return true;
    }
    return CharWeightable::Replace(from, to);
}

BEGIN_HANDLERS(CharDriver)
    HANDLE(play, OnPlay)
    HANDLE(play_group, OnPlayGroup)
    HANDLE(play_group_flags, OnPlayGroupFlags)
    HANDLE_ACTION(offset, Offset(_msg->Float(2), _msg->Float(_msg->Size() - 1)))
    HANDLE(get_first_playing_flags, OnGetFirstPlayingFlags)
    HANDLE(get_first_flags, OnGetFirstFlags)
    HANDLE_EXPR(first_clip, FirstClip())
    HANDLE_ACTION(set_starved, SetStarved(_msg->Sym(2)))
    HANDLE_ACTION(set_beat_scale, SetBeatScale(_msg->Float(2), true))
    HANDLE_ACTION(transfer, Transfer(*_msg->Obj<CharDriver>(2)))
    HANDLE(print, OnPrint)
    HANDLE(set_default_clip, OnSetDefaultClip)
    HANDLE(set_first_beat_offset, OnSetFirstBeatOffset)
    HANDLE_ACTION(clear, Clear())
    HANDLE(get_clip_or_group_list, OnGetClipOrGroupList)
    HANDLE_EXPR(get_last_played_group, mLastPlayedGroup)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharDriver)
    SYNC_PROP(bones, mBones)
    SYNC_PROP_SET(clips, mClips.Ptr(), SetClips(_val.Obj<ObjectDir>()))
    SYNC_PROP_SET(clip_type, mClipType, SetClipType(_val.Sym()))
    SYNC_PROP(realign, mRealign)
    SYNC_PROP_SET(apply, mApply, SetApply((ApplyMode)_val.Int()))
    SYNC_PROP_SET(first_playing_clip, FirstPlayingClip(), )
    SYNC_PROP(beat_scale, mBeatScale)
    SYNC_PROP(blend_width, mBlendWidth)
    SYNC_PROP(default_clip_or_group, mDefaultClip)
    SYNC_PROP(default_play_starved, mDefaultPlayStarved)
    SYNC_PROP(test_clip, mTestClip)
    SYNC_PROP(play_multiple_clips, mPlayMultipleClips)
    SYNC_PROP(display_zoom, CharClipDisplay::sZoom)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharDriver)
    SAVE_REVS(0xe, 0);
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mBones;
    bs << mClips;
    bs << mBlendWidth;
    bs << mRealign;
    bs << mApply;
    bs << mClipType;
    bs << mPlayMultipleClips;
    bs << mTestClip;
    bs << mDefaultClip;
    bs << mDefaultPlayStarved;
END_SAVES

BEGIN_COPYS(CharDriver)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharDriver)
    BEGIN_COPYING_MEMBERS
        mBones = c->GetBones();
        COPY_MEMBER(mClips)
        COPY_MEMBER(mRealign)
        COPY_MEMBER(mBeatScale)
        COPY_MEMBER(mBlendWidth)
        COPY_MEMBER(mTestClip)
        COPY_MEMBER(mClipType)
        COPY_MEMBER(mApply)
        COPY_MEMBER(mDefaultClip)
        COPY_MEMBER(mDefaultPlayStarved)
        COPY_MEMBER(mPlayMultipleClips)
        SyncInternalBones();
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0xe, 0)

BEGIN_LOADS(CharDriver)
    LOAD_REVS(bs)
    ASSERT_REVS(0xe, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    if (d.rev < 3) {
        int x;
        d >> x;
    }
    d >> mBones;
    if (d.rev < 8) {
        FilePath fp;
        d >> fp;
        if (d.rev > 6 && fp.empty()) {
            d >> mClips;
        }
    } else {
        d >> mClips;
    }
    if (d.rev > 8) {
        d >> mBlendWidth;
    }
    if (d.rev > 1) {
        d >> mRealign;
    } else {
        mRealign = false;
    }
    if (d.rev > 5)
        d >> (int &)mApply;
    else if (d.rev > 4) {
        bool b48;
        d >> b48;
        mApply = (ApplyMode)(b48 != false);
    } else
        mApply = kApplyBlend;
    if (d.rev > 9)
        d >> mClipType;
    if (d.rev > 0xC)
        d >> mPlayMultipleClips;
    if (d.rev <= 9 && mClips) {
        mClipType = mClips->Type();
        if (mClipType.Null()) {
            for (ObjDirItr<CharClip> it(mClips, true); it != nullptr; ++it) {
                mClipType = it->Type();
                break;
            }
        }
    }
    SyncInternalBones();
    if (d.rev > 3) {
        mTestClip.Load(d.stream, false, mClips);
    }
    if (d.rev > 0xB) {
        mDefaultClip.Load(d.stream, false, mClips);
    }
    if (d.rev > 0xD)
        d >> mDefaultPlayStarved;
END_LOADS

void CharDriver::Highlight() {
    if (gCharHighlightY == -1.0f)
        CharDeferHighlight(this);
    else
        gCharHighlightY = Display(gCharHighlightY);
}

void CharDriver::Enter() {
    Clear();
    mLastNode = 0;
    mOldBeat = kHugeFloat;
    mBeatScale = 1.0f;
    RndPollable::Enter();
    if (mDefaultClip)
        Play(DataNode(mDefaultClip), 1, -1.0f, kHugeFloat, 0.0f);
}

void CharDriver::Exit() { RndPollable::Exit(); }

void CharDriver::PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &change) {
    change.push_back(mBones);
}

CharClip *CharDriver::FirstClip() {
    if (mFirst)
        return mFirst->GetClip();
    else
        return nullptr;
}

CharClipDriver *CharDriver::Last() {
    CharClipDriver *d = mFirst;
    while (d && d->Next())
        d = d->Next();
    return d;
}

CharClipDriver *CharDriver::Before(CharClipDriver *driver) {
    CharClipDriver *d = mFirst;
    while (d && d->Next() != driver)
        d = d->Next();
    return d;
}

void CharDriver::Clear() {
    if (mFirst)
        mFirst->DeleteStack();
    mFirst = nullptr;
}

void CharDriver::SetClips(ObjectDir *dir) {
    if (dir != mClips) {
        mLastNode = NULL_OBJ;
        mClips = dir;
    }
}

void CharDriver::SetBones(CharBonesObject *obj) { mBones = obj; }

float CharDriver::TopClipFrame() {
    CharClipDriver *it = mFirst;
    if (!it)
        return 0;
    else {
        while (it->Next())
            it = it->Next();
        if (!it->GetClip())
            return 0;
        else {
            float avg = it->GetClip()->AverageBeatsPerSecond();
            float frame = 0;
            if (avg < 0)
                return frame;
            else
                frame = (it->mBeat - it->GetClip()->StartBeat()) / avg;
            return frame;
        }
    }
}

CharClipDriver *
CharDriver::Play(const DataNode &node, int i, float f1, float f2, float f3) {
    DataNode thisnode(node);
    CharClipDriver *driver = Play(FindClip(node), i, f1, f2, f3);
    mLastNode = thisnode;
    return driver;
}

void CharDriver::Transfer(const CharDriver &driver) {
    Clear();
    mClips = driver.mClips;
    mLastNode = driver.mLastNode;
    mRealign = driver.mRealign;
    mBeatScale = driver.mBeatScale;
    mBlendWidth = driver.mBlendWidth;
    if (driver.mFirst)
        mFirst = new CharClipDriver(this, *driver.mFirst);
}

CharClipDriver *CharDriver::Play(CharClip *clip, int i, float f1, float f2, float f3) {
    if (!clip) {
        MILO_NOTIFY_ONCE("%s: Could not find clip to play.", PathName(this));
        return nullptr;
    } else {
        mLastNode = clip;
        if (f1 == -1.0f)
            f1 = mBlendWidth;
        if (mPlayMultipleClips) {
            for (CharClipDriver *it = mFirst; it != nullptr; it = it->Next()) {
                if (clip == it->GetClip())
                    return nullptr;
            }
        }
        mFirst =
            new CharClipDriver(this, clip, i, f1, mFirst, f2, f3, mPlayMultipleClips);
        return mFirst;
    }
}

CharClipDriver *
CharDriver::PlayGroup(const char *cc, int i, float f1, float f2, float f3) {
    if (!mClips) {
        MILO_NOTIFY("%s has no clips", PathName(this));
        return nullptr;
    } else {
        CharClipGroup *grp = mClips->Find<CharClipGroup>(cc, false);
        if (!grp) {
            MILO_NOTIFY("%s could not find group %s", PathName(this), cc);
            return nullptr;
        } else
            return PlayGroup(grp, i, f1, f2, f3);
    }
}

CharClipDriver *
CharDriver::PlayGroup(CharClipGroup *grp, int i, float f1, float f2, float f3) {
    mLastPlayedGroup = grp;
    CharClip *clip = grp->GetClip(0);
    return Play(clip, i, f1, f2, f3);
}

void CharDriver::SyncInternalBones() {
    Clear();
    mLastNode = NULL_OBJ;
    if (mInternalBones && mClipType.Null()) {
        RELEASE(mInternalBones);
    } else if (!mInternalBones && mApply == kApplyBlendWeights && !mClipType.Null()) {
        mInternalBones = new CharBonesAlloc();
    }
    if (mInternalBones) {
        mInternalBones->ClearBones();
        CharBoneDir::StuffBones(*mInternalBones, mClipType);
    }
}

float CharDriver::EvaluateFlags(int i) {
    float mult = 1;
    float ret = 0;
    for (CharClipDriver *it = mFirst; it != nullptr; it = it->Next()) {
        float sigmoid = EaseSigmoid(it->mBlendFrac, 0, 0);
        if ((it->GetClip()->Flags() & i) != 0) {
            ret += sigmoid * mult;
        }
        mult *= 1 - sigmoid;
    }
    return ret;
}

void CharDriver::SetApply(ApplyMode mode) {
    if (mode != mApply) {
        mApply = mode;
        SyncInternalBones();
    }
}

void CharDriver::SetClipType(Symbol ty) {
    if (mClipType != ty) {
        mClipType = ty;
        SyncInternalBones();
    }
}

void CharDriver::Offset(float f1, float f2) {
    if (mFirst)
        mFirst->mBeat += RandomFloat(f1, f2);
}

void CharDriver::SetStarved(Symbol starved) { mStarvedHandler = starved; }

CharClipDriver *CharDriver::FirstPlaying() {
    CharClipDriver *d;
    for (d = mFirst; d != nullptr && !d->mBlendFrac; d = d->Next())
        ;
    return d;
}

void CharDriver::SetBeatScale(float beatscale, bool) {
    CharClipDriver *playing = FirstPlaying();
    if (playing) {
        float oldbeatscale = mBeatScale / beatscale;
        for (CharClipDriver *d = playing; d != nullptr; d = d->Next()) {
            if ((playing->mPlayFlags & 0xF600) != 0x200) {
                CharClip::SetDefaultBeatAlignModeFlag(d->mPlayFlags, 0);
                d->mTimeScale *= oldbeatscale;
            }
        }
    }
    mBeatScale = beatscale;
}

CharClip *CharDriver::FirstPlayingClip() {
    CharClipDriver *it;
    for (it = mFirst; it != nullptr; it = it->Next()) {
        if (it->mBlendFrac != 0) {
            break;
        }
    }
    if (it != nullptr) {
        return it->GetClip();
    } else {
        return nullptr;
    }
}

void CharDriver::SetClipWeightMap() {
    mClipWeightMap.clear();
    for (CharClipDriver *d = mFirst; d != nullptr; d = d->Next()) {
        CharClip *c = d->GetClip();
        auto it = mClipWeightMap.find(c);
        if (it != mClipWeightMap.end()) {
            it->second += d->mWeight;
        } else {
            mClipWeightMap.insert(std::make_pair(c, d->mWeight));
        }
    }
}

CharClip *CharDriver::FindClip(const DataNode &n, bool notify) {
    if (!mClips) {
        MILO_FAIL("%s: trying to FindClip with no mClips", PathName(this));
    }
    CharClip *clip = MyFindClip(n, mClips);
    if (!clip && notify) {
        String str;
        str << n;
        MILO_NOTIFY_ONCE("%s: missing \"%s\" in %s", PathName(this), str, mClips->Name());
    }
    return clip;
}

float CharDriver::Display(float f1) {
    CharClipDisplay::Init(Dir());
    std::vector<CharClipDisplay> displays;
    for (CharClipDriver *it = mFirst; it != nullptr; it = it->Next()) {
        CharClipDisplay disp;
        displays.push_back(disp);
        displays.back().unk1c = it->mBeat;
        displays.back().SetClip(it->GetClip(), false);
        displays.back().unk20 = it->mBlendFrac;
    }
    float lineSpacing = CharClipDisplay::LineSpacing();
    float f20 = (float)displays.size() * lineSpacing + TheRnd.Height() * f1;
    for (int i = 0; i < displays.size(); i++) {
        displays[i].unk18 = -(i * lineSpacing - f20);
    }
    Hmx::Object *src = CharClipDisplay::FindSource(this);
    TheRnd.DrawRectScreen(
        Hmx::Rect(
            0, f1, 1, ((src ? 1 : 2) * lineSpacing + f20 / (float)TheRnd.Height()) - f1
        ),
        Hmx::Color(0, 0, 0),
        nullptr,
        nullptr,
        nullptr
    );
    float oldBeat = mOldBeat;
    TheRnd.DrawString(
        MakeString("%s %s, beat: %.2f", Dir()->Name(), PathName(this), oldBeat),
        Vector2(CharClipDisplay::GetSEm(), TheRnd.Height() * f1 + lineSpacing * 0.1f),
        Hmx::Color(1, 1, 1),
        true
    );
    for (int i = 0; i < displays.size(); i++) {
        displays[i].DrawTrack();
    }
    for (CharClipDriver *d = mFirst; d != nullptr; d = d->Next()) {
        // ...
    }
    for (int i = 0; i < displays.size(); i++) {
        displays[i].DrawCursor();
    }
    if (src) {
        static Message msg("debug_draw", 2.0f, 2.0f);
        msg[0] = displays[0].unk18 + lineSpacing;
        msg[1] = TheTaskMgr.Beat();
        src->Handle(msg, false);
    }
    return 0;
}

DataNode CharDriver::OnSetFirstBeatOffset(DataArray *msg) {
    if (mFirst) {
        mFirst->SetBeatOffset(msg->Float(2), (TaskUnits)msg->Int(3), msg->Sym(4));
    }
    return 0;
}

DataNode CharDriver::OnPrint(const DataArray *) {
    MILO_LOG("%s\n", PathName(this));
    for (CharClipDriver *it = mFirst; it != nullptr; it = it->Next()) {
        MILO_LOG("   clip %s blend %.3f\n", it->GetClip()->Name(), it->mBlendFrac);
    }
    return 0;
}

DataNode CharDriver::OnSetDefaultClip(DataArray *arr) {
    if (mClips) {
        mDefaultClip = FindClip(arr->Str(2), true);
    }
    return mDefaultClip.Ptr();
}

DataNode CharDriver::OnPlay(const DataArray *msg) {
    int i2 = msg->Size() > 3 ? msg->Int(3) : 4;
    MILO_ASSERT(msg->Size()<=4, 0x39c);
    return Play(msg->Node(2), i2, -1, kHugeFloat, 0) != nullptr;
}

DataNode CharDriver::OnPlayGroup(const DataArray *msg) {
    MILO_ASSERT(msg->Size() <= 4, 0x3a2);
    int i2 = msg->Size() > 3 ? msg->Int(3) : 4;
    return PlayGroup(msg->Str(2), i2, -1, kHugeFloat, 0) != nullptr;
}

DataNode CharDriver::OnPlayGroupFlags(const DataArray *msg) {
    MILO_ASSERT(msg->Size() <= 5, 0x3aa);
    CharClipGroup *group = mClips->Find<CharClipGroup>(msg->Str(2), false);
    if (!group) {
        MILO_NOTIFY("%s could not find group %s", PathName(this), msg->Str(2));
        return 0;
    } else {
        int clipIdx = msg->Int(3);
        int i2 = msg->Size() > 4 ? msg->Int(4) : 4;
        return Play(group->GetClip(clipIdx), i2, -1, kHugeFloat, 0) != nullptr;
    }
}

DataNode CharDriver::OnGetClipOrGroupList(DataArray *) {
    Symbol clipName = "CharClip";
    Symbol clipGrpName = "CharClipGroup";
    std::list<Hmx::Object *> objects;
    if (mClips) {
        for (ObjDirItr<Hmx::Object> it(mClips, true); it != nullptr; ++it) {
            if (IsASubclass(it->ClassName(), clipName)
                || IsASubclass(it->ClassName(), clipGrpName)) {
                objects.push_back(it);
            }
        }
    }
    DataArrayPtr ptr;
    ptr->Resize(objects.size() + 1);
    int idx = 0;
    ptr->Node(idx++) = NULL_OBJ;
    for (std::list<Hmx::Object *>::iterator it = objects.begin(); it != objects.end();
         ++it) {
        ptr->Node(idx++) = *it;
    }
    ptr->SortNodes(0);
    return ptr;
}

DataNode CharDriver::OnGetFirstFlags(const DataArray *) {
    CharClip *clip = FirstPlayingClip();
    return clip ? clip->Flags() : 0;
}

DataNode CharDriver::OnGetFirstPlayingFlags(const DataArray *) {
    CharClip *clip = FirstPlayingClip();
    return clip ? clip->Flags() : 0;
}
