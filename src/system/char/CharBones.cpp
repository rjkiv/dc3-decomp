#include "char/CharBones.h"
#include "char/CharClip.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

CharBones *gPropBones;

void CharBones::Zero() { memset(mStart, 0, mTotalSize); }

int CharBones::TypeSize(int i) const {
    switch (i) {
    case TYPE_POS:
    case TYPE_SCALE:
        if (mCompression >= kCompressVects)
            return 6;
        else
            return sizeof(Vector3);
    case TYPE_QUAT:
        if (mCompression >= kCompressQuats)
            return 4;
        else if (mCompression != kCompressNone)
            return 8;
        else
            return sizeof(Hmx::Quat);

    default:
        if (mCompression != kCompressNone)
            return 2;
        else
            return 4;
    }
}

void CharBones::RecomputeSizes() {
    mOffsets[TYPE_POS] = 0;
    for (int i = 0; i < NUM_TYPES; i++) {
        int diff = mCounts[i + 1] - mCounts[i];
        mOffsets[i + 1] = mOffsets[i] + diff * TypeSize(i);
    }
    mTotalSize = mOffsets[TYPE_END] + 0xFU & 0xFFFFFFF0; // round up to the nearest 0x10,
                                                         // alignment moment
}

void CharBones::SetCompression(CompressionType ty) {
    if (ty != mCompression) {
        mCompression = ty;
        RecomputeSizes();
    }
}

CharBones::Type CharBones::TypeOf(Symbol s) {
    const char *p = s.Str();
    char c;
    while ((c = *p++) != 0) {
        if (c == '.') {
            switch (*p) {
            case 'p':
                return TYPE_POS;
            case 's':
                return TYPE_SCALE;
            case 'q':
                return TYPE_QUAT;
            case 'r': {
                // check if rot is x, y, or z
                char next = p[2];
                if (next >= 'x' && next <= 'z')
                    return (Type)(next - 'u');
            }
            default:
                break;
            }
        }
    }
    MILO_FAIL("Unknown bone suffix in %s", s);
    return NUM_TYPES;
}

const char *CharBones::SuffixOf(CharBones::Type t) {
    static const char *suffixes[NUM_TYPES] = { "pos",  "scale", "quat",
                                               "rotx", "roty",  "rotz" };
    MILO_ASSERT(t < TYPE_END, 0x66);
    return suffixes[t];
}

Symbol CharBones::ChannelName(const char *cc, CharBones::Type t) {
    MILO_ASSERT(t < TYPE_END, 0x6F);
    char buf[256];
    strcpy(buf, cc);
    char *chr = strchr(buf, '.');
    if (!chr) {
        chr = buf + strlen(buf);
        *chr = '.';
    }
    strcpy(chr + 1, SuffixOf(t));
    return Symbol(buf);
}

int CharBones::FindOffset(Symbol s) const {
    Type ty = TypeOf(s);
    int nextcount = mCounts[ty + 1];
    int size = TypeSize(ty);
    int count = mCounts[ty];
    int offset = mOffsets[ty];
    for (int i = 0; i < nextcount - count; i++) {
        if (mBones[count << 3].name == s)
            return offset;
        else
            offset += size;
    }
    return -1;
}

void CharBones::SetWeights(float wt, std::vector<Bone> &bones) {
    for (int i = 0; i < bones.size(); i++) {
        bones[i].weight = wt;
    }
}

void *CharBones::FindPtr(Symbol s) const {
    int offset = FindOffset(s);
    if (offset == -1)
        return 0;
    else
        return (void *)&mStart[offset];
}

void CharBones::Print() {
    for (std::vector<Bone>::iterator it = mBones.begin(); it != mBones.end(); ++it) {
        MILO_LOG("%s %.2f: %s\n", it->name, it->weight, StringVal(it->name));
    }
}

BinStream &operator<<(BinStream &bs, const CharBones::Bone &bone) {
    bs << bone.name;
    bs << bone.weight;
    return bs;
}

BinStream &operator>>(BinStream &bs, CharBones::Bone &bone) {
    bs >> bone.name;
    bs >> bone.weight;
    return bs;
}

void CharBones::SetWeights(float f) { SetWeights(f, mBones); }

BEGIN_CUSTOM_PROPSYNC(CharBones::Bone)
    SYNC_PROP(name, o.name)
    SYNC_PROP(weight, o.weight)
    SYNC_PROP_SET(preview_val, gPropBones->StringVal(o.name), )
END_CUSTOM_PROPSYNC

void CharBones::ListBones(std::list<Bone> &bones) const {
    for (int i = 0; i < mBones.size(); i++) {
        bones.push_back(mBones[i]);
    }
}

void CharBones::ClearBones() {
    mBones.clear();
    for (int i = 0; i < NUM_TYPES; i++) {
        mCounts[i] = 0;
        mOffsets[i] = 0;
    }
    mTotalSize = 0;
    mCompression = kCompressNone;
    ReallocateInternal();
}

void TestDstComplain(Symbol s) {
    MILO_NOTIFY_ONCE("src %s not in dst, punting animation", s);
}

CharBones::CharBones() : mCompression(kCompressNone), mStart(0), mTotalSize(0) {
    for (int i = 0; i < NUM_TYPES; i++) {
        mCounts[i] = 0;
        mOffsets[i] = 0;
    }
}

BEGIN_PROPSYNCS(CharBonesObject)
    gPropBones = this;
    SYNC_PROP(bones, mBones)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void CharBones::ScaleAdd(CharClip *clip, float f1, float f2, float f3) {
    clip->ScaleAdd(*this, f1, f2, f3);
}

void CharBonesAlloc::ReallocateInternal() {
    MemFree(mStart);
    mStart = (char *)MemAlloc(mTotalSize, __FILE__, 0x6C0, "CharBones");
}
