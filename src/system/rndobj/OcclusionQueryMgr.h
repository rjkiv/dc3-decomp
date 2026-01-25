#pragma once
#include "os/Debug.h"

class RndOcclusionQueryMgr {
public:
    enum QueryState {
        kQueryStateInvalid = 0,
        kQueryStateReady = 1,
        kQueryStateIssued = 2,
        kQueryStateFinished = 3,
        kMaxQueries = 256
    };

    RndOcclusionQueryMgr() {
        for (unsigned int i = 0; i < kMaxQueries; i++) {
            for (int j = 0; j < 2; j++) {
                unk1004[i][j] = 0;
                mQueryFrameIDs[i][j] = 0;
                mQueryStates[i][j] = kQueryStateInvalid;
            }
        }
        mCurrentFrameIndex = 0;
        unk1808 = 0;
        unk180c = 0;
    }

    virtual ~RndOcclusionQueryMgr() {}
    virtual void OnCreateQuery(unsigned int) = 0;
    virtual void OnBeginQuery(unsigned int) = 0;
    virtual void OnEndQuery(unsigned int) = 0;
    virtual bool OnGetQueryResult(unsigned int, unsigned int &) = 0;
    virtual bool OnIsValidQuery(unsigned int) const = 0;
    virtual void OnReleaseQuery(unsigned int, unsigned int) = 0;
    virtual void OnBeginFrame() = 0;
    virtual void OnEndFrame() = 0;

    void ReleaseQuery(unsigned int idx) {
        if (idx < kMaxQueries) {
            for (unsigned int i = 0; i < 2; i++) {
                OnReleaseQuery(idx, i);
                unk1004[idx][i] = 0;
                mQueryStates[idx][i] = kQueryStateInvalid;
                mQueryFrameIDs[idx][i] = 0;
            }
        }
    }
    void ReleaseQueries() {
        for (unsigned int i = 0; i < kMaxQueries; i++) {
            ReleaseQuery(i);
        }
    }
    bool GetQueryResults(unsigned int queryIndex, unsigned int &uiRef) {
        if (IsValidQuery(queryIndex)) {
            MILO_ASSERT(GetQueryState(queryIndex) == kQueryStateFinished, 0x8F);
            SetQueryState(queryIndex, kQueryStateReady);
            bool ret = OnGetQueryResult(queryIndex, uiRef);
            if (ret) {
                unk1004[queryIndex][mCurrentFrameIndex] = uiRef;
            }
            return ret;
        } else {
            return false;
        }
    }
    bool CreateQuery(unsigned int &uiRef) {
        unsigned int newIndex = unk180c;
        if (GetQueryFrameID(newIndex) < unk1808) {
            OnCreateQuery(newIndex);
            MILO_ASSERT(GetQueryState(newIndex) == kQueryStateReady, 0x4E);
            uiRef = newIndex;
            mQueryFrameIDs[newIndex][mCurrentFrameIndex] = unk1808;
            unk180c = (unk180c + 1) & (kMaxQueries - 1);
            return true;
        } else {
            uiRef = -1;
            return false;
        }
    }
    void BeginQuery(unsigned int queryIndex) {
        if (IsValidQuery(queryIndex)) {
            MILO_ASSERT(GetQueryState(queryIndex) == kQueryStateReady, 0x66);
            SetQueryState(queryIndex, kQueryStateIssued);
            OnBeginQuery(queryIndex);
        }
    }
    void EndQuery(unsigned int queryIndex) {
        if (IsValidQuery(queryIndex)) {
            MILO_ASSERT(GetQueryState(queryIndex) == kQueryStateIssued, 0x7F);
            SetQueryState(queryIndex, kQueryStateFinished);
            OnEndQuery(queryIndex);
        }
    }

protected:
    QueryState GetQueryState(unsigned int queryIndex) const {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xDD);
        return mQueryStates[queryIndex][mCurrentFrameIndex];
    }
    void SetQueryState(unsigned int queryIndex, QueryState state) {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xEF);
        mQueryStates[queryIndex][mCurrentFrameIndex] = state;
    }
    unsigned int GetQueryFrameID(unsigned int queryIndex) const {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xE4);
        return mQueryFrameIDs[queryIndex][mCurrentFrameIndex];
    }
    bool IsValidQuery(unsigned int queryIndex) const {
        return queryIndex < kMaxQueries && OnIsValidQuery(queryIndex);
    }
    unsigned int GetCurrentFrameIndex() const { return mCurrentFrameIndex; }

public:
    void ToggleFrameIndex() { mCurrentFrameIndex = (mCurrentFrameIndex - 1) & 1; }
    void IncrementFrameCounter() { unk1808++; }

protected:
    QueryState mQueryStates[kMaxQueries][2]; // 0x4
    unsigned int mQueryFrameIDs[kMaxQueries][2]; // 0x804
    unsigned int unk1004[kMaxQueries][2]; // 0x1004
    unsigned int mCurrentFrameIndex; // 0x1804
    unsigned int unk1808; // 0x1808
    unsigned int unk180c; // 0x180c
};
