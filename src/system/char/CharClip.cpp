#include "char/CharClip.h"
#include "char/CharBonesSamples.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

const float CharClip::kBeatAccuracy = 0.02;
CharClip::FacingSet::FacingBones CharClip::FacingSet::sFacingPos;
CharClip::FacingSet::FacingBones CharClip::FacingSet::sFacingRotAndPos;

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

BEGIN_SAVES(CharClip)
    SAVE_REVS(22, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mFramesPerSec;
    bs << mFlags;
    bs << mPlayFlags;
    bs << mRange;
    bs << mRelative;
    bs << mOldVer;
    bs << mDoNotCompress;
    mTransitions.Save(bs);
    bs << mBeatEvents.size();
    for (int i = 0; i < mBeatEvents.size(); i++) {
        mBeatEvents[i].Save(bs);
    }
    mFull.Save(bs);
    mOne.Save(bs);
    bs << mZeros;
    bs << mBeatTrack;
    bs << mSyncAnim;
    bs << unk18c;
    bs << unk198;
END_SAVES

BEGIN_COPYS(CharClip)
    static int _x = MemFindHeap("char");
    MemTempHeap tmp(_x);
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFramesPerSec)
        COPY_MEMBER(mBeatTrack)
        if (ty != kCopyFromMax) {
            COPY_MEMBER(mFlags)
            COPY_MEMBER(mPlayFlags)
            COPY_MEMBER(mRange)
            COPY_MEMBER(mRelative)
            mBeatEvents.resize(c->mBeatEvents.size());
            for (int i = 0; i < mBeatEvents.size(); i++) {
                mBeatEvents[i] = c->mBeatEvents[i];
            }
            COPY_MEMBER(mDoNotCompress)
            COPY_MEMBER(mSyncAnim)
        }
        mFull.Clone(c->mFull);
        mOne.Clone(c->mOne);
        COPY_MEMBER(mZeros)
        mFacing.Set(mFull);
        mDirty = true;
        COPY_MEMBER(unk18c)
        COPY_MEMBER(unk198)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharClip)
    static int _x = MemFindHeap("char");
    MemTempHeap temp(_x);
    LOAD_REVS(bs)
    ASSERT_REVS(0x16, 0)
    int oldRev = 0;
    if (gRev < 0x10)
        bs >> oldRev;
    else
        oldRev = 0xD;
    MILO_ASSERT(oldRev > 1, 0x531);
    LOAD_SUPERCLASS(Hmx::Object)
    if (gRev < 0x12) {
        int x, y;
        bs >> x;
        bs >> y;
    }
    bs >> mFramesPerSec;
    bs >> mFlags;
    bs >> mPlayFlags;
    if (oldRev < 0xD) {
        int x;
        bs >> x;
    }
    if (oldRev > 3)
        bs >> mRange;
    if (oldRev > 5) {
        mRelative.Load(bs, false, nullptr);
    } else if (oldRev > 4) {
        bool b117;
        bs >> b117;
        mRelative = b117 ? this : nullptr;
    } else
        mRelative = nullptr;
    // there's more, the usage of both BinStream and BinStreamRev is weird
END_LOADS

void CharClip::PreSave(BinStream &) {
    MILO_NOTIFY("You can only save a CharClip from PC");
}

void CharClip::Print() {
    TheDebug << "CharClip: " << Name() << "\n";
    TheDebug << MakeString("total allocation size %d\n", AllocSize());
    TheDebug << "Full:\n";
    mFull.Print();
    TheDebug << "One:\n";
    mOne.Print();
}

void CharClip::SetTypeDef(DataArray *def) {
    Hmx::Object::SetTypeDef(def);
    mDirty = true;
}

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
    int num_nodes = 0;
    int num_node_vectors = 0;
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        num_node_vectors++;
        num_nodes += it->size;
    }
    bs << num_nodes;
    bs << num_node_vectors;
    for (NodeVector *it = mNodeStart; it < mNodeEnd; it = it->Next()) {
        bs << it->clip->Name();
        bs << it->size;
        for (int i = 0; i < it->size; i++) {
            bs << it->nodes[i].curBeat;
            bs << it->nodes[i].nextBeat;
        }
    }
}

