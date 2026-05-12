#pragma once
#include "os/Debug.h"
#include <cstdlib>
#include <list>
#include <vector>

template <class T>
class PseudoRandomPicker {
public:
    PseudoRandomPicker() : mMode(1), mSeed(0.5), mNumGets(0) {}
    ~PseudoRandomPicker() {}

    void SetMode(int mode) { mMode = mode; }
    void AddItem(const T item) { mItems.push_back(item); }
    void SetNumGets(int i) { mNumGets = i; }

    void ResetModes() {
        mNumGets = 0;
        mMode = 2;
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
        switch (mMode) {
        case 0:
            idx = 0;
            break;
        case 2:
            // this is the only wrong case
            idx = rand() % (mNumGets % Size());
            break;
        case 3:
            idx = rand() % Size();
            break;
        default:
            int i1 = mSeed * (float)Size() + 1.01f;
            idx = rand() % i1;
            break;
        }
        mNumGets++;
        return GetItem(idx);
    }

private:
    std::list<T> mItems; // 0x0 - items?
    int mMode; // 0x8
    float mSeed; // 0xc
    int mNumGets; // 0x10
};
