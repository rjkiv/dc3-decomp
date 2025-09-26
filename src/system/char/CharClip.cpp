#include "char/CharClip.h"
#include "CharClipGroup.h"
#include "char/CharBones.h"
#include "char/CharBonesMeshes.h"
#include "char/CharBonesSamples.h"
#include "math/Rot.h"
#include "math/Trig.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
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
    MemHeapTracker tmp(_x);
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
    MemHeapTracker temp(_x);
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

CharClip::NodeVector *CharClip::Transitions::Resize(int size, const NodeVector *old) {
    static int _x = MemFindHeap("char");
    MemHeapTracker temp(_x);
    int n = (int)old - (int)mNodeStart;
    MILO_ASSERT((old == NULL) || (n >= 0), 0x9B);
    if (size != BytesInMemory()) {
        if (size == 0) {
            MemFree(mNodeStart);
            mNodeStart = nullptr;
            MILO_ASSERT(old == NULL, 0xA8);
        } else if (size < BytesInMemory()) {
            mNodeStart = (NodeVector *)MemTruncate(mNodeStart, size);
        } else {
            mNodeStart = (NodeVector *)MemRealloc(
                mNodeStart, size, __FILE__, 0xB0, "CharGraphNode", 0
            );
        }
    }
    mNodeEnd = mNodeStart + size;
    return mNodeStart + n;
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

int CharClip::GetContext() const {
    if (TypeDef()) {
        DataArray *found = TypeDef()->FindArray("resource", false);
        if (found) {
            return DataGetMacro(found->Str(2))->Int(0);
        }
    }
    return 0;
}

const CharGraphNode *CharClip::FindFirstNode(CharClip *clip, float beat) const {
    NodeVector *nodes = mTransitions.FindNodes(clip);
    if (nodes) {
        for (int i = 0; i < nodes->size; i++) {
            if (nodes->nodes[i].curBeat >= beat)
                return &nodes->nodes[i];
        }
    }
    return nullptr;
}

const CharGraphNode *CharClip::FindLastNode(CharClip *clip, float beat) const {
    NodeVector *nodes = mTransitions.FindNodes(clip);
    if (nodes) {
        for (int i = nodes->size - 1; i >= 0; i--) {
            if (nodes->nodes[i].curBeat >= beat)
                return &nodes->nodes[i];
        }
    }
    return nullptr;
}

void CharClip::EvaluateChannel(void *v1, const void *v2, int iii, float f) {
    if (!v2) {
        MILO_FAIL("%s passed in NULL for evaluate channel", PathName(this));
    }
    int i3 = (int)v2 - 1;
    if (i3 < mFull.TotalSize()) {
        mFull.EvaluateChannel(v1, i3, iii, f);
    } else {
        int i2 = i3 - mFull.TotalSize();
        if (i2 < mOne.TotalSize()) {
            mOne.EvaluateChannel(v1, i2, 0, 0);
        } else {
            MILO_FAIL("%s could not find offset %d %d", i3, i2, PathName(this));
        }
    }
}

void CharClip::ScaleAddSample(
    CharBones &bones, float f1, int i1, float f2, int i2, float f3
) {
    mFull.ScaleAddSample(bones, f1, i1, f2);
    mOne.ScaleAddSample(bones, f1, 0, 0);
    mFacing.ScaleAddSample(mFull, bones, f1, i1, f2, i2, f3);
}

void CharClip::Relativize() {
    if (mFull.GetCompression() != CharBones::kCompressNone) {
        MILO_NOTIFY("%s relativizing compressed clip, should reexport", PathName(this));
    }
    MILO_ASSERT(mRelative, 0x3A3);
    mFull.Relativize(mRelative);
    mOne.Relativize(mRelative);
}

int CharClip::TransitionVersion() {
    int version = -1;
    if (!Type().Null()) {
        const DataNode *prop = Property("transition_version", false);
        if (prop)
            version = prop->Int();
    }
    return version;
}

const CharGraphNode *
CharClip::FindNode(CharClip *clip, float f1, int iii, float f2) const {
    const CharGraphNode *n = nullptr;
    int blendMode = iii & 0xF;
    switch (blendMode) {
    case kPlayNoDefault:
        break;
    case kPlayNow:
        break;
    case kPlayDirty:
        break;
    case kPlayNoBlend:
        n = nullptr;
        break;
    case kPlayFirst:
        n = FindFirstNode(clip, f1);
        break;
    case kPlayLast:
        n = FindLastNode(clip, f1);
        break;
    default:
        MILO_NOTIFY("Unknown mode flags %x, default to kPlayNow", iii);
        break;
    }
    if (!n) {
        static CharGraphNode node;
        node.curBeat = f1;
        if (blendMode == kPlayLast) {
            MaxEq(node.curBeat, EndBeat() - f2 * 0.5f);
        }
        n = &node;
        node.nextBeat = StartBeat();
    }
    return n;
}

float CharClip::LengthSeconds() const {
    if (NumFrames() < 2)
        return 0;
    else
        return (NumFrames() - 1) / mFramesPerSec;
}

float CharClip::AverageBeatsPerSecond() const {
    if (LengthSeconds()) {
        return LengthBeats() / LengthSeconds();
    } else
        return 1;
}

float CharClip::FrameToBeat(float frame) const {
    float ret = 0;
    mBeatTrack.Linear(frame, ret);
    return ret;
}

float CharClip::BeatToFrame(float beat) const {
    float ret = 0;
    mBeatTrack.ReverseLinear(beat, ret);
    return ret;
}

float CharClip::DeltaSecondsToDeltaBeat(float f1, float beat) {
    if (mBeatTrack.size() == 1)
        return f1;
    else {
        float ret = FrameToBeat(f1 * mBeatTrack.front().value + BeatToFrame(beat));
        ret -= beat;
        return ret;
    }
}

int CharClip::BeatToSample(float f, float *fp) const {
    float frame = BeatToFrame(f);
    float f1 = 0;
    if (mBeatTrack.back().frame != 0) {
        *fp = frame / mBeatTrack.back().frame;
    }
    *fp = f1;
    return mFull.FracToSample(fp);
}

void CharClip::EvaluateChannel(void *v1, const void *v2, float f3) {
    float fp;
    int sample = BeatToSample(f3, &fp);
    EvaluateChannel(v1, v2, sample, fp);
}

void CharClip::RotateBy(CharBones &bones, float f) {
    float frac;
    int samp = BeatToSample(f, &frac);
    MILO_ASSERT(frac == 0, 0x36E);
    mFull.RotateBy(bones, samp);
    mOne.RotateBy(bones, 0);
}

void CharClip::RotateTo(CharBones &bones, float f1, float f2) {
    float fp;
    int sample = BeatToSample(f2, &fp);
    mFull.RotateTo(bones, f1, sample, fp);
    mOne.RotateTo(bones, f1, 0, 0);
}

void CharClip::ScaleAdd(CharBones &bones, float f1, float f2, float f3) {
    float fp;
    float fp2;
    int samp1 = BeatToSample(f2, &fp);
    int samp2 = BeatToSample(f2 - f3, &fp2);
    ScaleAddSample(bones, f1, samp1, fp, samp2, fp2);
}

void CharClip::SetRelative(CharClip *clip) {
    if (clip != mRelative) {
        if (clip == this) {
            MILO_NOTIFY("%s cannot be relative to itself", PathName(this));
        } else {
            mRelative = clip;
            if (mRelative)
                Relativize();
            else
                MILO_NOTIFY("%s cannot de-relativize clip, must reexport", PathName(this));
        }
    }
}

void CharClip::ListBones(std::list<CharBones::Bone> &bones) {
    mFull.ListBones(bones);
    mOne.ListBones(bones);
    mFacing.ListBones(bones);
    for (int i = 0; i < mZeros.size(); i++) {
        bones.push_back(mZeros[i]);
    }
}

void CharClip::StuffBones(CharBones &bones) {
    std::list<CharBones::Bone> blist;
    ListBones(blist);
    bones.AddBones(blist);
}

void CharClip::PoseMeshes(ObjectDir *dir, float f) {
    CharBonesMeshes meshes;
    meshes.SetName("tmp_viseme_bones", dir);
    StuffBones(meshes);
    ScaleDown(meshes, 0.0f);
    ScaleAdd(meshes, 1.0f, f, 0.0f);
    meshes.PoseMeshes();
}

DataNode CharClip::GetClipEvents() {
    static Symbol events("events");
    static DataArray *cfg = SystemConfig("objects", "CharClip");
    DataNode ret = 0;
    DataArray *clipArr = cfg->FindArray(events, false);
    if (clipArr) {
        ret = clipArr->Array(1);
    } else {
        DataArray *arr = new DataArray(1);
        arr->Node(0) = Symbol();
        ret = arr;
    }
    return ret;
}

void CharClip::ApplyBlendedSkeletons(
    CharClip **clips, CharBones &bones, float f1, float f2
) {
    float f60;
    int sample = BeatToSample(f1, &f2);
    float f7 = 0;
    std::map<int, float> &curMap = unk18c[sample];
    float f6 = 1;
    for (std::map<int, float>::iterator it = curMap.begin(); it != curMap.end(); ++it) {
        clips[it->first]->ScaleAdd(bones, (f6 - f60) * it->second * f2, f7, f7);
    }
    if (f7 < f60) {
        std::map<int, float> &nextMap = unk18c[sample + 1];
        for (std::map<int, float>::iterator it = nextMap.begin(); it != nextMap.end();
             ++it) {
            clips[it->first]->ScaleAdd(bones, f60 * it->second * f2, f7, f7);
        }
    }
}

bool CharClip::SharesGroups(CharClip *clip) {
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        Hmx::Object *owner = it->RefOwner();
        CharClipGroup *group = dynamic_cast<CharClipGroup *>(owner);
        if (group && group->HasClip(clip))
            return true;
    }
    return false;
}

int CharClip::InGroups() {
    int num = 0;
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        Hmx::Object *owner = it->RefOwner();
        CharClipGroup *group = dynamic_cast<CharClipGroup *>(owner);
        if (group)
            num++;
    }
    return num;
}

DataNode CharClip::OnGroups(DataArray *) {
    DataArray *groups = new DataArray(0);
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        Hmx::Object *owner = it->RefOwner();
        CharClipGroup *group = dynamic_cast<CharClipGroup *>(owner);
        if (group) {
            groups->Insert(groups->Size(), group);
        }
    }
    DataNode ret(groups);
    groups->Release();
    return ret;
}

DataNode CharClip::OnHasGroup(DataArray *arr) {
    const char *str = arr->Str(2);
    for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
        Hmx::Object *owner = it->RefOwner();
        CharClipGroup *group = dynamic_cast<CharClipGroup *>(owner);
        if (group && streq(group->Name(), str))
            return 1;
    }
    return 0;
}
