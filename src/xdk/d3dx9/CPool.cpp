#include "CPool.h"
#include "xapilibi/winerror.h"

namespace D3DXShader {
    HRESULT
    CPool::Initialize(
        const char *pName, DWORD dwFlags, uint uRegisterLim, uint uComponentLim
    ) {
        if (uComponentLim > 4) {
            uComponentLim = 4;
        }
        m_pName = pName;
        m_dwFlags = dwFlags;
        m_uRegisterLim = uRegisterLim;
        m_uComponentLim = uComponentLim;
        m_uRegister = 0;
        m_uComponent = 0;
        m_uRemap = -1;
        m_pMap = nullptr;
        m_pRWMap = nullptr;
        m_uWrite = -1;
        m_uRead = -1;
        m_uReadMax = 0;
        m_uReadCount = 0;
        return 0;
    }
    HRESULT CPool::Initialize(D3DXShader::CPool *pPool) {
        if (pPool == nullptr) {
            return E_FAIL;
        }
        m_pName = pPool->m_pName;
        m_dwFlags = pPool->m_dwFlags;
        m_uRegisterLim = pPool->m_uRegisterLim;
        m_uComponentLim = pPool->m_uComponentLim;
        m_uRegister = pPool->m_uRegister;
        m_uComponent = pPool->m_uComponent;
        m_uRemap = pPool->m_uRemap;
        m_pMap = pPool->m_pMap;
        m_pRWMap = pPool->m_pRWMap;
        m_uWrite = pPool->m_uWrite;
        m_uRead = pPool->m_uRead;
        m_uReadMax = pPool->m_uReadMax;
        m_uReadCount = pPool->m_uReadCount;
        return 0;
    }
}
