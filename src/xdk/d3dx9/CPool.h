#pragma once

#include "types.h"
#include "win_types.h"
#include "xapilibi/xbox.h"
#include <cstddef>

namespace D3DXShader {

    class CPool { /* Size=0x34 */
    public:
        const char *m_pName; // 0x00
        DWORD m_dwFlags; // 0x04
        uint m_uRegisterLim; // 0x08
        uint m_uComponentLim; // 0x0c
        uint m_uRegister; // 0x10
        uint m_uComponent; // 0x14
        uint m_uRemap; // 0x18
        uint *m_pMap; // 0x1c
        uint *m_pRWMap; // 0x20
        uint m_uWrite; // 0x24
        uint m_uRead; // 0x28
        uint m_uReadMax; // 0x2c
        uint m_uReadCount; // 0x30

        CPool() {}
        ~CPool();
        HRESULT Initialize(D3DXShader::CPool *pPool);
        HRESULT
        Initialize(
            const char *pName, DWORD dwFlags, uint uRegisterLim, uint uComponentLim
        );

        static void *operator new(size_t siz) { return XMemAlloc(siz, 0x24810000); }
        static void *operator new[](size_t);
        static void operator delete(void *ptr) { XMemFree(ptr, 0x24810000); }
        static void operator delete[](void *);
    };
}
