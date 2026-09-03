#pragma once

#include "d3dx9/CArgument.h"
#include "d3dx9/CInstruction.h"
#include "d3dx9/CPool.h"
#include "xapilibi/xbox.h"

namespace D3DXShader {
    class CBaseProgram { /* Size=0x28 */
    public:
        CBaseProgram();
        virtual ~CBaseProgram();
        HRESULT Initialize(CBaseProgram *);
        uint GetNumPools();
        uint GetNumArguments();
        uint GetNumInstructions();
        uint AddPool(const char *, DWORD, uint, uint);
        uint AddPool(CPool *);
        uint AddArgument(uint, uint, uint, double);
        uint AddArgument(CArgument *);
        uint AddInstruction(DWORD, uint, uint);
        uint AddInstruction(CInstruction *);
        uint CopyPool(CPool *);
        uint CopyArgument(CArgument *);
        uint CopyInstruction(CInstruction *);
        CPool *GetPool(uint uIndex) { return m_ppPools[uIndex]; }
        CArgument *GetArgument(uint uIndex) { return m_ppArgs[uIndex]; }
        CInstruction *GetInstruction(uint uIndex) { return m_ppInsts[uIndex]; }
        void DumpState(const char *, CBaseProgram *);

        static void *operator new(size_t siz) { return XMemAlloc(siz, 0x24810000); }
        static void operator delete(void *ptr) { XMemFree(ptr, 0x24810000); }

    public:
        uint m_uPools; // 0x04
        uint m_uArgs; // 0x08
        uint m_uInsts; // 0x0c
        CPool **m_ppPools; // 0x10
        CArgument **m_ppArgs; // 0x14
        CInstruction **m_ppInsts; // 0x18
    protected:
        uint m_uPoolsAlloc; // 0x1c
        uint m_uArgsAlloc; // 0x20
        uint m_uInstsAlloc; // 0x24

        LPCSTR GetArgumentType(CArgument *);
        HRESULT CreateArgumentErrorString(CArgument *, LPSTR, int);
    };
}
