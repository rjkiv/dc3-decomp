#pragma once

#include "d3dx9/calloc.h"

namespace D3DXShader {
    class CArgument;
    class CNode;
    class CInstruction;
    class ThreadLocalData { /* Size=0x18 */
        struct _CArgument { /* Size=0x8 */
            D3DXShader::CArgument *s_pFree; // 0x0
            D3DXCore::CAlloc *s_pAlloc; // 0x4
        };

        struct _CInstruction { /* Size=0x8 */
            D3DXShader::CInstruction *s_pFree; // 0x0
            D3DXCore::CAlloc *s_pAlloc; // 0x4
        };

        struct _CNode { /* Size=0x4 */
            D3DXCore::CAlloc *m_pAlloc; // 0x0
        };

    public:
        _CNode CNode; // 0x00
        _CArgument CArgument; // 0x04
        _CInstruction CInstruction; // 0x0c
    private:
        unsigned int m_NestingLevel; // 0x14

        ThreadLocalData();
        ~ThreadLocalData();
    };

    ThreadLocalData *GetThreadLocalData();
}
