#include "char/CharBones.h"
#include "char/CharClip.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

CharBones *gPropBones;

void TestDstComplain(Symbol s) {
    MILO_NOTIFY_ONCE("src %s not in dst, punting animation", s);
}

short charbonesfakelmao() { return MakeShortAng(0); }

void ByteQuat::Set(const Hmx::Quat &q) {
    x = floor(Clamp(-127.0f, 127.0f, q.x * 127.0f + 0.5f));
    y = floor(Clamp(-127.0f, 127.0f, q.y * 127.0f + 0.5f));
    z = floor(Clamp(-127.0f, 127.0f, q.z * 127.0f + 0.5f));
    w = floor(Clamp(-127.0f, 127.0f, q.w * 127.0f + 0.5f));
}

void ShortQuat::Set(const Hmx::Quat &q) {
    x = floor(Clamp(-32767.0f, 32767.0f, q.x * 32767.0f + 0.5f));
    y = floor(Clamp(-32767.0f, 32767.0f, q.y * 32767.0f + 0.5f));
    z = floor(Clamp(-32767.0f, 32767.0f, q.z * 32767.0f + 0.5f));
    w = floor(Clamp(-32767.0f, 32767.0f, q.w * 32767.0f + 0.5f));
}

void ShortVector3::Set(const Vector3 &vec) {
    x = ToShort(vec.x);
    y = ToShort(vec.y);
    z = ToShort(vec.z);
}

CharBones::CharBones() : mCompression(kCompressNone), mStart(0), mTotalSize(0) {
    for (int i = 0; i < NUM_TYPES; i++) {
        mCounts[i] = 0;
        mOffsets[i] = 0;
    }
}

void CharBones::ScaleAdd(CharClip *clip, float f1, float f2, float f3) {
    clip->ScaleAdd(*this, f1, f2, f3);
}

void CharBones::Print() {
    FOREACH (it, mBones) {
        MILO_LOG("%s %.2f: %s\n", it->name, it->weight, StringVal(it->name));
    }
}

void CharBones::Zero() { memset(mStart, 0, mTotalSize); }

int CharBones::TypeSize(int type) const {
    switch (type) {
    case TYPE_POS:
    case TYPE_SCALE:
        if (mCompression >= kCompressVects)
            return sizeof(ShortVector3);
        else
            return sizeof(Vector3);
    case TYPE_QUAT:
        if (mCompression >= kCompressQuats)
            return sizeof(ByteQuat);
        else if (mCompression != kCompressNone)
            return sizeof(ShortQuat);
        else
            return sizeof(Hmx::Quat);
    // type rotx/y/z
    default:
        if (mCompression != kCompressNone)
            return sizeof(short);
        else
            return sizeof(float);
    }
}

