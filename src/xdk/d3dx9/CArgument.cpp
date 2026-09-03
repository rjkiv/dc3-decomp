#include "CArgument.h"
#include "d3dx9/ThreadLocalData.h"
#include "win_types.h"
#include "xapilibi/winerror.h"

namespace D3DXShader {
    HRESULT
    CArgument::Initialize(uint uPool, uint uRegister, uint uComponent, double fValue) {
        m_fValue = fValue;
        m_fValueMax = fValue;
        m_uAddress = -1;
        m_uPool = uPool;
        m_uRegister = uRegister;
        m_uComponent = uComponent;
        m_uPredicate = -1;
        m_bPredicate = 1;
        m_dwFlags = 0;
        m_uRemap = -1;
        m_uAlias = -1;
        m_uOrigin = -1;
        m_uPhase = -1;
        m_uModifier = 0;
        m_uLink = -1;
        m_uWrite = -1;
        m_uWriteMin = -1;
        m_uWriteLim = -1;
        m_uRead = -1;
        m_uReadMax = -1;
        m_uReadCount = 0;
        m_pVariable = nullptr;
        m_uVariableIndex = 0;
        m_pSemantic = nullptr;
        m_uSemantic = -1;
        m_uSemanticIndex = 0;
        m_uInstruction = -1;
        return 0;
    }
    HRESULT CArgument::Initialize(D3DXShader::CArgument *pArgument) {
        if (pArgument == nullptr) {
            return E_FAIL;
        }
        m_uPool = pArgument->m_uPool;
        m_uAddress = pArgument->m_uAddress;
        m_uRegister = pArgument->m_uRegister;
        m_uComponent = pArgument->m_uComponent;
        m_uPredicate = pArgument->m_uPredicate;
        m_bPredicate = pArgument->m_bPredicate;
        m_fValue = pArgument->m_fValue;
        m_fValueMax = pArgument->m_fValueMax;
        m_dwFlags = pArgument->m_dwFlags;
        m_uRemap = pArgument->m_uRemap;
        m_uAlias = pArgument->m_uAlias;
        m_uOrigin = pArgument->m_uOrigin;
        m_uPhase = pArgument->m_uPhase;
        m_uModifier = pArgument->m_uModifier;
        m_uLink = pArgument->m_uLink;
        m_uWrite = pArgument->m_uWrite;
        m_uWriteMin = pArgument->m_uWriteMin;
        m_uWriteLim = pArgument->m_uWriteLim;
        m_uRead = pArgument->m_uRead;
        m_uReadMax = pArgument->m_uReadMax;
        m_uReadCount = pArgument->m_uReadCount;
        m_pVariable = pArgument->m_pVariable;
        m_uVariableIndex = pArgument->m_uVariableIndex;
        m_pSemantic = pArgument->m_pSemantic;
        m_uSemantic = pArgument->m_uSemantic;
        m_uSemanticIndex = pArgument->m_uSemanticIndex;
        m_uInstruction = pArgument->m_uInstruction;
        return 0;
    }
    HRESULT CArgument::Instance(D3DXShader::CArgument *pArgument) {
        if (pArgument == nullptr) {
            return E_FAIL;
        }
        m_pVariable = pArgument->m_pVariable;
        m_uVariableIndex = pArgument->m_uVariableIndex;
        m_pSemantic = pArgument->m_pSemantic;
        m_uSemantic = pArgument->m_uSemantic;
        m_uSemanticIndex = pArgument->m_uSemanticIndex;
        m_uInstruction = pArgument->m_uInstruction;
        return 0;
    }

    void CArgument::SetAlloc(D3DXCore::CAlloc *alloc) {
        ThreadLocalData *tls = GetThreadLocalData();
        tls->CArgument.s_pAlloc = alloc;
        tls->CArgument.s_pFree = nullptr;
    }

    void *CArgument::operator new(size_t siz) {
        ThreadLocalData *tls = GetThreadLocalData();
        if (tls->CArgument.s_pFree) {
            CArgument *old_free = tls->CArgument.s_pFree;
            tls->CArgument.s_pFree = tls->CArgument.s_pFree->m_pFree;
            return old_free;
        }
        return reinterpret_cast<uint *>(tls->CArgument.s_pAlloc->Alloc(siz, 0x10));
    }

    void CArgument::operator delete(void *ptr) {
        if (ptr != nullptr) {
            ThreadLocalData *tls = GetThreadLocalData();
            reinterpret_cast<CArgument *>(ptr)->m_pFree = tls->CArgument.s_pFree;
            tls->CArgument.s_pFree = reinterpret_cast<CArgument *>(ptr);
        }
    }
}
