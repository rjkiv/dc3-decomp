#include "char/CharDriver.h"
#include "char/Char.h"
#include "char/CharClip.h"
#include "macros.h"
#include "obj/Object.h"
#include "utl/Symbol.h"
#include "obj/Utl.h"

CharDriver::CharDriver()
    : mBones(this), mClips(this), mFirst(), unk5c(this), mDefaultClip(this), unk84(this),
      unk98(false), mOldBeat(1e+30), mRealign(false), mBeatScale(1.0f), mBlendWidth(1.0f),
      mApply(kApplyBlend), mInternalBones(), mPlayMultipleClips(false) {}

CharDriver::~CharDriver() {
    if (mFirst)
        mFirst->DeleteStack();
    delete mInternalBones;
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

void CharDriver::Highlight() {
#ifdef MILO_DEBUG
    if (gCharHighlightY == -1.0f)
        CharDeferHighlight(this);
    else
        gCharHighlightY = Display(gCharHighlightY);
#endif
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

CharClipDriver *
CharDriver::Play(const DataNode &node, int i, float f1, float f2, float f3) {
    DataNode thisnode(node);
    CharClip *found = FindClip(node, true);
    CharClipDriver *driver = Play(found, i, f1, f2, f3);
    mLastNode = thisnode;
    return driver;
}

DataNode CharDriver::OnSetDefaultClip(DataArray *arr) {
    if (mClips) {
        mDefaultClip = FindClip(arr->Str(2), true);
    }
    return mDefaultClip.Ptr();
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
    unk84 = grp;
    return Play(grp->GetClip(0), i, f1, f2, f3);
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
    float ret = 1.0f;
    float f1 = 0.0f;
    for (auto it = mFirst; it != nullptr; it = it->Next()) {
        float temp = EaseSigmoid(it->mBlendFrac, 0.0f, 0.0f);
        if ((it->mClip->Flags() & i) != 0) {
            ret += temp * f1;
        }
        f1 *= 1.0f - temp;
    }
    return ret;
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
    bs << unk5c;
    bs << unk98;

END_SAVES