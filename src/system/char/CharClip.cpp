#include "char/CharClip.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

CharClip::CharClip()
    : mTransitions(this), mFramesPerSec(30), mFlags(0), mPlayFlags(0), mRange(0),
      mRelative(this), mDirty(true), mOldVer(-1), mDoNotCompress(false), mSyncAnim(this),
      unk198(0) {
    mBeatTrack.resize(1);
    mBeatTrack[0].frame = 0;
    mBeatTrack[0].value = 0;
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

BEGIN_CUSTOM_PROPSYNC(CharGraphNode)
    SYNC_PROP(cur_beat, o.curBeat)
    SYNC_PROP(next_beat, o.nextBeat)
END_CUSTOM_PROPSYNC

bool PropSyncArray(
    CharClip::NodeVector &o, DataNode &val, DataArray *prop, int i, PropOp op
) {
    if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize, 0x720);
        val = o.size;
        return true;
    } else {
        CharGraphNode &node = o.nodes[prop->Int(i++)];
        if (i < prop->Size() || op & kPropGet) {
            return PropSync(node, val, prop, i, op);
        } else
            return false;
    }
}

BEGIN_CUSTOM_PROPSYNC(CharClip::NodeVector)
    SYNC_PROP_SET(clip, o.clip.Ptr(), ) {
        static Symbol _s("nodes");
        if (sym == _s) {
            PropSyncArray(o, _val, _prop, _i + 1, _op);
            return true;
        }
    }
END_CUSTOM_PROPSYNC

bool PropSync(
    CharClip ::Transitions &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op
) {
    if (_i == _prop->Size()) {
        MILO_ASSERT(_op == kPropSize, 0x73B);
        _val = o.Size();
        return true;
    } else {
        CharClip::NodeVector &vec = *o.GetNodes(_prop->Int(_i++));
        if (_i < _prop->Size() || _op & (kPropSize | kPropGet)) {
            return PropSync(vec, _val, _prop, _i, _op);
        } else
            return false;
    }
}

BEGIN_CUSTOM_PROPSYNC(CharClip::BeatEvent)
    SYNC_PROP_SET(beat, o.beat, o.beat = _val.Float())
    SYNC_PROP_SET(event, o.event, o.event = _val.Sym())
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

void CharClip::Transitions::Clear() {
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        it->clip->~CharClip(); // scalar deleting dtor gets called here
    }
    Resize(0, nullptr);
}

int CharClip::Transitions::Size() const {
    int size = 0;
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        size++;
    }
    return size;
}

CharClip::NodeVector *CharClip::Transitions::GetNodes(int idx) const {
    NodeVector *ret = mNodeStart;
    for (; idx > 0; idx--)
        ret = ret->Next();
    return ret;
}

CharClip::NodeVector *CharClip::Transitions::FindNodes(CharClip *clip) const {
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        if (it->clip == clip)
            return it;
    }
    return nullptr;
}

void CharClip::Transitions::RemoveClip(CharClip *clip) {
    NodeVector *it;
    for (it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        if (it->clip == clip) {
            goto uhm_ackshually;
        }
    }
    it = nullptr;
uhm_ackshually:
    if (it)
        RemoveNodes(it);
}

bool CharClip::Transitions::Replace(ObjRef *ref, Hmx::Object *obj) {
    NodeVector *vector = reinterpret_cast<NodeVector *>(ref); // i guess?
    if (!vector->clip.SetObj(obj)) {
        RemoveNodes(vector);
    }
    return true;
}

void CharClip::Transitions::RemoveNodes(NodeVector *n) {
    MILO_ASSERT(n, 0xEC);
    NodeVector *next = n->Next();
    memmove(n, next, (int)mNodeEnd - (int)next);
    Resize(BytesInMemory() - ((int)next - (int)n), nullptr);
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        it->clip->Release(nullptr);
    }
}

void CharClip::Transitions::Save(BinStream &bs) {
    int total_size = 0;
    int num_nodes = 0;
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        num_nodes++;
        total_size += it->size;
    }
    bs << total_size;
    bs << num_nodes;
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        bs << it->clip->Name();
        bs << it->size;
        for (int i = 0; i < it->size; i++) {
            bs << it->nodes[i].curBeat;
            bs << it->nodes[i].nextBeat;
        }
    }
}

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
