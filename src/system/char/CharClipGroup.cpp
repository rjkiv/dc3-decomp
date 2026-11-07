#include "char/CharClipGroup.h"
#include "CharClipGroup.h"
#include "char/CharClip.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include <cstring>

CharClipGroup::CharClipGroup()
    : mClips(this, (EraseMode)1), mWhich(0), mQueueStart(0), mFlags(0) {}

BEGIN_HANDLERS(CharClipGroup)
    HANDLE_EXPR(get_clip, GetClip(0))
    HANDLE_ACTION(delete_remaining, DeleteRemaining(_msg->Int(2)))
    HANDLE_EXPR(get_size, mClips.size())
    HANDLE_EXPR(has_clip, HasClip(_msg->Obj<CharClip>(2)))
    HANDLE_EXPR(find_clip, GetClip(_msg->Int(2)))
    HANDLE_ACTION(add_clip, AddClip(_msg->Obj<CharClip>(2)))
    HANDLE_ACTION(set_clip_flags, SetClipFlags(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharClipGroup)
    SYNC_PROP(clips, mClips)
    SYNC_PROP(flags, mFlags)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharClipGroup)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mClips;
    bs << mWhich;
    bs << mFlags;
END_SAVES

BEGIN_COPYS(CharClipGroup)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharClipGroup)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyFromMax) {
            for (int i = 0; i < c->mClips.size(); i++) {
                CharClip *curClip = (CharClip *)c->mClips[i];
                if (!FindClip(curClip->Name())) {
                    mClips.push_back(ObjOwnerPtr<CharClip>(this, curClip));
                }
            }
        } else
            COPY_MEMBER(mClips)
        COPY_MEMBER(mWhich)
        COPY_MEMBER(mFlags)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(CharClipGroup)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    mClips.Load(d, true, nullptr);
    d >> mWhich;
    mWhich = Max(mWhich, 0);
    if (d.rev > 1) {
        d >> mFlags;
    } else {
        mFlags = 0;
    }
END_LOADS

void CharClipGroup::AddClip(CharClip *clip) {
    if (!HasClip(clip)) {
        mClips.push_back(ObjOwnerPtr<CharClip>(this, clip));
    }
}

bool CharClipGroup::HasClip(CharClip *clip) const {
    auto &end = mClips.end();
    return mClips.find(clip) != end;
}

void CharClipGroup::DeleteRemaining(int i1) {
    CharClip *clips[256];
    MILO_ASSERT(mClips.size() < 256, 0x88);
    for (int i = 0; i < mClips.size(); i++) {
        clips[i] = mClips[i];
    }
    CharClip::LockAndDelete(clips, mClips.size(), i1);
}

CharClip *CharClipGroup::FindClip(const char *clipName) const {
    for (int i = 0; i < mClips.size(); i++) {
        if (streq(clipName, mClips[i]->Name())) {
            return (CharClip *)mClips[i];
        }
    }
    return nullptr;
}

void CharClipGroup::SetClipFlags(int flags) {
    for (int i = 0; i < mClips.size(); i++) {
        CharClip *cur = mClips[i];
        cur->SetFlags(cur->Flags() | flags);
    }
}

void CharClipGroup::Sort() { mClips.sort(Alphabetically()); }

int CharClipGroup::PosMod(int idx) const {
    int next = idx + 1;
    return next - (next < mClips.size() ? 0 : mClips.size());
}

int CharClipGroup::NegMod(int idx) const {
    int next = idx - 1;
    return next - (next < mClips.size() ? 0 : mClips.size());
}

int CharClipGroup::QueueRandom(int start, int end) const {
    int diff = end - start;
    int offset = diff < 0 ? mClips.size() : 0;
    int rand = Rand::sRand.FastInt(0, diff + offset) + start;
    return rand - (rand < mClips.size() ? 0 : mClips.size());
}

CharClip *CharClipGroup::GetClip(int flags) {
    if (mClips.size()) {
        MinEq<int>(mWhich, mClips.size() - 1);
        MinEq<int>(mQueueStart, mClips.size() - 1);
        int oldWhich = mWhich;
        mWhich = PosMod(mWhich);
        int it = mWhich;
        for (; it != mQueueStart; it = PosMod(it)) {
            mClips.swap(it, QueueRandom(it, mQueueStart));
            CharClip *clip = mClips[it];
            if ((clip->Flags() & flags) == flags) {
                mClips.swap(it, mWhich);
                return clip;
            }
        }
        for (; it != oldWhich; it = PosMod(it)) {
            mClips.swap(it, QueueRandom(it, oldWhich));
            CharClip *clip = mClips[it];
            if ((clip->Flags() & flags) == flags) {
                mClips.swap(it, mWhich);
                mClips.swap(it, mQueueStart);
                mQueueStart = PosMod(mQueueStart);
                return clip;
            }
        }
        CharClip *clip = mClips[it];
        if ((clip->Flags() & flags) == flags) {
            mClips.swap(it, mWhich);
            mClips.swap(it, mQueueStart);
            mQueueStart = PosMod(mQueueStart);
            return clip;
        }
    }
    return nullptr;
}
