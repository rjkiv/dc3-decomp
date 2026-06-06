#pragma once
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/OcclusionQueryMgr.h"
#include "xdk/D3D9.h"
#include "xdk/XAPILIB.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

// size 0x2010
class DxRndOcclusionQueryMgr : public RndOcclusionQueryMgr {
public:
    DxRndOcclusionQueryMgr() {
        for (unsigned int i = 0; i < 256; i++) {
            for (int j = 0; j < 2; j++) {
                mDXQueryArray[i][j] = 0;
            }
        }
    }
    virtual ~DxRndOcclusionQueryMgr() { ReleaseQueries(); }

protected:
    virtual void OnCreateQuery(unsigned int queryIndex) {
        GetDXQuery(queryIndex);
        SetQueryState(queryIndex, kQueryStateReady);
    }
    virtual void OnBeginQuery(unsigned int queryIndex) {
        GetDXQuery(queryIndex)->Issue(2);
    }
    virtual void OnEndQuery(unsigned int queryIndex) { GetDXQuery(queryIndex)->Issue(1); }
    virtual bool OnGetQueryResult(unsigned int queryIndex, unsigned int &uiRef) {
        uiRef = 0;
        return GetDXQuery(queryIndex)->GetData(&uiRef, 4, 0) == ERROR_SUCCESS;
    }
    virtual bool OnIsValidQuery(unsigned int queryIndex) const {
        return mDXQueryArray[queryIndex][mCurrentFrameIndex];
    }
    virtual void OnReleaseQuery(unsigned int queryIndex, unsigned int frameIndex) {
        if (mDXQueryArray[queryIndex][frameIndex]) {
            mDXQueryArray[queryIndex][frameIndex]->Release();
            mDXQueryArray[queryIndex][frameIndex] = nullptr;
        }
    }
    virtual void OnBeginFrame() {}
    virtual void OnEndFrame() {}

private:
    D3DQuery *GetDXQuery(unsigned int queryIndex) {
        MILO_ASSERT(queryIndex < kMaxQueries, 0x57);
        if (!mDXQueryArray[queryIndex][mCurrentFrameIndex]) {
            MILO_ASSERT(GetQueryState(queryIndex) == kQueryStateInvalid, 0x5D);
            if (mDXQueryArray[queryIndex][mCurrentFrameIndex]) {
                TheDxRnd.Device()->CreateQueryTiled(
                    D3DQUERYTYPE_OCCLUSION,
                    1,
                    &mDXQueryArray[queryIndex][mCurrentFrameIndex]
                );
            }
            SetQueryState(queryIndex, kQueryStateReady);
        }
        MILO_ASSERT(mDXQueryArray[queryIndex][GetCurrentFrameIndex()] != NULL, 0x67);
        return mDXQueryArray[queryIndex][mCurrentFrameIndex];
    }

    D3DQuery *mDXQueryArray[256][2]; // 0x1810
};
