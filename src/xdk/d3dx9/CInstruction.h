#pragma once

#include "d3dx9/CAlloc.h"
#include "d3dx9/CNode.h"
#include "types.h"
#include <cstddef>

namespace D3DXShader {

    class CInstruction { /* Size=0x74 */
    public:
        CInstruction();
        ~CInstruction();
        long Initialize(D3DXShader::CInstruction *);
        long Initialize(u32, unsigned int, unsigned int, int);
        long Instance(D3DXShader::CInstruction *);
        unsigned int GetSize();
        unsigned int GetParameters();
        unsigned int GetInputs(unsigned int, unsigned int **);
        unsigned int GetOutputs(unsigned int, unsigned int **);
        int IsNOP();
        int IsMOV();
        int IsNEG();
        int IsDOT();
        int IsCLIP();
        int IsLOOPIN();
        int IsREPEAT();
        int IsUnary();
        int IsBinary();
        int IsTernary();
        int IsQuaternary();
        int IsSpecial();
        int IsTexture();
        int IsScalar();
        int IsSymetric();
        int IsPerComponent();
        int IsCopy();
        int IsMacro();
        int IsFlowControl();
        int IsCombine();
        int IsPartial();
        int IsGradient();
        int IsLoop();
        int IsElse();
        int IsEndIf();

        static unsigned int *Alloc(u32);
        static void SetAlloc(D3DXCore::CAlloc *);
        static void *operator new(size_t);
        static void operator delete(void *);

    public:
        unsigned int m_Opcode; // 0x00
        unsigned int m_uInputs; // 0x04
        unsigned int *m_pInputs; // 0x08
        unsigned int m_uOutputs; // 0x0c
        unsigned int *m_pOutputs; // 0x10
        unsigned int m_uParents; // 0x14
        unsigned int *m_pParents; // 0x18
        unsigned int m_uChildren; // 0x1c
        unsigned int *m_pChildren; // 0x20
        unsigned int m_uMark; // 0x24
        unsigned int m_uMark2; // 0x28
        unsigned int m_uPhase; // 0x2c
        unsigned int m_uRemap; // 0x30
        unsigned int m_uBlock; // 0x34
        unsigned int m_uNesting; // 0x38
        D3DXShader::CNode *m_pExpression; // 0x3c
    private:
        unsigned int m_pInputsDefault[8]; // 0x40
        unsigned int m_pOutputsDefault[4]; // 0x60
        D3DXShader::CInstruction *m_pFree; // 0x70
    };
}
