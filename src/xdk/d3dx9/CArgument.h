#pragma once

#include "d3dx9/CAlloc.h"
#include "d3dx9/CNode.h"
#include "win_types.h"

namespace D3DXShader {

    class CArgument { /* Size=0x80 */
    public:
        uint m_dwFlags; // 0x00
        uint m_uPool; // 0x04
        uint m_uAddress; // 0x08
        uint m_uRegister; // 0x0c
        uint m_uComponent; // 0x10
        uint m_uPredicate; // 0x14
        int m_bPredicate; // 0x18
        double m_fValue; // 0x20
        double m_fValueMax; // 0x28
        uint m_uRemap; // 0x30
        uint m_uAlias; // 0x34
        uint m_uOrigin; // 0x38
        uint m_uModifier; // 0x3c
        uint m_uPhase; // 0x40
        uint m_uLink; // 0x44
        uint m_uWrite; // 0x48
        uint m_uWriteMin; // 0x4c
        uint m_uWriteLim; // 0x50
        uint m_uRead; // 0x54
        uint m_uReadMax; // 0x58
        uint m_uReadCount; // 0x5c
        D3DXShader::CNode *m_pVariable; // 0x60
        uint m_uVariableIndex; // 0x64
        D3DXShader::CNode *m_pSemantic; // 0x68
        uint m_uSemantic; // 0x6c
        uint m_uSemanticIndex; // 0x70
        uint m_uInstruction; // 0x74

        CArgument() {}
        ~CArgument();
        HRESULT Initialize(D3DXShader::CArgument *pArgument);
        HRESULT Initialize(uint uPool, uint uRegister, uint uComponent, double fValue);
        HRESULT Instance(D3DXShader::CArgument *pArgument);

        static void SetAlloc(D3DXCore::CAlloc *pAlloc);
        static void *operator new(size_t);
        static void operator delete(void *);

    private:
        D3DXShader::CArgument *m_pFree; // 0x78
    };
}
