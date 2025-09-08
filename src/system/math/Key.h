#pragma once
#include "utl/BinStream.h"
#include "utl/TextStream.h"
#include <vector>

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

    T value;
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
    void FindBounds(float &fstart, float &fend, int &istart, int &iend);

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
    AtFrame(float frame, const Key<T1> *&prev, const Key<T1> *&next, float &ref) const;

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
    int KeyLessEq(float frame) const;

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

    Key<T1 *> KeyNearest(float);

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
        else {
            if (size() == 1)
                fref = front().frame;
            else {
                int numKeys = size();
                int idx = Clamp<int>(0, numKeys - 2, ReverseKeyLessEq(fconst));
                const Key<T1> &keyNow = (*this)[idx];
                const Key<T1> &keyNext = (*this)[idx + 1];
                Interp(
                    keyNow.frame,
                    keyNext.frame,
                    (fconst - keyNow.value) / (keyNext.value - keyNow.value),
                    fref
                );
            }
            return true;
        }
    }

    int ReverseKeyLessEq(const float &fref) const;
    T1 *Cross(float, float) const;
};

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