void CharBones::RecomputeSizes() {
    mOffsets[TYPE_POS] = 0;
    for (int i = 0; i < NUM_TYPES; i++) {
        mOffsets[i + 1] = mOffsets[i] + (mCounts[i + 1] - mCounts[i]) * TypeSize(i);
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
    for (const char *p = s.Str(); *p != '\0'; p++) {
        if (p[0] == '.') {
            switch (p[1]) {
            case 'p':
                return TYPE_POS;
            case 's':
                return TYPE_SCALE;
            case 'q':
                return TYPE_QUAT;
            case 'r':
                // 'x' == TYPE_ROTX
                // 'y' == TYPE_ROTY
                // 'z' == TYPE_ROTZ
                if (p[3] >= 'x' && p[3] <= 'z') {
                    return (Type)(p[3] - 'u');
                }
                break;
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

void CharBones::AddBones(const std::vector<Bone> &bones) {
    FOREACH (it, bones) {
        AddBoneInternal(*it);
    }
    ReallocateInternal();
}

void CharBones::AddBones(const std::list<Bone> &bones) {
    FOREACH (it, bones) {
        AddBoneInternal(*it);
    }
    ReallocateInternal();
}

BEGIN_PROPSYNCS(CharBonesObject)
    gPropBones = this;
    SYNC_PROP(bones, mBones)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void CharBones::ScaleAddIdentity() {
    Hmx::Quat *quatEnd = (Hmx::Quat *)(mStart + mOffsets[TYPE_ROTX]);
    Hmx::Quat *quatStart = (Hmx::Quat *)(mStart + mOffsets[TYPE_QUAT]);
    Bone *bone = &mBones[mQuatCount];
    for (Hmx::Quat *q = quatStart; q != quatEnd; q++, bone++) {
        float diff = 1 - bone->weight;
        if (q->w < 0) {
            q->w -= diff;
        } else {
            q->w += diff;
        }
    }
}

const char *CharBones::StringVal(Symbol s) {
    void *ptr = FindPtr(s);
    switch (TypeOf(s)) {
    case TYPE_POS:
    case TYPE_SCALE:
        if (mCompression >= kCompressVects) {
            ShortVector3 *vptr = (ShortVector3 *)ptr;
            Vector3 v;
            vptr->ToVector3(v);
            return MakeString("%g %g %g", v.x, v.y, v.z);
        } else {
            Vector3 *vptr = (Vector3 *)ptr;
            return MakeString("%g %g %g", vptr->x, vptr->y, vptr->z);
        }
        break;
    case TYPE_QUAT: {
        Hmx::Quat quat;
        if (mCompression >= kCompressQuats) {
            ByteQuat *qPtr = (ByteQuat *)ptr;
            qPtr->ToQuat(quat);
        } else if (mCompression != kCompressNone) {
            ShortQuat *qPtr = (ShortQuat *)ptr;
            qPtr->ToQuat(quat);
        } else {
            Hmx::Quat *qPtr = (Hmx::Quat *)ptr;
            quat = *qPtr;
        }
        Vector3 v;
        MakeEuler(quat, v);
        v *= RAD2DEG;
        return MakeString(
            "quat(%g %g %g %g) euler(%g %g %g)",
            quat.x,
            quat.y,
            quat.z,
            quat.w,
            v.x,
            v.y,
            v.z
        );
        break;
    }
    default: {
        float floatVal;
        if (mCompression != kCompressNone) {
            floatVal = *((short *)ptr) * 0.00061035156f;
        } else {
            floatVal = *((float *)ptr);
        }
        floatVal *= RAD2DEG;
        if (mCompression != kCompressNone) {
            return MakeString("deg %g raw %d", floatVal, *((short *)ptr));
        } else {
            return MakeString("deg %g rad %g", floatVal, *((float *)ptr));
        }

        break;
    }
    }
}

void CharBones::ScaleDown(CharBones &bones, float f2) const {
    if (!mBones.empty()) {
        Bone *myBonesItr = (Bone *)&mBones[0];
        if (f2 == 0) {
            if (mQuatCount > mPosCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
                Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
                Vector3 *otherVecItr = bones.GetPos();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    myBonesItr++;
                    otherVecItr->Zero();
                    otherBonesItr->weight = 0;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                }
            }
            if (mRotXCount > mQuatCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
                Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
                Hmx::Quat *otherQuatItr = bones.GetQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    myBonesItr++;
                    otherQuatItr->Set(0, 0, 0, 0);
                    otherBonesItr->weight = 0;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                }
            }
            if (mEndCount > mRotXCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
                Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
                float *otherRotItr = bones.GetRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    myBonesItr++;
                    *otherRotItr = 0;
                    otherBonesItr->weight = 0;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                }
            }
        } else {
            if (mQuatCount > mPosCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
                Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
                Vector3 *otherVecItr = bones.GetPos();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    myBonesItr++;
                    *otherVecItr *= f2;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                }
            }
            if (mRotXCount > mQuatCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
                Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
                Hmx::Quat *otherQuatItr = bones.GetQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    myBonesItr++;
                    otherQuatItr->Set(
                        otherQuatItr->x * f2,
                        otherQuatItr->y * f2,
                        otherQuatItr->z * f2,
                        otherQuatItr->w * f2
                    );
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                }
            }
            if (mEndCount > mRotXCount) {
                Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
                Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
                Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
                float *otherRotItr = bones.GetRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    myBonesItr++;
                    *otherRotItr *= f2;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                }
            }
        }
    }
}

