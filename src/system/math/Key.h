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

inline BinStream &operator>>(BinStream &bs, Weight &w) {
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
BinStreamRev &operator>>(BinStreamRev &d, Key<T> &key) {
    d >> key.value >> key.frame;
    return d;
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
    /** Remove the key at the given index.
     * @param [in] idx The index in the vector to remove.
     */
    void Remove(int idx) { erase(begin() + idx); }

    /** Remove all keyframes whose frames are in the range [from, to).
     * @param [in] from The first frame to remove.
     * @param [in] to The end of the removal range (exclusive).
     * @returns The index in the vector where the removal began.
     */
    int Remove(float from, float to) {
        int startidx = KeyGreaterEq(from);
        int endidx = KeyGreaterEq(to);
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
    int Add(const T1 &value, float frame, bool unique) {
        int bound = KeyGreaterEq(frame);
        if (unique && bound != size() && (*this)[bound].frame == frame) {
            (*this)[bound].value = value;
        } else {
            while (bound < size() && (*this)[bound].frame == frame) {
                bound++;
            }
            insert(&(*this)[bound], Key<T1>(value, frame));
        }
        return bound;
    }

    /** Given a start and end frame, get the closest start and end indices of this vector.
     * NOTE: if both the start and end frame are 0, they will be overwritten to the first
     * and last frame, istart will become 0, and iend will become the last index of the
     * vector.
     * @param [in] start The start frame.
     * @param [in] end The end frame.
     * @param [out] startKey The index of the last key whose frame <= the start frame.
     * @param [out] endKey The index of the first key whose frame >= the end frame.
     */
    void FindBounds(float &start, float &end, int &startKey, int &endKey) {
        MILO_ASSERT(size(), 0x1AF);
        if (!start && !end) {
            start = front().frame;
            end = back().frame;
            startKey = 0;
            endKey = size() - 1;
        } else {
            startKey = KeyLessEq(Max(start, front().frame));
            endKey = KeyGreaterEq(Min(end, back().frame));
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
     * @param [out] r The interpolation ratio between prev and next, used to blend their
     * values.
     * @returns The index in the vector where this keyframe resides.
     */
    int AtFrame(float frame, const Key<T1> *&prev, const Key<T1> *&next, float &r) const {
        if (empty()) {
            prev = next = nullptr;
            r = 0;
            return -1;
        } else if (frame < front().frame) {
            prev = next = &front();
            r = 0;
            return -1;
        } else if (frame >= back().frame) {
            prev = next = &back();
            r = 0;
            return size() - 1;
        } else {
            int frameIdx = KeyLessEq(frame);
            prev = &(*this)[frameIdx];
            next = &(*this)[frameIdx + 1];
            float den = next->frame - prev->frame;
            MILO_ASSERT(den != 0, 0xFF);
            r = (frame - prev->frame) / den;
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

    /** Get the index range of all keyframes whose frames are <= the supplied frame.
     * If several keyframes share the same frame, the full run of them is returned.
     * NOTE: if the vector is empty or the frame is before the first keyframe, both
     * indices are set to -1.
     * @param [in] frame The supplied frame.
     * @param [out] first The index of the first keyframe at that frame.
     * @param [out] last The index of the last keyframe at that frame.
     */
    void KeysLessEq(float frame, int &first, int &last) const {
        last = -1;
        first = -1;
        if (empty() || frame < front().frame)
            return;
        int i1 = 0;
        int i2 = size();
        while (i2 > i1 + 1) {
            int i5 = (i1 + i2) >> 1;
            const Key<T1> &cur = (*this)[i5];
            if (frame < cur.frame)
                i2 = i5;
            else
                i1 = i5;
        }
        last = i1;
        first = i1;
        while (i1 - 1 >= 0 && (*this)[i1 - 1].frame == (*this)[i1].frame) {
            i1--;
            first = i1;
        }
        while (i1 + 1 < size() && (*this)[i1 + 1].frame == (*this)[i1].frame) {
            i1++;
            last = i1;
        }
    }

    /** Get the keyframe whose frame is closest to the supplied frame.
     * Only the keyframes immediately before and after the supplied frame are considered.
     * @param [in] frame The supplied frame.
     * @returns A pointer to the nearest keyframe, or nullptr if none exists.
     */
    Key<T1> *KeyNearest(float frame) {
        int i4 = -1;
        float diff = kHugeFloat;
        int idx = KeyLessEq(frame);
        if (idx >= 0 && idx < size()) {
            if (MinEq(diff, frame - (*this)[idx].frame)) {
                i4 = idx;
            }
        }
        int next = idx + 1;
        if (next >= 0 && next < size()) {
            if (MinEq(diff, (*this)[next].frame - frame)) {
                i4 = next;
            }
        }
        if (i4 == -1) {
            return nullptr;
        } else {
            return &(*this)[i4];
        }
    }

    /** Linearly interpolate the value at the supplied frame.
     * Frames outside the key range are clamped to the first or last segment.
     * NOTE: unlike AtFrame, this interpolates the key value type T1 directly.
     * @param [in] frame The frame to interpolate at.
     * @param [out] val The interpolated value.
     * @returns False if the vector is empty, otherwise true.
     */
    bool Linear(float frame, T1 &val) const {
        if (size() == 0)
            return false;
        else {
            if (size() == 1)
                val = front().value;
            else {
                int idx = Clamp<int>(0, size() - 2, KeyLessEq(frame));
                const Key<T1> &keyNow = (*this)[idx];
                const Key<T1> &keyNext = (*this)[idx + 1];
                Interp(
                    keyNow.value,
                    keyNext.value,
                    (frame - keyNow.frame) / (keyNext.frame - keyNow.frame),
                    val
                );
            }
            return true;
        }
    }

    /** Find the frame at which the supplied value would occur, by linearly
     * interpolating between the keyframes that bracket the value.
     * @param [in] val The value to search for.
     * @param [out] frame The interpolated frame at which this value occurs.
     * @returns False if the vector is empty, otherwise true.
     */
    bool ReverseLinear(const T1 &val, float &frame) const {
        if (size() == 0)
            return false;
        else if (size() == 1) {
            frame = front().frame;
            return true;
        } else {
            int idx = Clamp<int>(0, size() - 2, ReverseKeyLessEq(val));
            const Key<T1> &keyNow = (*this)[idx];
            const Key<T1> &keyNext = (*this)[idx + 1];
            Interp(
                keyNow.frame,
                keyNext.frame,
                (val - keyNow.value) / (keyNext.value - keyNow.value),
                frame
            );
            return true;
        }
    }

    /** Get the index of the last keyframe whose value <= the supplied value.
     * If several keyframes share the same value, the last of them is returned.
     * NOTE: unlike KeyLessEq, this searches by key *value* rather than by frame.
     * @param [in] val The supplied value.
     * @returns The index of the keyframe that satisfies the condition above, or -1
     * if the vector is empty or the value is below the first key's value.
     */
    int ReverseKeyLessEq(const T1 &val) const {
        if (empty() || val < front().value) {
            return -1;
        } else {
            int i1 = 0;
            int i2 = size();
            while (i2 > i1 + 1) {
                int newCnt = (i1 + i2) >> 1;
                if (val < (*this)[newCnt].value)
                    i2 = newCnt;
                else
                    i1 = newCnt;
            }
            while (i1 + 1 < size() && (*this)[i1 + 1].value == (*this)[i1].value)
                i1++;
            return i1;
        }
    }

    /** Get the value of the last keyframe at or before the supplied frame.
     * @param [in] frame The supplied frame.
     * @param [in] lastFrame The frame the key must lie after.
     * @returns A pointer to the key's value, or nullptr if none qualifies.
     */
    const T1 *Cross(float frame, float lastFrame) const {
        int idx = KeyLessEq(frame);
        if (idx == -1)
            return 0;
        else {
            if (lastFrame >= (*this)[idx].frame)
                return 0;
            else
                return &(*this)[idx].value;
        }
    }

    // int KeyIdxNearest(float);
    // void Linearize(Keys&, float) const;
    // void LinearizeHelper(Keys&, float, int, int) const;
    // int FindFirst(T1&);
    // bool AllSameValues() const;
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
void InterpTangent(
    const Vector3 &, const Vector3 &, const Vector3 &, const Vector3 &, float, Vector3 &
);
void InterpVector(
    const Keys<Vector3, Vector3> &,
    const Key<Vector3> *,
    const Key<Vector3> *,
    float,
    bool,
    Vector3 &,
    Vector3 *
);
void InterpVector(const Keys<Vector3, Vector3> &, bool, float, Vector3 &, Vector3 *);
void QuatSpline(
    const Keys<Hmx::Quat, Hmx::Quat> &,
    const Key<Hmx::Quat> *,
    const Key<Hmx::Quat> *,
    float,
    Hmx::Quat &
);
