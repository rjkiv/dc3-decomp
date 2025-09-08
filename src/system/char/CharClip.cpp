#include "char/CharClip.h"
#include "obj/Object.h"

CharClip::CharClip()
    : mTransitions(this), mFramesPerSec(30), mFlags(0), mPlayFlags(0), mRange(0),
      mRelative(this), mDirty(true), mOldVer(-1), mDoNotCompress(false), mSyncAnim(this),
      unk198(0) {
    mBeatTrack.resize(1);
    mBeatTrack[0].frame = 0.0f;
    mBeatTrack[0].value = 0.0f;
}

CharClip::~CharClip() {}

BEGIN_HANDLERS(CharClip)
    HANDLE_EXPR(in_groups, InGroups())
    HANDLE(groups, OnGroups)
    HANDLE_EXPR(shares_groups, SharesGroups(_msg->Obj<CharClip>(2)))
    HANDLE(has_group, OnHasGroup)
    HANDLE_EXPR(get_clip_events, GetClipEvents())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool PropSync(
    CharClip ::Transitions &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op
);

BEGIN_CUSTOM_PROPSYNC(CharClip::BeatEvent)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharClip)
    SYNC_PROP_SET(start_beat, StartBeat(), )
    SYNC_PROP_SET(end_beat, EndBeat(), )
    SYNC_PROP_SET(length_beats, LengthBeats(), )
    SYNC_PROP_SET(frames_per_sec, mFramesPerSec, )
    SYNC_PROP_SET(length_seconds, LengthSeconds(), )
    SYNC_PROP_SET(average_beats_per_sec, AverageBeatsPerSecond(), )
    SYNC_PROP_SET(flags, mFlags, SetFlags(_val.Int()))
    SYNC_PROP_SET(default_blend, mPlayFlags & 0xF, SetDefaultBlend(_val.Int()))
    SYNC_PROP_SET(default_loop, mPlayFlags & 0xF0, SetDefaultLoop(_val.Int()))
    SYNC_PROP_SET(beat_align, mPlayFlags & 0xF600, SetBeatAlignMode(_val.Int()))
    SYNC_PROP(range, mRange)
    SYNC_PROP_SET(relative, mRelative.Ptr(), SetRelative(_val.Obj<CharClip>()))
    SYNC_PROP_MODIFY(events, mBeatEvents, SortEvents())
    SYNC_PROP_SET(dirty, mDirty, )
    SYNC_PROP_SET(size, AllocSize(), )
    SYNC_PROP(do_not_compress, mDoNotCompress)
    SYNC_PROP(transitions, mTransitions)
    SYNC_MEMBER(full, mFull)
    SYNC_MEMBER(one, mOne)
    SYNC_PROP_SET(compression, mFull.GetCompression(), )
    SYNC_PROP_SET(num_frames, NumFrames(), )
    SYNC_PROP(sync_anim, mSyncAnim)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void CharClip::Init() { REGISTER_OBJ_FACTORY(CharClip); }

void CharClip::SetPlayFlags(int i) {
    if (i != mPlayFlags) {
        mPlayFlags = i;
        mDirty = true;
    }
}

void CharClip::SetFlags(int i) {
    if (i != mFlags) {
        mFlags = i;
        mDirty = true;
    }
}

void CharClip::SetDefaultBlend(int blend) {
    int flags = mPlayFlags;
    SetDefaultBlendFlag(flags, blend);
    SetPlayFlags(flags);
}

void CharClip::SetDefaultLoop(int loop) {
    int flags = mPlayFlags;
    SetDefaultLoopFlag(flags, loop);
    SetPlayFlags(flags);
}

void CharClip::SetBeatAlignMode(int align) {
    int flags = mPlayFlags;
    SetDefaultBeatAlignModeFlag(flags, align);
    SetPlayFlags(flags);
}

struct SortByFrame {
    bool operator()(const CharClip::BeatEvent &e1, const CharClip::BeatEvent &e2) const {
        return e1.beat < e2.beat ? true : false;
    }
};

void CharClip::SortEvents() {
    std::sort(mBeatEvents.begin(), mBeatEvents.end(), SortByFrame());
}
