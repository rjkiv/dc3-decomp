#pragma once

#include "types.h"

namespace D3DXCore {
    class CAlloc { /* Size=0x18 */
    protected:
        unsigned char *m_pbRegion; // 0x00
        u32 m_cbPage; // 0x04
        u32 m_cbAlloc; // 0x08
        u32 m_cbCommit; // 0x0c
        u32 m_cbReserve; // 0x10
        u32 m_cbReserveMin; // 0x14

    public:
        CAlloc(uint cbReserve, uint cbCommit);
        ~CAlloc();
        unsigned char *Alloc(uint cb, uint uAlign);
    };

}