void CharClip::FacingSet::Init() {
    sFacingPos.Set(false);
    sFacingRotAndPos.Set(true);
}

void CharClip::FacingSet::Set(CharBonesSamples &samples) {
    mFacingBones = nullptr;
    mFullRot = -1;
    mFullPos = samples.FindOffset("bone_facing.pos");
    if (mFullPos != -1) {
        mFullRot = samples.FindOffset("bone_facing.rotz");
        mFacingBones = mFullRot == -1 ? &sFacingPos : &sFacingRotAndPos;
    }
}

void CharClip::FacingSet::ListBones(std::list<CharBones::Bone> &bones) {
    if (mFacingBones) {
        mFacingBones->SetWeights(mWeight);
        mFacingBones->ListBones(bones);
    }
}

void CharClip::FacingSet::ScaleAddSample(
    CharBonesSamples &samples,
    CharBones &bones,
    float f1,
    int i1,
    float f2,
    int i2,
    float f3
) {
    if (mFacingBones) {
        Vector3 v;
        samples.EvaluateChannel(&v, mFullPos, i1, f2);
        samples.EvaluateChannel(&mFacingBones->mDeltaPos, mFullPos, i2, f3);
        Subtract(v, mFacingBones->mDeltaPos, mFacingBones->mDeltaPos);
        if (mFullRot != -1) {
            float f64, f68;
            samples.EvaluateChannel(&f64, mFullRot, i1, f2);
            samples.EvaluateChannel(&f68, mFullRot, i2, f3);
            mFacingBones->mDeltaAng = LimitAng(f64 - f68);
            RotateAboutZ(mFacingBones->mDeltaPos, -f68, mFacingBones->mDeltaPos);
        }
        mFacingBones->SetWeights(f1);
        mFacingBones->ScaleAdd(bones, f1);
    }
}

void CharClip::FacingSet::FacingBones::Set(bool b) {
    ClearBones();
    std::list<CharBones::Bone> bones;
    bones.push_back(CharBones::Bone("bone_facing_delta.pos", 1));
    if (b) {
        bones.push_back(CharBones::Bone("bone_facing_delta.rotz", 1));
    }
    AddBones(bones);
}

void CharClip::FacingSet::ScaleDown(CharBones &bones, float f) {
    if (mFacingBones)
        mFacingBones->ScaleDown(bones, f);
}

void CharClip::FacingSet::FacingBones::ReallocateInternal() {
    mStart = (char *)&mDeltaPos;
}

void CharClip::BeatEvent::Save(BinStream &bs) {
    bs << event;
    bs << beat;
}

void CharClip::BeatEvent::Load(BinStream &bs) {
    bs >> event;
    bs >> beat;
}

void CharClip::Init() {
    FacingSet::Init();
    REGISTER_OBJ_FACTORY(CharClip);
}

int CharClip::AllocSize() {
    int size = mTransitions.BytesInMemory();
    size += mFull.AllocateSize() + mOne.AllocateSize();
    size += sizeof(CharClip);
    return size;
}

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
        return e1.beat < e2.beat;
    }
};

void CharClip::SortEvents() {
    std::sort(mBeatEvents.begin(), mBeatEvents.end(), SortByFrame());
}

void *CharClip::GetChannel(Symbol s) {
    int off = mFull.FindOffset(s);
    if (off > -1) {
        return (void *)(off + 1);
    } else {
        off = mOne.FindOffset(s);
        if (off > -1)
            return (void *)(off + mFull.TotalSize() + 1);
        else
            return 0;
    }
}

void CharClip::ScaleDown(CharBones &bones, float f) {
    mFull.ScaleDown(bones, f);
    mOne.ScaleDown(bones, f);
    mFacing.ScaleDown(bones, f);
}
