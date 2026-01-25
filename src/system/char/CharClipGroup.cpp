#include "char/CharClipGroup.h"
#include "CharClipGroup.h"
#include "char/CharClip.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include <cstring>

CharClipGroup::CharClipGroup()
    : mClips(this, (EraseMode)1), mWhich(0), unk24(0), mFlags(0) {}

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

BEGIN_LOADS(CharClipGroup)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    mClips.Load(d.stream, true, nullptr);
    d >> mWhich;
    mWhich = Max(mWhich, 0);
    if (d.rev > 1) {
        d >> mFlags;
    } else {
        mFlags = 0;
    }
END_LOADS

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

void CharClipGroup::AddClip(CharClip *clip) {
    if (!HasClip(clip)) {
        mClips.push_back(ObjOwnerPtr<CharClip>(this, clip));
    }
}

bool CharClipGroup::HasClip(CharClip *clip) const {
    return mClips.find(clip) != mClips.end();
}

int CharClipGroup::QueueRandom(int pos, int end) const {
    int range = (pos >= end ? mClips.size() : 0) + end - pos;
    int result = Rand::sRand.FastInt(0, range) + pos;
    int size = mClips.size();
    if (result >= size) {
        result -= size;
    }
    return result;
}

CharClip *CharClipGroup::GetClip(int flags) {
    int size = mClips.size();
    if (size != 0) {
        // Clamp mWhich to valid range
        if (mWhich > size - 1) {
            mWhich = size - 1;
        }

        // Clamp unk24 to valid range
        if (unk24 > size - 1) {
            unk24 = size - 1;
        }

        int origWhich = mWhich;
        int origUnk24 = unk24;

        // Calculate starting position (next after mWhich, wrapping)
        int pos = mWhich + 1;
        if (pos >= size) {
            pos -= size;
        }
        mWhich = pos;

        // First loop: search from pos to unk24
        if (pos != unk24) {
            do {
                int swapIdx = QueueRandom(pos, unk24);
                mClips.swap(pos, swapIdx);
                CharClip *clip = mClips[pos];
                if ((clip->Flags() & flags) == flags) {
                    // Found matching clip
                    mClips.swap(pos, mWhich);
                    // Update unk24
                    int newUnk24 = unk24 + 1;
                    if (newUnk24 >= size) {
                        newUnk24 -= size;
                    }
                    unk24 = newUnk24;
                    return clip;
                }
                pos = pos + 1;
                if (pos >= size) {
                    pos -= size;
                }
            } while (pos != unk24);
        }

        // Second loop: search from pos to origWhich
        if (pos != origWhich) {
            do {
                int swapIdx = QueueRandom(pos, origWhich);
                mClips.swap(pos, swapIdx);
                CharClip *clip = mClips[pos];
                if ((clip->Flags() & flags) == flags) {
                    // Found matching clip
                    mClips.swap(pos, mWhich);
                    mClips.swap(pos, unk24);
                    // Update unk24
                    int newUnk24 = unk24 + 1;
                    if (newUnk24 >= size) {
                        newUnk24 -= size;
                    }
                    unk24 = newUnk24;
                    return clip;
                }
                pos = pos + 1;
                if (pos >= size) {
                    pos -= size;
                }
            } while (pos != origWhich);
        }

        // Final check at current position
        CharClip *clip = mClips[pos];
        if ((clip->Flags() & flags) == flags) {
            // Found matching clip, update state
            mClips.swap(pos, mWhich);
            mClips.swap(pos, unk24);
            // Update unk24
            int newUnk24 = unk24 + 1;
            if (newUnk24 >= size) {
                newUnk24 -= size;
            }
            unk24 = newUnk24;
            return clip;
        }
    }
    return nullptr;
}
