#pragma once
#include "math/Geo.h"
#include "math/Vec.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include <float.h>
#include <list>

// kdTree size: 0x2c
template <class T>
class kdTree {
public:
    enum SplitPlaneType {
        kSplitPlane_Mean = 0,
        kSplitPlane_Median = 1,
        kSplitPlane_SAH = 2
    };

    class kdTriList {
    public:
        MEM_ARRAY_OVERLOAD(kdTriList, 0xC6);

        kdTriList() : mData(0) {}

        // const Triangle* GetNext(); // returns this + 4
        // bool IsEnd() const;
        // void SetData(const Triangle*);

        static kdTriList *Allocate(unsigned int inNumNodes) {
            kdTriList *list = new kdTriList[inNumNodes + 1];
            list[inNumNodes].mData = (Triangle *)-1;
            return list;
        }

        const Triangle *mData; // 0x0
    };

    // size 0x6
    class kdTreeNode {
    public:
        // size 0xc
        struct Stack {
            kdTreeNode *mNode; // 0x0
            float mTMin; // 0x4
            float mTMax; // 0x8
        };

        kdTreeNode() {
            mTriList = 0;
            mLeaf_Index = 0x8000;
            mSplitValue = 0;
            mSplitAxis &= ~3;
        }
        ~kdTreeNode() {
            if (mLeaf_Index & 0x8000 && mTriList) {
                delete[] mTriList;
                mTriList = nullptr;
            }
        }

        float EvaluateSplit(
            const Box &box,
            const std::list<Triangle *> &triangles,
            unsigned char idx,
            float threshold
        ) const {
            if (box.mMax[idx] >= threshold && box.mMin[idx] <= threshold) {
                Box box100 = box;
                Box boxe0 = box;
            } else {
                return FLT_MAX;
            }
        }

        bool
        FindSplit_Mean(const Box &inDimensions, const std::list<Triangle *> &inTriList) {
            float yDiff = inDimensions.mMax.y - inDimensions.mMin.y;
            float zDiff = inDimensions.mMax.z - inDimensions.mMin.z;
            if (inDimensions.mMax.x - inDimensions.mMin.x > yDiff) {
                mSplitAxis = 0;
            } else {
                mSplitAxis = 1;
            }
            if (zDiff > yDiff) {
                mSplitAxis = 2;
            }
            unsigned int vecIdx = mSplitAxis;
            float idxDiff = inDimensions.mMax[vecIdx] - inDimensions.mMin[vecIdx];
            int numContains = 0;
            mSplitValue = idxDiff / 2.0f + inDimensions.mMin[mSplitAxis];
            mSplitAxis = 3;
            float fsum = 0;
            if (!inTriList.empty()) {
                FOREACH (it, inTriList) {
                    Triangle *cur = *it;
                    for (int i = 0; i < 3; i++) {
                        if (inDimensions.Contains(cur->origin)) {
                            numContains++;
                            fsum += cur->origin[mSplitAxis];
                        }
                    }
                }
                if (numContains != 0) {
                    mSplitValue = fsum / numContains;
                    mSplitAxis = 3;
                }
            }
            return true;
        }
        bool
        FindSplit_SAH(const Box &inDimensions, const std::list<Triangle *> &inTriList);
        void Pack(
            SplitPlaneType splitType,
            const Box &inDimensions,
            std::list<Triangle *> &inTriList,
            kdTreeNode *inRoot,
            unsigned char depth
        ) {
            if (depth < 0xF) {
                // a whole lotta stuff
                if (!inTriList.empty()) {
                    if (inTriList.size() >= 10) {
                        bool find = false;
                        switch (splitType) {
                        case 0:
                        case 1:
                            find = FindSplit_Mean(inDimensions, inTriList);
                            break;
                        case 2:
                            find = FindSplit_SAH(inDimensions, inTriList);
                            break;
                        default:
                            MILO_FAIL("Invalid split plane type");
                            break;
                        }
                    }
                }
            }
            MILO_ASSERT(GetIsLeaf(), 0x19F);
            if (inTriList.empty()) {
                mTriList = nullptr;
            } else {
                mTriList = kdTriList::Allocate(inTriList.size());
                FOREACH (it, inTriList) {
                }
            }
        }

        MEM_ARRAY_OVERLOAD(kdTreeNode, 0xEC);

        bool GetIsLeaf() const { return mLeaf_Index & 0x8000; }
        unsigned short GetIndex() const { return mLeaf_Index & 0x7FFF; }
        unsigned int GetSplitAxis() const { return mSplitAxis & 0x3; }
        float GetSplitValue() const { return mSplitValue; }
        void SetIsLeaf(unsigned int leaf) { mLeaf_Index |= (leaf << 15); }

        // from RB3 bank 5
        // GetChild_0(kdTreeNode*)
        // GetChild_1(kdTreeNode*)

        union {
            float mSplitValue;
            unsigned int mSplitAxis;
            kdTriList *mTriList;
        }; // 0x0
        // bit 16 = is leaf
        // bits 15-0 = index
        unsigned short mLeaf_Index; // 0x4
    };

    kdTree(const Box &box) {
        mAABB.Set(box.mMin, box.mMax);
        mNodeArray = new kdTreeNode[0x8000];
        for (int i = 0; i < 0x8000; i++) {
        }
    }
    ~kdTree() { delete[] mNodeArray; }

    // from RB3 bank 5 and DC1 debug
    void Build(SplitPlaneType splitType);
    void Insert(T *inTri) { mInitialList.push_back(inTri); }
    bool Intersect(const Vector3 &, const Vector3 &, float, float &) const;

    // guessed/inferred
    void PackNodes(SplitPlaneType splitType, unsigned char depth) {
        mNodeArray->Pack(splitType, mAABB, mInitialList, mNodeArray, depth);
    }

private:
    std::list<T *> mInitialList; // 0x0
    kdTreeNode *mNodeArray; // 0x8
    Box mAABB; // 0xc
};