void CharBones::ScaleAdd(CharBones &bones, float f2) const {
    if (!mBones.empty()) {
        Bone *myBonesItr = (Bone *)&mBones[0];
        if (mQuatCount > mPosCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
            Vector3 *otherVecItr = bones.GetPos();
            if (mCompression >= kCompressVects) {
                ShortVector3 *myVecItr = GetShortPos();
                while (true) {
                    Vector3 v;
                    myVecItr->ToVector3(v);
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    ScaleAddEq(*otherVecItr, v, f2);
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            } else {
                Vector3 *myVecItr = GetPos();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    ScaleAddEq(*otherVecItr, *myVecItr, f2);
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            }
        }
        if (mRotXCount > mQuatCount) {
            float f2abs = fabsf(f2);
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
            Hmx::Quat *otherQuatItr = bones.GetQuat();
            if (mCompression >= kCompressQuats) {
                float absConstant = f2abs * 0.007874016f;
                float notAbsConstant = f2 * 0.007874016f;
                ByteQuat *myQuatItr = GetByteQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    q.Set(
                        myQuatItr->x * absConstant,
                        myQuatItr->y * absConstant,
                        myQuatItr->z * absConstant,
                        myQuatItr->w * notAbsConstant
                    );
                    if (q * *otherQuatItr < 0) {
                        otherQuatItr->x -= q.x;
                        otherQuatItr->y -= q.y;
                        otherQuatItr->z -= q.z;
                        otherQuatItr->w -= q.w;
                    } else {
                        otherQuatItr->x += q.x;
                        otherQuatItr->y += q.y;
                        otherQuatItr->z += q.z;
                        otherQuatItr->w += q.w;
                    }
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else if (mCompression != kCompressNone) {
                float absConstant = f2abs * 0.000030518509f;
                float notAbsConstant = f2 * 0.000030518509f;
                ShortQuat *myQuatItr = GetShortQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    q.Set(
                        myQuatItr->x * absConstant,
                        myQuatItr->y * absConstant,
                        myQuatItr->z * absConstant,
                        myQuatItr->w * notAbsConstant
                    );
                    if (q * *otherQuatItr < 0) {
                        otherQuatItr->x -= q.x;
                        otherQuatItr->y -= q.y;
                        otherQuatItr->z -= q.z;
                        otherQuatItr->w -= q.w;
                    } else {
                        otherQuatItr->x += q.x;
                        otherQuatItr->y += q.y;
                        otherQuatItr->z += q.z;
                        otherQuatItr->w += q.w;
                    }
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else {
                Hmx::Quat *myQuatItr = GetQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    q.Set(
                        myQuatItr->x * f2abs,
                        myQuatItr->y * f2abs,
                        myQuatItr->z * f2abs,
                        myQuatItr->w * f2
                    );
                    if (q * *otherQuatItr < 0) {
                        otherQuatItr->x -= q.x;
                        otherQuatItr->y -= q.y;
                        otherQuatItr->z -= q.z;
                        otherQuatItr->w -= q.w;
                    } else {
                        otherQuatItr->x += q.x;
                        otherQuatItr->y += q.y;
                        otherQuatItr->z += q.z;
                        otherQuatItr->w += q.w;
                    }
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            }
        }
        if (mEndCount > mRotXCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
            Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
            float *otherRotItr = bones.GetRotX();
            if (mCompression != kCompressNone) {
                float shortConstant = f2 * 0.00061035156f;
                short *myRotItr = GetShortRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr * shortConstant;
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            } else {
                float *myRotItr = GetRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr * f2;
                    otherBonesItr->weight += myBonesItr->weight * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            }
        }
    }
}

void CharBones::Blend(CharBones &bones) const {
    MILO_ASSERT(!mCompression && !bones.mCompression, 0x311);
    if (!mBones.empty()) {
        Bone *myBonesItr = (Bone *)&mBones[0];
        if (mQuatCount > mPosCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
            Vector3 *myVecItr = GetPos();
            Vector3 *otherVecItr = bones.GetPos();
            while (true) {
                while (otherBonesItr->name != myBonesItr->name) {
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                }
                *otherVecItr *= 1 - myBonesItr->weight;
                *otherVecItr += *myVecItr;
                myBonesItr++;
                if (myBonesItr == myBonesEnd) {
                    break;
                }
                otherBonesItr++;
                if (otherBonesItr >= otherBonesEnd) {
                    TestDstComplain(myBonesItr->name);
                    return;
                }
                otherVecItr++;
                myVecItr++;
            }
        }
        if (mRotXCount > mQuatCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
            Hmx::Quat *otherQuatItr = bones.GetQuat();
            Hmx::Quat *myQuatItr = GetQuat();
            while (true) {
                while (otherBonesItr->name != myBonesItr->name) {
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                }
                float scalar = 1 - myBonesItr->weight;
                otherQuatItr->x *= scalar;
                otherQuatItr->y *= scalar;
                otherQuatItr->z *= scalar;
                otherQuatItr->w *= scalar;
                float abs = fabsf(myBonesItr->weight);
                Hmx::Quat q(
                    myQuatItr->x * abs,
                    myQuatItr->y * abs,
                    myQuatItr->z * abs,
                    myQuatItr->w * myBonesItr->weight
                );
                if (q * *otherQuatItr < 0) {
                    otherQuatItr->x -= q.x;
                    otherQuatItr->y -= q.y;
                    otherQuatItr->z -= q.z;
                    otherQuatItr->w -= q.w;
                } else {
                    otherQuatItr->x += q.x;
                    otherQuatItr->y += q.y;
                    otherQuatItr->z += q.z;
                    otherQuatItr->w += q.w;
                }
                myBonesItr++;
                if (myBonesItr == myBonesEnd) {
                    break;
                }
                otherBonesItr++;
                if (otherBonesItr >= otherBonesEnd) {
                    TestDstComplain(myBonesItr->name);
                    return;
                }
                otherQuatItr++;
                myQuatItr++;
            }
        }
        if (mEndCount > mRotXCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
            Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
            float *otherRotItr = bones.GetRotX();
            float *myRotItr = GetRotX();
            while (true) {
                while (otherBonesItr->name != myBonesItr->name) {
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                }
                *otherRotItr *= 1 - myBonesItr->weight;
                *otherRotItr += *myRotItr * myBonesItr->weight;
                myBonesItr++;
                if (myBonesItr == myBonesEnd) {
                    return;
                }
                otherBonesItr++;
                if (otherBonesItr >= otherBonesEnd) {
                    TestDstComplain(myBonesItr->name);
                    return;
                }
                otherRotItr++;
                myRotItr++;
            }
        }
    }
}

void CharBones::RotateBy(CharBones &bones) const {
    if (!mBones.empty()) {
        Bone *myBonesItr = (Bone *)&mBones[0];
        if (mQuatCount > mPosCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
            Vector3 *otherVecItr = bones.GetPos();
            if (mCompression >= kCompressVects) {
                ShortVector3 *myVecItr = GetShortPos();
                while (true) {
                    Vector3 v;
                    myVecItr->ToVector3(v);
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    *otherVecItr += v;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            } else {
                Vector3 *myVecItr = GetPos();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    *otherVecItr += *myVecItr;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            }
        }
        if (mRotXCount > mQuatCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
            Hmx::Quat *otherQuatItr = bones.GetQuat();
            if (mCompression >= kCompressQuats) {
                ByteQuat *myQuatItr = GetByteQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    myQuatItr->ToQuat(q);
                    Multiply(q, *otherQuatItr, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else if (mCompression != kCompressNone) {
                ShortQuat *myQuatItr = GetShortQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    myQuatItr->ToQuat(q);
                    Multiply(q, *otherQuatItr, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else {
                Hmx::Quat *myQuatItr = GetQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Multiply(*myQuatItr, *otherQuatItr, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            }
        }
        if (mEndCount > mRotXCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
            Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
            float *otherRotItr = bones.GetRotX();
            if (mCompression != kCompressNone) {
                short *myRotItr = GetShortRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr * 0.00061035156f;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            } else {
                float *myRotItr = GetRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            }
        }
    }
}

void CharBones::RotateTo(CharBones &bones, float f2) const {
    if (!mBones.empty()) {
        Bone *myBonesItr = (Bone *)&mBones[0];
        if (mQuatCount > mPosCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mPosCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *myBonesEnd = (Bone *)&mBones[mQuatCount];
            Vector3 *otherVecItr = bones.GetPos();
            if (mCompression >= kCompressVects) {
                ShortVector3 *myVecItr = GetShortPos();
                while (true) {
                    Vector3 v;
                    myVecItr->ToVector3(v);
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    ScaleAddEq(*otherVecItr, v, f2);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            } else {
                Vector3 *myVecItr = GetPos();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherVecItr++;
                    }
                    ScaleAddEq(*otherVecItr, *myVecItr, f2);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherVecItr++;
                    myVecItr++;
                }
            }
        }
        if (mRotXCount > mQuatCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mQuatCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *myBonesEnd = (Bone *)&mBones[mRotXCount];
            Hmx::Quat *otherQuatItr = bones.GetQuat();
            if (mCompression >= kCompressQuats) {
                ByteQuat *myQuatItr = GetByteQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    myQuatItr->ToQuat(q);
                    q.x *= f2;
                    q.y *= f2;
                    q.z *= f2;
                    if (q.w < 0) {
                        q.w = (q.w * f2) - (1 - f2);
                    } else {
                        q.w = (q.w * f2) + (1 - f2);
                    }
                    Multiply(*otherQuatItr, q, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else if (mCompression != kCompressNone) {
                ShortQuat *myQuatItr = GetShortQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    myQuatItr->ToQuat(q);
                    q.x *= f2;
                    q.y *= f2;
                    q.z *= f2;
                    if (q.w < 0) {
                        q.w = (q.w * f2) - (1 - f2);
                    } else {
                        q.w = (q.w * f2) + (1 - f2);
                    }
                    Multiply(*otherQuatItr, q, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            } else {
                Hmx::Quat *myQuatItr = GetQuat();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherQuatItr++;
                    }
                    Hmx::Quat q;
                    q.Set(
                        myQuatItr->x * f2,
                        myQuatItr->y * f2,
                        myQuatItr->z * f2,
                        myQuatItr->w * f2
                    );
                    if (myQuatItr->w < 0) {
                        q.w -= (1 - f2);
                    } else {
                        q.w += (1 - f2);
                    }
                    Multiply(*otherQuatItr, q, *otherQuatItr);
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        break;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherQuatItr++;
                    myQuatItr++;
                }
            }
        }
        if (mEndCount > mRotXCount) {
            Bone *otherBonesItr = (Bone *)&bones.mBones[bones.mRotXCount];
            Bone *otherBonesEnd = (Bone *)&bones.mBones[bones.mEndCount];
            Bone *myBonesEnd = (Bone *)&mBones[mEndCount];
            float *otherRotItr = bones.GetRotX();
            if (mCompression != kCompressNone) {
                float shortConstant = f2 * 0.00061035156f;
                short *myRotItr = GetShortRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr * shortConstant;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            } else {
                float *myRotItr = GetRotX();
                while (true) {
                    while (otherBonesItr->name != myBonesItr->name) {
                        otherBonesItr++;
                        if (otherBonesItr >= otherBonesEnd) {
                            TestDstComplain(myBonesItr->name);
                            return;
                        }
                        otherRotItr++;
                    }
                    *otherRotItr += *myRotItr * f2;
                    myBonesItr++;
                    if (myBonesItr == myBonesEnd) {
                        return;
                    }
                    otherBonesItr++;
                    if (otherBonesItr >= otherBonesEnd) {
                        TestDstComplain(myBonesItr->name);
                        return;
                    }
                    otherRotItr++;
                    myRotItr++;
                }
            }
        }
    }
}

void CharBones::AddBoneInternal(const Bone &bone) {
    CharBones::Type t = TypeOf(bone.name);
    int idx;
    for (idx = mCounts[t]; idx < mCounts[t + 1]; idx++) {
        if (mBones[idx].name == bone.name) {
            return;
        }
        if (strcmp(mBones[idx].name.Str(), bone.name.Str()) >= 0) {
            break;
        }
    }
    mBones.insert(mBones.begin() + idx, bone);
    int size = TypeSize(t);
    for (t = (Type)(t + 1); t < CharBones::NUM_TYPES; t = (Type)(t + 1)) {
        mCounts[t]++;
        mOffsets[t] += size;
    }
    mTotalSize = mOffsets[TYPE_END] + 0xFU & 0xFFFFFFF0; // round up to the nearest 0x10,
                                                         // alignment moment
}

CharBonesAlloc::~CharBonesAlloc() { MemFree(mStart); }

void CharBonesAlloc::ReallocateInternal() {
    MemFree(mStart);
    mStart = (char *)MemAlloc(mTotalSize, __FILE__, 0x6C0, "CharBones");
}
