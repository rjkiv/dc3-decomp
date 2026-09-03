#include "CProgram.h"
#include "win_types.h"
#include "xapilibi/winerror.h"
#include "xapilibi/xbox.h"
#include <cstring>

namespace D3DXShader {
#define CHECK_HRESULT(hr)                                                                \
    {                                                                                    \
        HRESULT hr_ = (hr);                                                              \
        if (hr_ != S_OK && hr_ < 0)                                                      \
            return hr_;                                                                  \
    }

    CProgram::CProgram(int swapBytes) : m_bSwapBytes(swapBytes) {
        m_pProgram = 0;
        m_pErrors = 0;
        m_Version = 0;
        m_Flags = 0;
        m_pName = 0;
        m_pPreShader = nullptr;
        m_bOptimized = 0;
        m_bVectorized = 0;
        m_uPoolVoid = -1;
        m_uPoolLiteral = -1;
        m_uPoolConstant = -1;
        m_uPoolInput = -1;
        m_uPoolOutput = -1;
        m_uPoolResult = -1;
        m_uPoolAddress = -1;
        m_uPoolPredicate = -1;
        m_uPoolObject = -1;
        m_uPoolIncomplete = -1;
        m_uPoolBool = -1;
        m_uPoolBlock = -1;
        m_uPoolClip = -1;
        m_uPoolCycle = -1;
        m_uPoolLoop = -1;
        m_uPoolOutputBool = -1;
        m_uPoolOutputInt = -1;
    }

    HRESULT CProgram::InitCaps() {
        m_Caps.ResultRegs = m_Caps.InputRegs = 0x40;
        m_Caps.AddressRegs = 0;
        m_Caps.ConstantRegs = 0x2000;
        m_Caps.bNoModLim = false;
        m_Caps.bIngoreWriteInfo = false;
        m_Caps.bCoIssue = true;
        return S_OK;
    }

    uint CProgram::MergeInstructions_Input(
        D3DXShader::CInstruction *pInst, uint uParam, uint uIndex, int bSymetric
    ) {
        uint new_input_ct = pInst->m_uOutputs;
        if (bSymetric != 0) {
            new_input_ct += uIndex;
            uint in2 = pInst->m_pInputs[new_input_ct];
            new_input_ct = pInst->m_pInputs[uIndex];

            if (uParam != 0) {
                if (new_input_ct <= in2) {
                    new_input_ct = in2;
                }
            } else if (new_input_ct > in2) {
                new_input_ct = in2;
            }
        } else {
            new_input_ct = pInst->m_pInputs[pInst->m_uOutputs * uParam + uIndex];
        }
        auto arg = m_ppArgs[new_input_ct];
        while (new_input_ct != arg->m_uRemap) {
            new_input_ct = arg->m_uRemap;
            arg = m_ppArgs[new_input_ct];
        }
        return new_input_ct;
    }

    int CProgram::CanVectorize_IsSameRegister(uint iArgA, uint iArgB) {
        if (m_ppArgs[iArgA]->m_uPool == m_ppArgs[iArgB]->m_uPool
            && m_ppArgs[iArgA]->m_uAddress == m_ppArgs[iArgB]->m_uAddress
            && m_ppArgs[iArgA]->m_uRegister == m_ppArgs[iArgB]->m_uRegister) {
            return true;
        }
        return false;
    }

    HRESULT CProgram::IdentityRemap() {
        for (int i = 0; i < m_uArgs; i++) {
            m_ppArgs[i]->m_uRemap = i;
        }
        return S_OK;
    }

    static int InstructionIsSubset(uint iInst1, uint iInst2, const void *pContext) {}
    static int
    CompareDuplicateInstructionsStable(uint iInst1, uint iInst2, const void *pContext) {
        int subset1 = InstructionIsSubset(iInst1, iInst2, pContext),
            subset2 = InstructionIsSubset(iInst1, iInst2, pContext);
    }

    CProgram::~CProgram() {
        XMemFree(m_pName, 0x24810000);
        // delete m_pPreShader;
    }

    HRESULT CProgram::SetName(const char *pName) {
        char *new_str = nullptr;
        if (pName != nullptr) {
            auto len = strlen(pName) + 1;
            new_str = static_cast<char *>(XMemAlloc(len, 0x24810000));
            if (new_str == nullptr) {
                return E_OUTOFMEMORY;
            }
            memcpy(new_str, pName, len);
        }
        XMemFree(m_pName, 0x24810000);
        m_pName = new_str;
        return S_OK;
    }

    HRESULT CProgram::Vectorize() {
        CHECK_HRESULT(VectorizeLeft());
        CHECK_HRESULT(VectorizeRight());
        CHECK_HRESULT(SplitRegisters(1));
        for (uint i = 0; i < 0x100; i++) {
            HRESULT hr_ = (RemoveDuplicateInstructions(1));
            if (hr_ == S_OK)
                continue;
            if (hr_ < 0)
                return hr_;
            if (hr_ == S_FALSE)
                break;
        }
        CHECK_HRESULT(PropagateSwizzles());
        CHECK_HRESULT(CombineInstructions());
        CHECK_HRESULT(Translate());
        CHECK_HRESULT(SplitRegisters(0));
        CHECK_HRESULT(VectorizeLiterals());
        m_bVectorized = true;
        CHECK_HRESULT(ReorderInstructions());
        CHECK_HRESULT(SeparatePhases());
        CHECK_HRESULT(PropagateMovs());
        CHECK_HRESULT(Reschedule());
        CHECK_HRESULT(Constrain());
        CHECK_HRESULT(CompactInstructions());
        CHECK_HRESULT(CompactArguments());
        CHECK_HRESULT(CompactRegisters());
        CHECK_HRESULT(SwizzleRegisters());
        return S_OK;
    }

    HRESULT CProgram::GenerateCode(ID3DXBuffer **ppCode) {
        m_bOptimized = false;
        m_bVectorized = false;
        if (ppCode != nullptr) {
            *ppCode = nullptr;
        }
        CHECK_HRESULT(Optimize());
        CHECK_HRESULT(Vectorize());
        return S_OK;
    }
}
