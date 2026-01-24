#pragma once
#include "math/Utl.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/TextStream.h"
#include <vector>

struct Weight {
    float weight;
    float derivIn;
    float derivOut;
};

inline BinStreamRev &operator>>(BinStreamRev &bs, Weight &w) {
    bs >> w.weight >> w.derivIn >> w.derivOut;
    return bs;
}

/**
 * @brief A keyframe.
 *
 * @tparam T The value of this keyframe.
 */
template <class T>
class Key {
public:
    Key() : value(T()), frame(0.0f) {}
    Key(const T &v, float f) : value(v), frame(f) {}
    bool operator<(const Key &k) const { return frame < k.frame; }

    /** "The [value] to animate to" */
    T value;
    /** "The frame" */
    float frame;
};

template <class T>
TextStream &operator<<(TextStream &ts, const Key<T> &key) {
    ts << "(frame:" << key.frame << " value:" << key.value << ")";
    return ts;
}

template <class T>
BinStream &operator<<(BinStream &bs, const Key<T> &key) {
    bs << key.value << key.frame;
    return bs;
}

template <class T>
BinStream &operator>>(BinStream &bs, Key<T> &key) {
    bs >> key.value >> key.frame;
    return bs;
}

template <class T>
BinStreamRev &operator>>(BinStreamRev &bs, Key<T> &key) {
    bs >> key.value >> key.frame;
    return bs;
}

// Keys is a vector<Key<T>>
/**
 * @brief A specialized vector for keyframes.
 *
 * @tparam T1 The value type stored inside each keyframe.
 * @tparam T2 The interpolated value type (see AtFrame).
 */
template <class T1, class T2>
class Keys : public std::vector<Key<T1> > {
public:
    /** Get the number of keyframes in this collection. */
    int NumKeys() const { return size(); }

    /** Remove the key at the given index.
     * @param [in] idx The index in the vector to remove.
     */
    void Remove(int idx) { erase(begin() + idx); }

    int Remove(float start, float end) {
        int startidx = KeyGreaterEq(start);
        int endidx = KeyGreaterEq(end);
        erase(begin() + startidx, begin() + endidx);
        return startidx;
    }

    /** Add a value to the keys at a given frame.
     * @param [in] val The value to add.
     * @param [in] frame The frame at which this value will be.
     * @param [in] unique If true, overwrite the existing value at this frame. Otherwise,
     * create a new keyframe.
     * @returns The index in the vector corresponding to this new keyframe.
     */
    int Add(const T1 &val, float frame, bool unique) {
        int bound = KeyGreaterEq(frame);
        if (unique && bound != size() && (*this)[bound].frame == frame) {
            (*this)[bound].value = val;
        } else {
            while (bound < size() && (*this)[bound].frame == frame) {
                bound++;
            }
            insert(&(*this)[bound], Key<T1>(val, frame));
        }
        return bound;
    }

    /** Given a start and end frame, get the closest start and end indices of this vector.
     * NOTE: if both the start and end frame are 0, they will be overwritten to the first
     * and last frame, istart will become 0, and iend will become the last index of the
     * vector.
     * @param [in] fstart The start frame.
     * @param [in] fend The end frame.
     * @param [out] istart The index of the last key whose frame <= the start frame.
     * @param [out] iend The index of the first key whose frame >= the end frame.
     */
    void FindBounds(float &fstart, float &fend, int &istart, int &iend) {
        MILO_ASSERT(size(), 0x1AF);
        if (!fstart && !fend) {
            fstart = front().frame;
            fend = back().frame;
            istart = 0;
            iend = size() - 1;
        } else {
            istart = KeyLessEq(Max(fstart, front().frame));
            iend = KeyGreaterEq(Min(fend, back().frame));
        }
    }

    /** Get the value associated with the supplied frame.
     * @param [in] frame The keyframe to get a value from.
     * @param [out] val The retrieved value.
     * @returns The index in the vector where this keyframe resides.
     */
    int AtFrame(float frame, T2 &val) const {
        const Key<T1> *prev;
        const Key<T1> *next;
        float r;
        int ret = AtFrame(frame, prev, next, r);
        if (prev) {
            Interp(prev->value, next->value, r, val);
        }
        return ret;
    }

