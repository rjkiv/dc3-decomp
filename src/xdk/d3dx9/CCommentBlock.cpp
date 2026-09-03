#include "CCommentBlock.h"
#include "xapilibi/winerror.h"
#include "xapilibi/xbox.h"

namespace D3DXShader {
    CCommentBlock::CCommentBlock(DWORD dwFourCC) : m_dwFourCC(dwFourCC) {
        m_cbSegments = 0;
        m_ppSegment = &m_pSegment;
        m_pSegment = nullptr;
    }

    uint D3DXShader::CCommentBlock::SizeInDwords() {
        return (m_cbSegments + 3) / sizeof(DWORD) + 2;
    }

    CCommentBlock::~CCommentBlock() {
        while (m_pSegment) {
            auto old_one = m_pSegment;
            m_pSegment = m_pSegment->pNext;
            if ((old_one->Flags & 8) || !(old_one->Flags & 1)) {
                XMemFree(const_cast<void *>(old_one->pv), 0x24810000);
            }
            XMemFree(old_one, 0x24810000);
        }
    }

    HRESULT
    CCommentBlock::WriteOrderedComment(DWORD *pdw, uint cdw, int EnableEndianSwap) {
        uint siz = SizeInDwords();
        uint more_size;
        if (cdw == -1 || siz >= cdw) {
            if (siz > 0x8000) {
                return E_FAIL;
            }
            more_size = 0;
            pdw += 2;
            DWORD size_pack = siz - 1;
            size_pack = (size_pack & 0x7FFF) << 16;
            size_pack |= 0xFFFE;
            pdw[0] = size_pack;
            pdw[1] = m_dwFourCC;
        } else {
            return E_FAIL;
        }
        auto segment = m_pSegment;
        char *pb = reinterpret_cast<char *>(pdw);
        while (segment) {
            if (!(segment->Flags & 4)) {
                // pad
                int roundsiz = ROUNDUP(more_size, 4) - more_size;
                memset(pdw, 0xAB, roundsiz);
                pb += roundsiz;
                more_size += roundsiz;
            }
            memcpy(pb, segment->pv, segment->cb);
            pb += segment->cb;
            more_size += segment->cb;
            segment = segment->pNext;
        }
        memset(pdw, 0xAB, more_size - (siz - 2) * sizeof(DWORD));
        return S_OK;
    }

    HRESULT CCommentBlock::WriteComment(DWORD *pdw, uint cdw, int swapBytes) {
        return WriteOrderedComment(pdw, cdw, swapBytes);
    }

    HRESULT CCommentBlock::WriteSwappedComment(DWORD *pdw, uint cdw) {
        return WriteOrderedComment(pdw, cdw, 1);
    }
}
