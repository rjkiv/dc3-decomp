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
        long Initialize(u32, uint, uint, int);
        long Instance(D3DXShader::CInstruction *);
        uint GetSize();
        uint GetParameters();
        uint GetInputs(uint, uint **);
        uint GetOutputs(uint, uint **);
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

        static uint *Alloc(u32);
        static void SetAlloc(D3DXCore::CAlloc *);
        static void *operator new(size_t);
        static void operator delete(void *);

    public:
        uint m_Opcode; // 0x00
        uint m_uInputs; // 0x04
        uint *m_pInputs; // 0x08
        uint m_uOutputs; // 0x0c
        uint *m_pOutputs; // 0x10
        uint m_uParents; // 0x14
        uint *m_pParents; // 0x18
        uint m_uChildren; // 0x1c
        uint *m_pChildren; // 0x20
        uint m_uMark; // 0x24
        uint m_uMark2; // 0x28
        uint m_uPhase; // 0x2c
        uint m_uRemap; // 0x30
        uint m_uBlock; // 0x34
        uint m_uNesting; // 0x38
        D3DXShader::CNode *m_pExpression; // 0x3c
    private:
        uint m_pInputsDefault[8]; // 0x40
        uint m_pOutputsDefault[4]; // 0x60
        D3DXShader::CInstruction *m_pFree; // 0x70
    };
}