    /** Get the value associated with the supplied frame.
     * @param [in] frame The keyframe to get a value from.
     * @param [out] prev The previous key relative to the keyframe we want.
     * @param [out] next The next key relative to the keyframe we want.
     * @param [out] ref TODO: unknown
     * @returns The index in the vector where this keyframe resides.
     */
    int
    AtFrame(float frame, const Key<T1> *&prev, const Key<T1> *&next, float &ref) const {
        if (empty()) {
            prev = next = nullptr;
            ref = 0;
            return -1;
        } else if (frame < front().frame) {
            prev = next = &front();
            ref = 0;
            return -1;
        } else if (frame >= back().frame) {
            prev = next = &back();
            ref = 0;
            return size() - 1;
        } else {
            int frameIdx = KeyLessEq(frame);
            prev = &(*this)[frameIdx];
            next = &(*this)[frameIdx + 1];
            float den = next->frame - prev->frame;
            MILO_ASSERT(den != 0, 0xFF);
            ref = (frame - prev->frame) / den;
            return frameIdx;
        }
    }

    /** Get the first frame of the keys. */
    float FirstFrame() const {
        if (size() != 0)
            return front().frame;
        else
            return 0.0f;
    }

    /** Get the last frame of the keys. */
    float LastFrame() const {
        if (size() != 0)
            return back().frame;
        else
            return 0.0f;
    }

    /** Get the index of the first possible keyframe KF, such that KF's frame >= the
     * supplied frame.
     * @param [in] frame The supplied frame.
     * @returns The index of the keyframe that satisfies the condition above.
     */
    int KeyGreaterEq(float frame) const {
        if (empty() || (frame <= front().frame))
            return 0;
        else {
            const Key<T1> &backKey = back();
            if (frame > backKey.frame) {
                return size();
            } else {
                int cnt = 0;
                int threshold = size() - 1;
                while (threshold > cnt + 1) {
                    int newCnt = (cnt + threshold) >> 1;
                    if (frame > (*this)[newCnt].frame)
                        cnt = newCnt;
                    else
                        threshold = newCnt;
                }
                while (threshold > 1
                       && (*this)[threshold - 1].frame == (*this)[threshold].frame)
                    threshold--;
                return threshold;
            }
        }
    }

    /** Get the index of the last possible keyframe KF, such that KF's frame <= the
     * supplied frame.
     * @param [in] frame The supplied frame.
     * @returns The index of the keyframe that satisfies the condition above.
     */
    int KeyLessEq(float frame) const {
        if (empty() || (frame < front().frame))
            return -1;
        else {
            int cnt = 0;
            int threshold = size();
            while (threshold > cnt + 1) {
                int newCnt = (cnt + threshold) >> 1;
                if (frame < (*this)[newCnt].frame)
                    threshold = newCnt;
                else
                    cnt = newCnt;
            }
            while (cnt + 1 < size() && (*this)[cnt + 1].frame == (*this)[cnt].frame)
                cnt++;
            return cnt;
        }
    }

    void KeysLessEq(float f, int &iref1, int &iref2) const {
        iref2 = -1;
        iref1 = -1;
        if (empty() || f < front().frame)
            return;
        int i1 = 0;
        int i2 = size();
        while (i2 > i1 + 1) {
            int i5 = (i1 + i2) >> 1;
            const Key<T1> &cur = (*this)[i5];
            if (f < cur.frame)
                i2 = i5;
            else
                i1 = i5;
        }
        iref2 = i1;
        iref1 = i1;
        while (i1 - 1 >= 0 && (*this)[i1 - 1].frame == (*this)[i1].frame) {
            i1--;
            iref1 = i1;
        }
        while (i1 + 1 < size() && (*this)[i1 + 1].frame == (*this)[i1].frame) {
            i1++;
            iref2 = i1;
        }
    }

