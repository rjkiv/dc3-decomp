#include "CBaseProgram.h"
#include "d3dx9/CArgument.h"
#include "d3dx9/CInstruction.h"
#include "d3dx9/CPool.h"
#include "win_types.h"
#include "xapilibi/strsafe.h"
#include "xapilibi/winerror.h"
#include "xapilibi/xbox.h"
#include <cmath>
#include <cstring>

namespace D3DXShader {
    CBaseProgram::CBaseProgram() {
        m_uPools = 0;
        m_uPoolsAlloc = 0;
        m_ppPools = nullptr;
        m_uArgs = 0;
        m_uArgsAlloc = 0;
        m_ppArgs = nullptr;
        m_uInsts = 0;
        m_uInstsAlloc = 0;
        m_ppInsts = nullptr;
    }

    HRESULT CBaseProgram::Initialize(D3DXShader::CBaseProgram *pProgram) {
        m_uPools = 0;
        m_uPoolsAlloc = pProgram->m_uPools;
        m_ppPools = new CPool *[m_uPoolsAlloc > 0x3FFFFFFF ? -1 : m_uPoolsAlloc];
        if (m_ppPools == nullptr) {
            return E_OUTOFMEMORY;
        }
        for (int i = 0; i < pProgram->m_uPools; i++) {
            if (CopyPool(pProgram->m_ppPools[i]) == -1)
                return E_OUTOFMEMORY;
        }
        m_uArgs = 0;
        m_uArgsAlloc = pProgram->m_uArgs;
        m_ppArgs = new CArgument *[m_uArgsAlloc > 0x3FFFFFFF ? -1 : m_uArgsAlloc];
        if (m_ppArgs == nullptr) {
            return E_OUTOFMEMORY;
        }
        for (int i = 0; i < pProgram->m_uArgs; i++) {
            if (CopyArgument(pProgram->m_ppArgs[i]) == -1)
                return E_OUTOFMEMORY;
        }
        m_uInsts = 0;
        m_uInstsAlloc = pProgram->m_uInsts;
        m_ppInsts = new CInstruction *[m_uInstsAlloc > 0x3FFFFFFF ? -1 : m_uInstsAlloc];
        if (m_ppInsts == nullptr) {
            return E_OUTOFMEMORY;
        }
        for (int i = 0; i < pProgram->m_uInsts; i++) {
            if (CopyInstruction(pProgram->m_ppInsts[i]) == -1)
                return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    CBaseProgram::~CBaseProgram() {
        if (m_ppPools != nullptr) {
            // i think this'll leak if there are allocated but unused things
            for (int i = 0; i < m_uPools; i++) {
                delete m_ppPools[i];
            }
            XMemFree(m_ppPools, 0x24810000);
        }
        if (m_ppArgs != nullptr) {
            // i think this'll leak if there are allocated but unused things
            for (int i = 0; i < m_uArgs; i++) {
                delete m_ppArgs[i];
            }
            XMemFree(m_ppArgs, 0x24810000);
        }
        if (m_ppInsts != nullptr) {
            // i think this'll leak if there are allocated but unused things
            for (int i = 0; i < m_uInsts; i++) {
                delete m_ppInsts[i];
            }
            XMemFree(m_ppInsts, 0x24810000);
        }
    }

    LPCSTR CBaseProgram::GetArgumentType(D3DXShader::CArgument *pArg) {
        const char *ret = "input";
        if (GetPool(pArg->m_uPool)->m_dwFlags & 0x20) {
            ret = "output";
        }

        return ret;
    }

    uint CBaseProgram::AddPool(D3DXShader::CPool *pPool) {
        if (m_uPools == m_uPoolsAlloc) {
            int newct = m_uPoolsAlloc != 0 ? m_uPoolsAlloc * 2 : 16;
            auto new_pools =
                static_cast<CPool **>(XMemAlloc(newct * sizeof(CPool *), 0x24810000));
            if (new_pools == nullptr) {
                if (pPool) {
                    delete pPool;
                }
                return -1;
            }
            memcpy(new_pools, m_ppPools, m_uPools * sizeof(CPool *));
            memset(new_pools + m_uPools, 0, (newct - m_uPools) * sizeof(CPool *));
            XMemFree(m_ppPools, 0x24810000);
            m_ppPools = new_pools;
            m_uPoolsAlloc = newct;
        }
        m_ppPools[m_uPools] = pPool;
        return m_uPools++;
    }

    uint CBaseProgram::AddArgument(D3DXShader::CArgument *pArg) {
        if (m_uArgs == m_uArgsAlloc) {
            int newct = m_uArgsAlloc != 0 ? m_uArgsAlloc * 2 : 1024;
            auto new_args = static_cast<CArgument **>(
                XMemAlloc(newct * sizeof(CArgument *), 0x24810000)
            );
            if (new_args == nullptr) {
                if (pArg) {
                    delete pArg;
                }
                return -1;
            }
            memcpy(new_args, m_ppArgs, m_uArgs * sizeof(CArgument *));
            memset(new_args + m_uArgs, 0, (newct - m_uArgs) * sizeof(CArgument *));
            XMemFree(m_ppArgs, 0x24810000);
            m_ppArgs = new_args;
            m_uArgsAlloc = newct;
        }
        if (pArg != nullptr && (GetPool(pArg->m_uPool)->m_dwFlags & 0x100)
            && pArg->m_uAddress == -1) {
            pArg->m_dwFlags |= 0x80;
            if (pArg->m_fValue == 0 || pArg->m_fValue == 1) {
                pArg->m_dwFlags |= 1;
            }
            // small decimal portion
            s32 integral = pArg->m_fValue;
            if (fabs(pArg->m_fValue - integral) < 0.000001) {
                pArg->m_dwFlags |= 2;
            }
            if (pArg->m_fValue >= 0) {
                pArg->m_dwFlags |= 4;
            }
            if (pArg->m_fValue <= 0) {
                pArg->m_dwFlags |= 8;
            }
            if (fabs(pArg->m_fValue) <= 1) {
                pArg->m_dwFlags |= 0x10;
            }
        }
        m_ppArgs[m_uArgs] = pArg;
        return m_uArgs++;
    }

    uint CBaseProgram::AddInstruction(D3DXShader::CInstruction *pInst) {
        if (m_uInsts == m_uInstsAlloc) {
            int newct = m_uInstsAlloc != 0 ? m_uInstsAlloc * 2 : 256;
            auto new_insts = static_cast<CInstruction **>(
                XMemAlloc(newct * sizeof(CInstruction *), 0x24810000)
            );
            if (new_insts == nullptr) {
                if (pInst) {
                    delete pInst;
                }
                return -1;
            }
            memcpy(new_insts, m_ppInsts, m_uInsts * sizeof(CInstruction *));
            memset(new_insts + m_uInsts, 0, (newct - m_uInsts) * sizeof(CInstruction *));
            XMemFree(m_ppInsts, 0x24810000);
            m_ppInsts = new_insts;
            m_uInstsAlloc = newct;
        }
        m_ppInsts[m_uInsts] = pInst;
        return m_uInsts++;
    }

    uint CBaseProgram::AddPool(
        const char *pName, DWORD dwFlags, uint uRegisterLim, uint uComponentLim
    ) {
        CPool *new_pool = new CPool;
        if (new_pool == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(
                new_pool->Initialize(pName, dwFlags, uRegisterLim, uComponentLim)
            )) {
            delete new_pool;
            return -1;
        }
        return AddPool(new_pool);
    }

    uint
    CBaseProgram::AddArgument(uint uPool, uint uRegister, uint uComponent, double fValue) {
        CArgument *new_arg = new CArgument;
        if (new_arg == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(new_arg->Initialize(uPool, uRegister, uComponent, fValue))) {
            delete new_arg;
            return -1;
        }
        return AddArgument(new_arg);
    }

    unsigned int
    D3DXShader::CBaseProgram::AddInstruction(DWORD Opcode, uint uInputs, uint uOutputs) {
        CInstruction *new_inst = new CInstruction;
        if (new_inst == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(new_inst->Initialize(Opcode, uInputs, uOutputs, 0))) {
            delete new_inst;
            return -1;
        }
        return AddInstruction(new_inst);
    }

    uint CBaseProgram::CopyPool(D3DXShader::CPool *pPool) {
        CPool *new_pool = new CPool;
        if (new_pool == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(new_pool->Initialize(pPool))) {
            delete new_pool;
            return -1;
        }
        return AddPool(new_pool);
    }

    uint CBaseProgram::CopyArgument(D3DXShader::CArgument *pArg) {
        CArgument *new_arg = new CArgument;
        if (new_arg == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(new_arg->Initialize(pArg))) {
            delete new_arg;
            return -1;
        }
        return AddArgument(new_arg);
    }

    uint CBaseProgram::CopyInstruction(D3DXShader::CInstruction *pInst) {
        CInstruction *new_inst = new CInstruction;
        if (new_inst == nullptr) {
            return -1;
        }
        if (!SUCCEEDED(new_inst->Initialize(pInst))) {
            delete new_inst;
            return -1;
        }
        return AddInstruction(new_inst);
    }

    HRESULT CBaseProgram::CreateArgumentErrorString(
        D3DXShader::CArgument *pArg, LPSTR pBuffer, int bufferSize
    ) {
        char buf[0x40];
        // std::strncpy(buf, pArg->m_pSemantic->AsTokenNode()->m_Token.String, 0x40);
        auto input = pArg->m_pSemantic->AsTokenNode()->m_Token.String;
        auto output = buf;
        int i;
        for (i = 0x40; i != 0; i--) {
            if (input[i] == 0)
                break;
            *output++ = *input++;
        }
        if (i == 0) {
        }
        *output = 0;
        StringCbPrintfA(pBuffer, bufferSize, "%s semantic '%s'", GetArgumentType(pArg));
        pBuffer[bufferSize - 1] = 0;
        return S_OK;
    }
}
