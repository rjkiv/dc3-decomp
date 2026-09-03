#pragma once

#include "d3dx9/CBaseProgram.h"
#include "d3dx9/CTransform.h"
#include "d3dx9/d3dx9mesh.h"

namespace D3DXShader {
    typedef struct _D3DXPROGRAM_CAPS {
        uint InputRegs; // 0x00
        uint ResultRegs; // 0x04
        uint AddressRegs; // 0x08
        uint PredicateRegs; // 0x0c
        uint ConstantRegs; // 0x10
        uint TextureRegs; // 0x14
        uint LoopRegs; // 0x18
        uint TotalInputs; // 0x1c
        uint OutColRegs; // 0x20
        uint Samplers; // 0x24
        uint TexRemap; // 0x28
        uint StaticFlowControlDepth; // 0x2c
        uint DynamicFlowControlDepth; // 0x30
        uint LoopCallFlowControlDepth; // 0x34
        uint BoolRegs; // 0x38
        uint CostLOG; // 0x3c
        uint MaxLoop; // 0x40
        int bPackScalars : 1; // 44; BitPos=0
        int bNoModLim : 1; // 44; BitPos=1
        int bIngoreWriteInfo : 1; // 44; BitPos=2
        int bPairTexLoads : 1; // 44; BitPos=3
        int bMinimizePhases : 1; // 44; BitPos=4
        int bLegacyLRP : 1; // 44; BitPos=5
        int bAddressFloor : 1; // 44; BitPos=6
        int bLegacyWriteMasks : 1; // 44; BitPos=7
        int bLegacyClip : 1; // 44; BitPos=8
        int bAlwaysPatternMatch : 1; // 44; BitPos=9
        int bUnitLiterals : 1; // 44; BitPos=10
        int bNoX2 : 1; // 44; BitPos=11
        int bBiasPositive : 1; // 44; BitPos=12
        int bCantNegateSat : 1; // 44; BitPos=13
        int DestSAT : 1; // 44; BitPos=14
        int DestD8 : 1; // 44; BitPos=15
        int DestD4 : 1; // 44; BitPos=16
        int DestD2 : 1; // 44; BitPos=17
        int DestX8 : 1; // 44; BitPos=18
        int DestX4 : 1; // 44; BitPos=19
        int DestX2 : 1; // 44; BitPos=20
        int bEmitTrig : 1; // 44; BitPos=21
        int bEmitSINCOS : 1; // 44; BitPos=22
        int bEmitCMP : 1; // 44; BitPos=23
        int bEmitLIT : 1; // 44; BitPos=24
        int bEmitDP2 : 1; // 44; BitPos=25
        int bNoConstModifiers : 1; // 44; BitPos=26
        int DoColMoveLast : 1; // 44; BitPos=27
        int NoSwizzles : 1; // 44; BitPos=28
        int VectorizeConsts : 1; // 44; BitPos=29
        int OutMoveOnce : 1; // 44; BitPos=30
        int bCompactInputRegs : 1; // 44; BitPos=31
        int bLegacyFRC : 1; // 48; BitPos=17
        int bAbsModifier : 1; // 48; BitPos=18
        int bAddressOutputs : 1; // 48; BitPos=19
        int bAddressInputs : 1; // 48; BitPos=20
        int bAddressConsts : 1; // 48; BitPos=21
        int bMinMax : 1; // 48; BitPos=22
        int bOutSemantics : 1; // 48; BitPos=23
        int bVertShader : 1; // 48; BitPos=24
        int bTexLOD : 1; // 48; BitPos=25
        int bDXDY : 1; // 48; BitPos=26
        int bCoIssue : 1; // 48; BitPos=27
        int bBreak : 1; // 48; BitPos=28
        int bDotPerfersXYZ : 1; // 48; BitPos=29
        int bEvenOddRegisterBanks : 1; // 48; BitPos=30
        int bScalarPipe : 1; // 48; BitPos=31
    } D3DXPROGRAM_CAPS;

    typedef struct _D3DXCP_READ {
        uint iInst; // 0x0
        uint uPredicate; // 0x4
        int bPredicate; // 0x8
    } D3DXCP_READ;

