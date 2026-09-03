#include "d3dx9/CInstruction.h"
#include "d3dx9/ThreadLocalData.h"
#include "types.h"
#include "win_types.h"
#include <cstring>
#include "xapilibi/winerror.h"

namespace D3DXShader {
    CInstruction::CInstruction() {
        m_Opcode = 0;
        m_uInputs = 0;
        m_pInputs = nullptr;
        m_uOutputs = 0;
        m_pOutputs = nullptr;
        m_uParents = 0;
        m_pParents = nullptr;
        m_uChildren = 0;
        m_pChildren = nullptr;
        m_uMark = 0;
        m_uMark2 = 0;
        m_uPhase = 0;
        m_uRemap = -1;
        m_uBlock = -1;
        m_uNesting = 0;
        m_pExpression = nullptr;
    }

    HRESULT CInstruction::Instance(D3DXShader::CInstruction *insn) {
        if (insn == nullptr) {
            return E_FAIL;
        }
        m_pExpression = insn->m_pExpression;
        return 0;
    }

    uint CInstruction::GetInputs(uint req_inputs, uint **outptr) {
        int opcode_lowpart = m_Opcode & 0x000FFFFF;
        uint whuh = req_inputs * opcode_lowpart;
        if (whuh + opcode_lowpart > m_uInputs) {
            if (outptr != nullptr) {
                *outptr = nullptr;
            }
            return 0;
        }
        if (outptr != nullptr) {
            *outptr = m_pInputs + whuh;
        }
        return opcode_lowpart;
    }

    int CInstruction::IsSpecial() {
        if ((m_Opcode & 0xF0000000) == 0x60000000)
            return true;
        switch (m_Opcode & 0xFFF00000) {
        case 0x50100000:
        case 0x50200000:
        case 0x50300000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsScalar() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x10300000:
        case 0x10500000:
        case 0x10600000:
        case 0x10700000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsSymetric() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x20000000:
        case 0x20100000:
        case 0x20400000:
        case 0x20500000:
        case 0x50000000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsCopy() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x10000000:
        case 0x10100000:
        case 0x11200000:
        case 0x20700000:
        case 0x20800000:
        case 0x20900000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsMacro() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x70700000:
        case 0x50300000:
        case 0x70100000:
        case 0x70200000:
        case 0x70500000:
        case 0x70600000:
        case 0x70B00000:
        case 0x70C00000:
        case 0x70D00000:
        case 0x71000000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsLoop() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x74200000:
        case 0x11500000:
        case 0x11100000:
        case 0x11200000:
        case 0x11300000:
        case 0x11400000:
        case 0x20800000:
        case 0x20900000:
        case 0x74100000:
        case 0x74300000:
        case 0x74400000:
        case 0x74500000:
        case 0x74600000:
        case 0x74700000:
        case 0x74A00000:
        case 0x74B00000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsElse() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x73100000:
        case 0x73300000:
        case 0x73D00000:
        case 0x73E00000:
        case 0x73F00000:
        case 0x74000000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsEndIf() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x20700000:
        case 0x73400000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsFlowControl() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x10F00000:
        case 0x11100000:
        case 0x11200000:
        case 0x11300000:
        case 0x11400000:
        case 0x11500000:
        case 0x20700000:
        case 0x20800000:
        case 0x20900000:
        case 0x73000000:
        case 0x73100000:
        case 0x73200000:
        case 0x73300000:
        case 0x73400000:
        case 0x73500000:
        case 0x73600000:
        case 0x73700000:
        case 0x73800000:
        case 0x73900000:
        case 0x73A00000:
        case 0x73B00000:
        case 0x73C00000:
        case 0x73D00000:
        case 0x73E00000:
        case 0x73F00000:
        case 0x74000000:
        case 0x74100000:
        case 0x74200000:
        case 0x74300000:
        case 0x74400000:
        case 0x74500000:
        case 0x74600000:
        case 0x74700000:
        case 0x74A00000:
        case 0x74B00000:
            return true;
        default:
            return false;
        }
    }

    int CInstruction::IsGradient() {
        switch (m_Opcode & 0xFFF00000) {
        case 0x10D00000:
        case 0x10E00000:
        case 0x60000000:
        case 0x60200000:
        case 0x60300000:
        case 0x60500000:
        case 0x60700000:
        case 0x60800000:
        case 0x60A00000:
        case 0x60C00000:
        case 0x60D00000:
        case 0x60F00000:
        case 0x61100000:
        case 0x61200000:
            return true;
        default:
            return false;
        }
    }

    void CInstruction::SetAlloc(D3DXCore::CAlloc *alloc) {
        ThreadLocalData *tls = GetThreadLocalData();
        tls->CInstruction.s_pAlloc = alloc;
        tls->CInstruction.s_pFree = nullptr;
    }

    unsigned int *CInstruction::Alloc(u32 siz) {
        ThreadLocalData *tls = GetThreadLocalData();
        return reinterpret_cast<uint *>(tls->CInstruction.s_pAlloc->Alloc(siz * 4, 0x10));
    }

