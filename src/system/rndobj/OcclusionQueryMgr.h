#pragma once
#include "os/Debug.h"

class RndOcclusionQueryMgr {
public:
    enum QueryState {
        kQueryStateInvalid = 0,
        kQueryStateReady = 1,
        kQueryStateIssued = 2,
        kQueryStateFinished = 3,
    };

    RndOcclusionQueryMgr() {
        for (unsigned int i = 0; i < kMaxQueries; i++) {
            for (int j = 0; j < 2; j++) {
                mCachedResults[i][j] = 0;
                mQueryFrameID[i][j] = 0;
                mQueryState[i][j] = kQueryStateInvalid;
            }
        }
        mCurrentFrameIndex = 0;
        mCurrentFrameID = 0;
        mCurrentQueryIndex = 0;
    }

    virtual ~RndOcclusionQueryMgr() {}

    void BeginFrame() {
        mCurrentFrameID++;
        OnBeginFrame();
    }

    void EndFrame() {
        mCurrentFrameIndex = (mCurrentFrameIndex - 1) & 1;
        OnEndFrame();
    }

    // public: bool CreateAndBeginQuery(uint32_t&);

    void ReleaseQuery(unsigned int queryIndex) {
        if (queryIndex < kMaxQueries) {
            for (unsigned int i = 0; i < 2; i++) {
                OnReleaseQuery(queryIndex, i);
                mCachedResults[queryIndex][i] = 0;
                mQueryState[queryIndex][i] = kQueryStateInvalid;
                mQueryFrameID[queryIndex][i] = 0;
            }
        }
    }
    void ReleaseQueries() {
        for (unsigned int i = 0; i < kMaxQueries; i++) {
            ReleaseQuery(i);
        }
    }
    bool GetQueryResults(unsigned int queryIndex, unsigned int &queryResult) {
        if (IsValidQuery(queryIndex)) {
            MILO_ASSERT(GetQueryState(queryIndex) == kQueryStateFinished, 0x8F);
            SetQueryState(queryIndex, kQueryStateReady);
            bool ret = OnGetQueryResult(queryIndex, queryResult);
            if (ret) {
                mCachedResults[queryIndex][mCurrentFrameIndex] = queryResult;
            }
            return ret;
        } else {
            return false;
        }
    }
    bool CreateQuery(unsigned int &queryIndex) {
        unsigned int newIndex = mCurrentQueryIndex;
        if (GetQueryFrameID(newIndex) < mCurrentFrameID) {
            OnCreateQuery(newIndex);
            MILO_ASSERT(GetQueryState(newIndex) == kQueryStateReady, 0x4E);
            queryIndex = newIndex;
            mQueryFrameID[newIndex][mCurrentFrameIndex] = mCurrentFrameID;
            mCurrentQueryIndex = (mCurrentQueryIndex + 1) & (kMaxQueries - 1);
            return true;
        } else {
            queryIndex = -1;
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
    virtual void OnCreateQuery(unsigned int queryIndex) = 0;
    virtual void OnBeginQuery(unsigned int queryIndex) = 0;
    virtual void OnEndQuery(unsigned int queryIndex) = 0;
    virtual bool OnGetQueryResult(unsigned int queryIndex, unsigned int &queryResult) = 0;
    virtual bool OnIsValidQuery(unsigned int queryIndex) const = 0;
    virtual void OnReleaseQuery(unsigned int queryIndex, unsigned int frameIndex) = 0;
    virtual void OnBeginFrame() = 0;
    virtual void OnEndFrame() = 0;

    QueryState GetQueryState(unsigned int queryIndex) const {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xDD);
        return mQueryState[queryIndex][mCurrentFrameIndex];
    }
    void SetQueryState(unsigned int queryIndex, QueryState state) {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xEF);
        mQueryState[queryIndex][mCurrentFrameIndex] = state;
    }
    unsigned int GetQueryFrameID(unsigned int queryIndex) const {
        MILO_ASSERT(queryIndex < kMaxQueries, 0xE4);
        return mQueryFrameID[queryIndex][mCurrentFrameIndex];
    }
    void SetQueryFrameID(unsigned int queryIndex, unsigned int frameID) {
        // probably an assert here too
        mQueryFrameID[queryIndex][mCurrentFrameIndex] = frameID;
    }
    bool IsValidQuery(unsigned int queryIndex) const {
        return queryIndex < kMaxQueries && OnIsValidQuery(queryIndex);
    }
    unsigned int GetCurrentFrameIndex() const { return mCurrentFrameIndex; }
    unsigned int GetCurrentQueryIndex() const { return mCurrentQueryIndex; }

    static const unsigned int kMaxQueries = 256;
    // static const unsigned int kMaxBuffered; // 2?

    QueryState mQueryState[kMaxQueries][2]; // 0x4
    unsigned int mQueryFrameID[kMaxQueries][2]; // 0x804
    unsigned int mCachedResults[kMaxQueries][2]; // 0x1004
    unsigned int mCurrentFrameIndex; // 0x1804
    unsigned int mCurrentFrameID; // 0x1808
    unsigned int mCurrentQueryIndex; // 0x180c
};