    typedef struct _D3DXCP_DATA {
        uint uPool; // 0x00
        uint uSize; // 0x04
        uint *pSize; // 0x08
        uint *pCopy; // 0x0c
        uint *pRemap; // 0x10
        uint *pSwizzle; // 0x14
        uint *pArguments; // 0x18
        uint *pRegisters; // 0x1c
        uint *pLRU; // 0x20
        uint uLRU; // 0x24
        uint uRegisters; // 0x28
        uint uRegisterLim; // 0x2c
        uint *pLinkCount; // 0x30
        uint *pLinkOffset; // 0x34
        uint *pLink; // 0x38
        uint *pExcludeCount; // 0x3c
        uint *pExcludeOffset; // 0x40
        uint *pExclude; // 0x44
        uint uArguments; // 0x48
        uint *pReadCount; // 0x4c
        uint *pReadOffset; // 0x50
        uint *pReadIndex; // 0x54
        D3DXShader::_D3DXCP_READ *pRead; // 0x58
        CInstruction *pInst; // 0x5c
        int bReassign : 1; // 0x60; BitPos=30
        int bPackScalars : 1; // 0x60; BitPos=31
    } D3DXCP_DATA;

    class CProgram : public CBaseProgram {
    public:
        CProgram(const CProgram &);
        CProgram(int);
        virtual ~CProgram();
        HRESULT SetName(const char *);
        HRESULT Initialize(CProgram *, uint, uint);
        HRESULT Initialize(CNode *, CTErrors *, uint, uint);
        HRESULT Print(CInstruction **, uint);
        HRESULT ChangeAttrib(unsigned short);
        virtual HRESULT IndexSemantic(CArgument *, int);
        virtual HRESULT GenerateCode(ID3DXBuffer **);
        int InputsAreVRegisters(CInstruction *);

    protected:
        virtual HRESULT InitCaps();
        HRESULT Optimize();
        HRESULT Split();
        HRESULT Vectorize();
        HRESULT Link();
        HRESULT DelayOutputs();
        HRESULT SimplifyInstructions();
        HRESULT RemoveDeadCode();
        HRESULT RemoveDuplicateArguments();
        HRESULT SquishInstructions();
        HRESULT RemoveDuplicateInstructions(int);
        HRESULT MergeInstructions();
        HRESULT ReorderBinary();
        HRESULT SimplifyAddresses();
        HRESULT SimplifyPredicates();
        HRESULT PropagatePredicates();
        HRESULT CompactOutputs();
        HRESULT CompactArguments();
        HRESULT CompactInstructions();
        HRESULT VectorizeLeft();
        HRESULT VectorizeRight();
        HRESULT PropagateSwizzles();
        HRESULT CombineInstructions();
        virtual HRESULT Translate();
        HRESULT SplitRegisters(int);
        HRESULT VectorizeLiterals();
        HRESULT ReorderInstructions();
        HRESULT SeparatePhases();
        HRESULT PropagateMovs();
        virtual HRESULT Reschedule();
        virtual HRESULT Constrain();
        HRESULT CompactRegisters();
        HRESULT SwizzleRegisters();
        HRESULT Validate();
        HRESULT ReadWriteInfo();
        HRESULT ParentChildInfo();
        HRESULT IdentityRemap();
        HRESULT RemapArguments();
        uint Remap(uint);
        uint Origin(uint);
        HRESULT ApplyTransform(HRESULT, const char *);
        HRESULT Error(CNode *, uint, const char *, ...);
        HRESULT Warning(CNode *, uint, const char *, ...);
        HRESULT CompactPool_Begin(D3DXCP_DATA *, uint, uint);
        HRESULT CompactPool_Process(D3DXCP_DATA *, int);
        HRESULT CompactPool_End(D3DXCP_DATA *);
        virtual void GetArgumentName(CArgument *, char *, uint);