    void *CInstruction::operator new(size_t siz) {
        ThreadLocalData *tls = GetThreadLocalData();
        if (tls->CInstruction.s_pFree) {
            CInstruction *old_free = tls->CInstruction.s_pFree;
            tls->CInstruction.s_pFree = tls->CInstruction.s_pFree->m_pFree;
            return old_free;
        }
        return reinterpret_cast<uint *>(tls->CInstruction.s_pAlloc->Alloc(siz, 0x10));
    }

    void CInstruction::operator delete(void *ptr) {
        if (ptr != nullptr) {
            ThreadLocalData *tls = GetThreadLocalData();
            reinterpret_cast<CInstruction *>(ptr)->m_pFree = tls->CInstruction.s_pFree;
            tls->CInstruction.s_pFree = reinterpret_cast<CInstruction *>(ptr);
        }
    }

    // i hate this thing
    HRESULT CInstruction::Initialize(
        u32 Opcode, unsigned int uInputs, unsigned int uOutputs, int bNoCheck
    ) {
        m_Opcode = Opcode;
        m_uInputs = uInputs;
        m_uOutputs = uOutputs;
        if (!bNoCheck) {
        }
        uint cached_ins = m_uInputs;
        if (m_uInputs <= 8) {
            m_pInputs = m_pInputsDefault;
        } else {
            ThreadLocalData *tls = GetThreadLocalData();
            m_pInputs = reinterpret_cast<uint *>(
                tls->CInstruction.s_pAlloc->Alloc(cached_ins * 4, 0x10)
            );
            if (m_pInputs == nullptr) {
                return E_OUTOFMEMORY;
            }
        }
        uint cached_outs = m_uOutputs;
        if (m_uOutputs <= 4) {
            m_pOutputs = m_pOutputsDefault;
        } else {
            ThreadLocalData *tls = GetThreadLocalData();
            m_pOutputs = reinterpret_cast<uint *>(
                tls->CInstruction.s_pAlloc->Alloc(cached_outs * 4, 0x10)
            );
            if (m_pOutputs == nullptr) {
                return E_OUTOFMEMORY;
            }
        }
        memset(m_pInputs, 0xFF, m_uInputs * 4);
        memset(m_pOutputs, 0xFF, m_uOutputs * 4);
        m_uMark = 0;
        m_uMark2 = 0;
        m_uPhase = 0;
        m_uNesting = 0;
        m_pExpression = nullptr;
        return 0;
    }

    HRESULT CInstruction::Initialize(CInstruction *insn) {
        if (insn == nullptr) {
            return E_FAIL;
        }
        m_Opcode = insn->m_Opcode;
        uint cached_ins = insn->m_uInputs;
        if (cached_ins > m_uInputs) {
            if (insn->m_uInputs <= 8) {
                m_pInputs = m_pInputsDefault;
            } else {
                ThreadLocalData *tls = GetThreadLocalData();
                m_pInputs = reinterpret_cast<uint *>(
                    tls->CInstruction.s_pAlloc->Alloc(cached_ins * 4, 0x10)
                );
                if (m_pInputs == nullptr) {
                    return E_OUTOFMEMORY;
                }
            }
        }
        m_uInputs = insn->m_uInputs;
        memcpy(m_pInputs, insn->m_pInputs, m_uInputs * 4);
        uint cached_outs = insn->m_uOutputs;
        if (cached_outs > m_uOutputs) {
            if (insn->m_uOutputs <= 4) {
                m_pOutputs = m_pOutputsDefault;
            } else {
                ThreadLocalData *tls = GetThreadLocalData();
                m_pOutputs = reinterpret_cast<uint *>(
                    tls->CInstruction.s_pAlloc->Alloc(cached_outs * 4, 0x10)
                );
                if (m_pOutputs == nullptr) {
                    return E_OUTOFMEMORY;
                }
            }
        }
        m_uOutputs = insn->m_uOutputs;
        memcpy(m_pOutputs, insn->m_pOutputs, m_uOutputs * 4);
        m_uMark = insn->m_uMark;
        m_uMark2 = insn->m_uMark2;
        m_uRemap = insn->m_uRemap;
        m_pExpression = insn->m_pExpression;
        return 0;
    }

    uint CInstruction::GetOutputs(uint req_outputs, uint **outptr) {
        uint opcode_name = m_Opcode & 0xFFF00000;
        int size;
        switch (opcode_name) {
        case 0:
            size = 0;
            break;
        case 0x50000000:
        case 0x70800000:
            size = m_uOutputs;
            break;
        case 0x50200000:
            size = 1;
            break;
        default: {
            if ((m_Opcode & 0xF0000000) == 0x60000000) {
                size = 4;
                break;
            }
            size = m_Opcode & 0x000FFFFF;
        } break;
        }
        uint whuh = req_outputs * size;
        if (whuh + size > m_uOutputs) {
            if (outptr != nullptr) {
                *outptr = nullptr;
            }
            return 0;
        }
        if (outptr != nullptr) {
            *outptr = m_pOutputs + whuh;
        }
        return size;
    }
}
