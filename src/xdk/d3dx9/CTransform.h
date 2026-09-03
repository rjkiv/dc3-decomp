#pragma once

#include "d3dx9/CInstruction.h"
#include "d3dx9/CNode.h"
#include "d3dx9/d3dx9mesh.h"
#include "types.h"
namespace D3DXShader {
    class CTransform {
    public:
        CTransform(class CProgram *);
        virtual ~CTransform();
        virtual long Apply();

    protected:
        class CProgram *m_pProgram; // 0x4
    };

    class CTReorderInstructions : public CTransform {
    public:
        CTReorderInstructions(D3DXShader::CProgram *);
        virtual long Apply(int);

    protected:
        long Reduce();
        long Bubble(uint);
        void Read(uint, uint);
        void Write(uint, uint);
        void UpdateRead(uint, uint);
        void UpdateWrite(uint, uint);
        void RemoveDuplicates(uint *, uint *);
        void ReverseInsts(uint, uint);
        void RecalculateLoad(uint *, uint *);

        static int CompareLoad(uint, uint, const void *);
        static int CompareIndex(uint, uint, const void *);

        uint *m_pInputsCount; // 0x08
        uint *m_pInputsOffset; // 0x0c
        uint *m_pInputs; // 0x10
        uint *m_pInputsA; // 0x14
        uint *m_pInputsB; // 0x18
        uint *m_pOutputsCount; // 0x1c
        uint *m_pOutputsOffset; // 0x20
        uint *m_pOutputs; // 0x24
        int *m_pBouyancy; // 0x28
        uint *m_pLoad; // 0x2c
        uint *m_pPriority; // 0x30
        int *m_pFlowControl; // 0x34
        uint m_uInsts; // 0x38
        uint *m_pInsts; // 0x3c
        uint *m_pInstsOld; // 0x40
        D3DXShader::CInstruction **m_ppInsts; // 0x44
        uint m_uMark; // 0x48
        uint m_uMaxLoad; // 0x4c
        uint m_uTotalLoad; // 0x50
    };

    class CTErrorContext {
    public:
        virtual int AppendContextInfo(char *, int);
    };

    class CTErrors {
    public:
        CTErrors();
        ~CTErrors();
        long Clear();
        long Error(D3DXShader::D3DXTOKEN *, uint, const char *, ...);
        long Warning(D3DXShader::D3DXTOKEN *, uint, const char *, ...);
        long SyntaxError(uint, D3DXShader::D3DXTOKEN *);
        long PreformattedError(const char *);
        long GetErrorBuffer(ID3DXBuffer **);
        uint GetErrorCount();
        uint GetWarningCount();
        long SetWarningLevel(uint);
        long SetWarningSpecifier(uint, uint);

        static void *operator new(size_t);
        static void *operator new[](size_t);
        static void operator delete(void *);
        static void operator delete[](void *);

    protected:
        long Push(uint, uint *);

    public:
        uint m_uPragmaCount; // 0x00
        uint *m_pErrorNums; // 0x04
        uint *m_pSpecifiers; // 0x08
        D3DXShader::CTErrorContext *m_pErrorContext; // 0x0c
    protected:
        void *m_pErrors; // 0x10
        uint m_cbErrors; // 0x14
        uint m_uErrorCount; // 0x18
        uint m_uWarningCount; // 0x1c
        uint m_uWarningLevel; // 0x20
    };
}