    private:
        HRESULT ReorderInstructions(uint, uint *, uint *);
        int IsVarying(uint);
        HRESULT MarkVarying();
        HRESULT StripVarying();
        HRESULT StripUniform();
        HRESULT DeadLinkRemove();
        HRESULT MergePredicates(uint *, int *, uint, int);
        HRESULT MergePredicates(CArgument *, CArgument *);
        HRESULT MergeArguments(uint, uint);
        HRESULT SimplifyUnary(CInstruction *, uint, uint);
        HRESULT SimplifyBinary(CInstruction *, uint, uint, uint);
        HRESULT SimplifyTernary(CInstruction *, uint, uint, uint, uint);
        HRESULT SimplifyDotProduct(CInstruction *, int);
        int MulSequence(uint, uint);
        HRESULT AddSequence(uint, uint *, double *);
        HRESULT RangeSequence(uint, uint *, uint *, uint *, uint *, uint *);
        uint MergeInstructions_Input(CInstruction *, uint, uint, int);
        int VectorizeLeft_IsSameRegister(uint, uint);
        int VectorizeLeft_IsUsedTogether(uint, uint);
        int VectorizeLeft_IsConflicting(CInstruction *, uint, uint);
        HRESULT VectorizeLeft_PerComponent(CInstruction *);
        HRESULT VectorizeLeft_DotProduct(CInstruction *);
        HRESULT VectorizeLeft_Special(CInstruction *);
        HRESULT VectorizeLeft_UpdateLinks(CInstruction *);
        HRESULT PropagateSwizzles_Unswizzle(uint, uint *, uint, uint *);
        HRESULT CompactLiterals(uint *, uint *, uint *);
        HRESULT CompactLiterals_Remap(uint *, uint *, uint);
        uint CompactLiterals_Allocate(uint *, uint, uint, uint, int);
        uint CompactLiterals_Negate(uint, uint, uint);
        double CompactLiterals_GetValue(uint);
        uint AllocateLiteral(uint *, uint, uint, uint);
        int CanVectorize(uint *, uint, uint *, uint *, CInstruction *, uint, uint);
        int CanVectorize_CanRead(uint, CInstruction *, uint, uint);
        int CanVectorize_IsSameRegister(uint, uint);
        uint CanVectorize_TraceMulAdd(uint, double *, double *);
        HRESULT CombinePools(uint, uint, uint);
        int MutuallyExclusive(uint, int, uint, int);
        HRESULT SwizzleParameter(uint *, uint *, uint, int);
        void RemoveDeadCode_Reference(uint, uint);
        int SimplifyPredicates_IsEqual(uint, uint);
        void PropagatePredicates_Depth(uint, uint *);
        void ReorderInstructions_MarkParents(CInstruction *, uint *, uint);
        void ReorderInstructions_ComputeDependency(CInstruction *, uint *, int *, uint);
        uint SeparatePhases_Phases();
        int SeparatePhases_MoveTex(uint);
        uint CompactPool_Map(D3DXCP_DATA *, uint, uint, uint);
        uint CompactPool_Score(D3DXCP_DATA *, uint, uint, int, uint *, uint);
        HRESULT CompactPool_Read(D3DXCP_DATA *, uint, int, uint, uint);
        HRESULT CompactPool_Write(D3DXCP_DATA *, uint);
        void CompactPool_ReadCount(D3DXCP_DATA *, uint, uint);
        void CompactPool_WriteCount(D3DXCP_DATA *, uint, uint);
        void ReadWriteInfo_ReadInfo(uint, uint);
        void ReadWriteInfo_WriteInfo(uint, uint);
        HRESULT ReadWriteInfo_CommitOrigin();
        void ParentChildInfo_ParentInfo(uint, uint, int);
        HRESULT Print_Register(uint *, uint, int, unsigned short);

    public:
        D3DXPROGRAM_CAPS m_Caps; // 0x28
        uint m_uPoolVoid; // 0x74
        uint m_uPoolLiteral; // 0x78
        uint m_uPoolConstant; // 0x7c
        uint m_uPoolInput; // 0x80
        uint m_uPoolOutput; // 0x84
        uint m_uPoolResult; // 0x88
        uint m_uPoolAddress; // 0x8c
        uint m_uPoolPredicate; // 0x90
        uint m_uPoolObject; // 0x94
        uint m_uPoolIncomplete; // 0x98
        uint m_uPoolBool; // 0x9c
        uint m_uPoolBlock; // 0xa0
        uint m_uPoolBlockLoop; // 0xa4
        uint m_uPoolClip; // 0xa8
        uint m_uPoolCycle; // 0xac
        uint m_uPoolLoop; // 0xb0
        uint m_uPoolOutputBool; // 0xb4
        uint m_uPoolOutputInt; // 0xb8
        D3DXCP_DATA *m_pCPData; // 0xbc
    protected:
        CNode *m_pProgram; // 0xc0
        CTErrors *m_pErrors; // 0xc4
        uint m_Version; // 0xc8
        uint m_Flags; // 0xcc
        char *m_pName; // 0xd0
        int m_bFail; // 0xd4
        int m_bOptimized; // 0xd8
        int m_bVectorized; // 0xdc
        int m_bSwapBytes; // 0xe0
        CProgram *m_pPreShader; // 0xe4
    };
}
