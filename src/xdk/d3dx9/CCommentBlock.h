#pragma once

#include "types.h"
#include "win_types.h"

namespace D3DXShader {
    enum SegmentType {
        CBST_DWORDARRAY = 0x0000,
        CBST_STRING = 0x0001,
        CBST_SHADER_CONSTANTINFO = 0x0002,
        CBST_SHADER_CONSTANTTABLE = 0x0003,
        CBST_SHADER_TYPEINFO = 0x0004,
        CBST_SHADER_INSTRUCTIONINFO = 0x0005,
        CBST_SHADER_STRUCTMEMBERINFO = 0x0006,
        CBST_SHADER_VARIABLEINFO = 0x0007,
        CBST_SHADER_WRITEINFO = 0x0008,
        CBST_SHADER_DEBUGINFO = 0x0009,
        CBST_SHADER_FILEINFO = 0x000a,
    };

    struct D3DXCB_SEGMENT { /* Size=0x18 */
        const void *pv; // 0x00
        uint cb; // 0x04
        uint Flags; // 0x08
        uint Offset; // 0x0c
        D3DXCB_SEGMENT *pNext; // 0x10
        SegmentType Type; // 0x14
    };

    class CCommentBlock { /* Size=0x10 */
    public:
        CCommentBlock(DWORD dwFourCC);
        ~CCommentBlock();
        HRESULT Add(const void *, uint, uint, uint *, SegmentType);
        HRESULT WriteComment(DWORD *, uint, int);
        HRESULT WriteSwappedComment(DWORD *, uint);
        uint SizeInDwords();

    protected:
        HRESULT WriteOrderedComment(DWORD *, uint, int);
        void SwapBytes(unsigned char *, D3DXCB_SEGMENT *);

        static void SwapWORD(unsigned char *);
        static void SwapDWORD(unsigned char *);
        static void SwapDWORDArray(unsigned char *, uint);

        DWORD m_dwFourCC; // 0x0
        uint m_cbSegments; // 0x4
        D3DXCB_SEGMENT *m_pSegment; // 0x8
        D3DXCB_SEGMENT **m_ppSegment; // 0xc
    };
}
