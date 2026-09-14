#pragma once
#include "math/Geo.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "math/Vec.inl"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include <float.h>
#include <list>

// kdTree size: 0x2c
// https://en.wikipedia.org/wiki/K-d_tree
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

        kdTriList *GetNext() {
            kdTriList *cur = this;
            return &cur[1];
        }
        bool IsEnd() const { return (int)mData == -1; }
        void SetData(const Triangle *t) { mData = t; }

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
            if (threshold <= box.mMax[idx] && threshold >= box.mMin[idx]) {
                Box box100;
                Box boxe0;
                box100.Set(box.mMin, box.mMax);
                boxe0.Set(box.mMin, box.mMax);
                box100.mMax[idx] = threshold;
                boxe0.mMin[idx] = threshold;
                float f10 = 1 / box.SurfaceArea();
                float f8 = box100.SurfaceArea() * f10;
                float f7 = boxe0.SurfaceArea() * f10;
                f10 = 0;
                float f11 = 0;
                FOREACH (it, triangles) {
                    Triangle *cur = *it;
                    if (box100.Contains(*cur)) {
                        f10 += 1;
                    } else if (boxe0.Contains(*cur)) {
                        f11 += 1;
                    } else {
                        if (::Intersect(*cur, box100)) {
                            f10 += 0.5f;
                        }
                        if (::Intersect(*cur, boxe0)) {
                            f11 += 0.5f;
                        }
                    }
                }
                return f11 * f7 + f10 * f8 + 0.3f;
            } else {
                return FLT_MAX;
            }
        }

        bool
        FindSplit_Mean(const Box &inDimensions, const std::list<Triangle *> &inTriList) {
            float yDiff = inDimensions.mMax.y - inDimensions.mMin.y;
            float zDiff = inDimensions.mMax.z - inDimensions.mMin.z;
            if (inDimensions.mMax.x - inDimensions.mMin.x > yDiff) {
                SetSplitAxis(kSplitPlane_Mean);
            } else {
                SetSplitAxis(kSplitPlane_Median);
            }
            if (zDiff > yDiff) {
                SetSplitAxis(kSplitPlane_SAH);
            }
            float idxDiff =
                inDimensions.mMax[GetSplitAxis()] - inDimensions.mMin[GetSplitAxis()];
            SetSplitValue(idxDiff / 2 + inDimensions.mMin[GetSplitAxis()]);
            unsigned int numContains = 0;
            double fsum = 0;
            FOREACH (it, inTriList) {
                Triangle *cur = *it;
                Vector3 vecs[3];
                vecs[0] = cur->origin;
                Add(cur->origin, cur->frame.x, vecs[1]);
                Add(cur->origin, cur->frame.y, vecs[2]);
                for (int i = 0; i < 3; i++) {
                    if (inDimensions.Contains(vecs[i])) {
                        fsum += vecs[i][GetSplitAxis()];
                        numContains++;
                    }
                }
            }
            if (numContains != 0) {
                SetSplitValue(fsum / numContains);
            }
            return true;
        }

        bool
        FindSplit_SAH(const Box &inDimensions, const std::list<Triangle *> &inTriList) {
            float numTriangles = inTriList.size();

            float f90[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
            float f80[3] = { FLT_MAX, FLT_MAX, FLT_MAX };

            for (unsigned char i = 0; i < 3; i++) {
                float f12 = (inDimensions.mMax[i] - inDimensions.mMin[i]) / 17;
                float f13 = inDimensions.mMin[i];
                for (int j = 0; j < 16; j++) {
                    f13 += f12;
                    float f14 = EvaluateSplit(inDimensions, inTriList, i, f13);
                    if (f14 < f90[i]) {
                        f90[i] = f14;
                        f80[i] = f13;
                    }
                }
            }
            unsigned char idx;
            if (f90[1] >= f90[0]) {
                idx = 0;
            } else {
                idx = 1;
            }
            if (f90[2] < f90[idx]) {
                idx = 2;
            }
            if (f90[idx] >= numTriangles) {
                return false;
            } else {
                SetSplitAxis((SplitPlaneType)idx);
                SetSplitValue(f80[idx]);
                return true;
            }
        }

        void Pack(
            SplitPlaneType splitType,
            const Box &inDimensions,
            std::list<Triangle *> &inTriList,
            kdTreeNode *inRoot,
            unsigned char depth
        ) {
            if (depth < 0xF && inTriList.size() >= 10) {
                bool find = false;
                if (splitType == kSplitPlane_Mean) {
                    find = FindSplit_Mean(inDimensions, inTriList);
                } else if (splitType == kSplitPlane_Median) {
                    find = FindSplit_Mean(inDimensions, inTriList);
                } else if (splitType == kSplitPlane_SAH) {
                    find = FindSplit_SAH(inDimensions, inTriList);
                } else {
                    TheDebugFailer << MakeStringNotInlined("Invalid split plane type");
                }
                if (find && GetSplitValue() >= inDimensions.mMin[GetSplitAxis()]
                    && GetSplitValue() <= inDimensions.mMax[GetSplitAxis()]) {
                    Box boxe0, boxc0;
                    boxe0.Set(inDimensions.mMin, inDimensions.mMax);
                    boxc0.Set(boxe0.mMin, boxe0.mMax);
                    boxe0.mMax[GetSplitAxis()] = GetSplitValue();
                    boxc0.mMin[GetSplitAxis()] = GetSplitValue();
                    std::list<Triangle *> listf0;
                    std::list<Triangle *> listf8;
                    auto it = inTriList.begin();
                    bool b10 = true;
                    while (it != inTriList.end()) {
                        Triangle *pCurr = *it;
                        MILO_ASSERT(::Intersect(*pCurr, inDimensions), 0x166);
                        bool intersecte0 = ::Intersect(*pCurr, boxe0);
                        bool intersectc0 = ::Intersect(*pCurr, boxc0);
                        if (!intersecte0 && !intersectc0) {
                            b10 = false;
                            break;
                        }
                        ++it;
                        if (intersecte0) {
                            listf0.push_back(pCurr);
                        }
                        if (intersectc0) {
                            listf8.push_back(pCurr);
                        }
                    }
                    if (b10 && GetIndex() <= 0x3FFE) {
                        inTriList.clear();
                        SetIsLeaf(0);
                        kdTreeNode *nodea8 = nullptr;
                        kdTreeNode *nodea4 = nullptr;
                        nodea8 = GetChild_0(inRoot);
                        nodea4 = GetChild_1(inRoot);
                        nodea8->Pack(splitType, boxe0, listf0, inRoot, depth + 1);
                        nodea4->Pack(splitType, boxc0, listf8, inRoot, depth + 1);
                        return;
                    }
                }
            }
            MILO_ASSERT(GetIsLeaf(), 0x19F);
            if (inTriList.empty()) {
                mTriList = nullptr;
            } else {
                mTriList = kdTriList::Allocate(inTriList.size());
                kdTriList *pCurr = mTriList;
                for (auto it = inTriList.begin(); it != inTriList.end();) {
                    MILO_ASSERT(!pCurr->IsEnd(), 0x1AE);
                    pCurr->SetData(*it);
                    it = inTriList.erase(it);
                    pCurr = pCurr->GetNext();
                }
            }
        }

        MEM_ARRAY_OVERLOAD(kdTreeNode, 0xEC);

        bool GetIsLeaf() const { return mLeaf_Index >> 15; }
        unsigned short GetIndex() const { return mLeaf_Index & 0x7FFF; }
        unsigned int GetSplitAxis() const { return mSplitAxis & 0x3; }
        float GetSplitValue() const { return mSplitValue; }
        void SetIsLeaf(unsigned int leaf) {
            mLeaf_Index = (mLeaf_Index & 0x7FFF) | (leaf << 15);
        }
        void SetSplitAxis(SplitPlaneType t) {
            mSplitAxis = (mSplitAxis & 0xfffffffc) | t;
        }
        void SetSplitValue(float value) {
            unsigned int oldAxis = GetSplitAxis();
            mSplitValue = value;
            mSplitAxis = (mSplitAxis & 0xfffffffc) | (oldAxis & 3);
        }

        // from RB3 bank 5
        kdTreeNode *GetChild_0(kdTreeNode *n) { return &n[GetIndex() * 2 + 1]; }
        kdTreeNode *GetChild_1(kdTreeNode *n) { return &n[GetIndex() * 2 + 2]; }

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
        for (unsigned short i = 0; i < 0x8000; i++) {
            kdTreeNode &node = mNodeArray[i];
            node.mLeaf_Index = (node.mLeaf_Index & 0x8000) | (i & 0x7FFF);
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