    Key<T1> *KeyNearest(float f1) {
        int i4 = -1;
        float diff = kHugeFloat;
        int idx = KeyLessEq(f1);
        if (idx >= 0 && idx < size()) {
            diff = f1 - (*this)[idx].frame;
            if (MaxEq(diff, kHugeFloat)) {
                i4 = idx;
            }
        }
        int next = idx + 1;
        if (next >= 0 && next < size()) {
            f1 = (*this)[next].frame - f1;
            if (MaxEq(diff, f1)) {
                i4 = next;
            }
        }
        if (i4 == -1) {
            return nullptr;
        } else {
            return &(*this)[i4];
        }
    }

    bool Linear(float f1, float &fref) const {
        if (size() == 0)
            return false;
        else {
            if (size() == 1)
                fref = front().value;
            else {
                int idx = Clamp<int>(0, size() - 2, KeyLessEq(f1));
                const Key<T1> &keyNow = (*this)[idx];
                const Key<T1> &keyNext = (*this)[idx + 1];
                Interp(
                    keyNow.value,
                    keyNext.value,
                    (f1 - keyNow.frame) / (keyNext.frame - keyNow.frame),
                    fref
                );
            }
            return true;
        }
    }

    bool ReverseLinear(const float &fconst, float &fref) const {
        if (size() == 0)
            return false;
        else if (size() == 1) {
            fref = front().frame;
            return true;
        } else {
            int idx = Clamp<int>(0, size() - 2, ReverseKeyLessEq(fconst));
            const Key<T1> &keyNow = (*this)[idx];
            const Key<T1> &keyNext = (*this)[idx + 1];
            Interp(
                keyNow.frame,
                keyNext.frame,
                (fconst - keyNow.value) / (keyNext.value - keyNow.value),
                fref
            );
            return true;
        }
    }

    int ReverseKeyLessEq(const T1 &fref) const {
        if (empty() || fref < front().value) {
            return -1;
        } else {
            int i1 = 0;
            int i2 = size();
            while (i2 > i1 + 1) {
                int newCnt = (i1 + i2) >> 1;
                if (fref < (*this)[newCnt].value)
                    i2 = newCnt;
                else
                    i1 = newCnt;
            }
            while (i1 + 1 < size() && (*this)[i1 + 1].value == (*this)[i1].value)
                i1++;
            return i1;
        }
    }

    const T1 *Cross(float f1, float f2) const {
        int idx = KeyLessEq(f1);
        if (idx == -1)
            return 0;
        else {
            if (f2 >= (*this)[idx].frame)
                return 0;
            else
                return &(*this)[idx].value;
        }
    }
};

template <class T1, class T2>
BinStreamRev &operator>>(BinStreamRev &bs, Keys<T1, T2> &keys) {
    return bs >> (std::vector<Key<T1> > &)keys;
}

/** Scale keyframes by a supplied multiplier.
 * @param [in] keys The collection of keys to multiply the frames of.
 * @param [in] scale The multiplier value.
 */
template <class T1, class T2>
void ScaleFrame(Keys<T1, T2> &keys, float scale) {
    for (Keys<T1, T2>::iterator it = keys.begin(); it != keys.end(); ++it) {
        (*it).frame *= scale;
    }
}

class Vector3;
namespace Hmx {
    class Quat;
}

// math functions defined in math/Key.cpp:
void SplineTangent(const Keys<Vector3, Vector3> &, int, Vector3 &);
void InterpTangent(const Vector3 &, const Vector3 &, const Vector3 &, const Vector3 &, float, Vector3 &);
void InterpVector(const Keys<Vector3, Vector3> &, const Key<Vector3> *, const Key<Vector3> *, float, bool, Vector3 &, Vector3 *);
void InterpVector(const Keys<Vector3, Vector3> &, bool, float, Vector3 &, Vector3 *);
void QuatSpline(const Keys<Hmx::Quat, Hmx::Quat> &, const Key<Hmx::Quat> *, const Key<Hmx::Quat> *, float, Hmx::Quat &);
