#pragma once
#include "os/Debug.h"
#include <cstdlib>
#include <list>
#include <vector>

enum RandomType {
    NOT_RANDOM = 0x0000,
    PARTITIONED_RANDOM = 0x0001,
    JUKEBOX_RANDOM = 0x0002,
    FULL_RANDOM = 0x0003
};

template <class T>
class PseudoRandomPicker {
public:
    PseudoRandomPicker() : mRandomType(PARTITIONED_RANDOM), mTopPct(0.5), mNumPicked(0) {}
    ~PseudoRandomPicker() {}

    void SetType(RandomType type) { mRandomType = type; }
    void AddItem(const T item) { mItems.push_back(item); }
    void SetNumPicked(int i) { mNumPicked = i; }
    void SetTopPct(float f) { mTopPct = f; }

    void ResetModes() {
        mNumPicked = 0;
        mRandomType = JUKEBOX_RANDOM;
    }

    void AddItems(const std::vector<T> &itemVec) {
        for (int i = 0; i < itemVec.size(); i++) {
            T cur = itemVec[i];
            mItems.push_back(cur);
        }
    }

    void Randomize() { // matches but i dont know why
        MILO_ASSERT(Size() > 0, 0x83);
        int size = Size();
        for (int n = mItems.size(); n != 0; n--) {
            typename std::list<T>::iterator first = mItems.begin();
            T val = *first;
            mItems.pop_front();
            mItems.size();
            int idx = rand() % size;
            typename std::list<T>::iterator it = mItems.begin();
            if (idx != 0) {
                for (int i = idx; i != 0; i--) {
                    ++it;
                }
            }
            mItems.insert(it, val);
            mItems.size();
        }
    }

    int Size() const { return mItems.size(); }
    void Clear() { mItems.resize(0); }
    void Resize(int size) { mItems.resize(size); }

    const T GetItem(int idx) {
        if (idx >= 0) {
            typename std::list<T>::iterator it;
            if (idx <= (unsigned int)mItems.size()) {
                it = mItems.begin();
                if (idx != 0) {
                    for (int i = idx; i != 0; i--) {
                        ++it;
                    }
                }
                T val = *it;
                mItems.erase(it);
                mItems.push_back(val);
                return val;
            }
        }
        return T(0);
    }

    const T GetNext() {
        MILO_ASSERT(Size() > 0, 0x52);
        int idx;
        switch (mRandomType) {
        case NOT_RANDOM:
            idx = 0;
            break;
        case FULL_RANDOM:
            idx = rand() % Size();
            break;
        case JUKEBOX_RANDOM: {
            int size = Size();
            idx = (size - (mNumPicked % size));
            idx = rand() % idx;
        } break;
        default:
            int i1 = mTopPct * (float)Size() + 1.01f;
            idx = rand() % i1;
            break;
        }
        mNumPicked++;
        return GetItem(idx);
    }

private:
    std::list<T> mItems; // 0x0
    RandomType mRandomType; // 0x8
    float mTopPct; // 0xc
    int mNumPicked; // 0x10
};
