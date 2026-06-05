#pragma once
#include "xdk/D3D9.h"
#include "xdk/XAPILIB.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/win_types.h"
#include "d3dx9math.h"
#include "d3dx9mesh.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD D3DXHANDLE;

typedef enum _D3DXINCLUDE_TYPE {
    D3DXINC_LOCAL = 0x0000,
    D3DXINC_SYSTEM = 0x0001,
    D3DXINC_FORCE_DWORD = 0x7fffffff,
} D3DXINCLUDE_TYPE;

// https://learn.microsoft.com/en-us/windows/win32/direct3d9/id3dxinclude
struct ID3DXInclude {
    virtual HRESULT Open(
        D3DXINCLUDE_TYPE IncludeType,
        LPCSTR pFileName,
        LPCVOID pParentData,
        LPCVOID *ppData,
        UINT *pBytes,
        LPSTR pFullPath,
        DWORD cbFullPath
    );
    virtual HRESULT Close(LPCVOID pData);

    ID3DXInclude(const ID3DXInclude &);
    ID3DXInclude();
    ID3DXInclude &operator=(const ID3DXInclude &);
};

typedef struct _D3DXCONSTANTTABLE_DESC {
    LPCSTR Creator;
    DWORD Version;
    UINT Constants;
} D3DXCONSTANTTABLE_DESC, *LPD3DXCONSTANTTABLE_DESC;

typedef enum _D3DXPARAMETER_TYPE {
    D3DXPT_VOID = 0x0000,
    D3DXPT_BOOL = 0x0001,
    D3DXPT_INT = 0x0002,
    D3DXPT_FLOAT = 0x0003,
    D3DXPT_STRING = 0x0004,
    D3DXPT_TEXTURE = 0x0005,
    D3DXPT_TEXTURE1D = 0x0006,
    D3DXPT_TEXTURE2D = 0x0007,
    D3DXPT_TEXTURE3D = 0x0008,
    D3DXPT_TEXTURECUBE = 0x0009,
    D3DXPT_SAMPLER = 0x000a,
    D3DXPT_SAMPLER1D = 0x000b,
    D3DXPT_SAMPLER2D = 0x000c,
    D3DXPT_SAMPLER3D = 0x000d,
    D3DXPT_SAMPLERCUBE = 0x000e,
    D3DXPT_PIXELSHADER = 0x000f,
    D3DXPT_VERTEXSHADER = 0x0010,
    D3DXPT_PIXELFRAGMENT = 0x0011,
    D3DXPT_VERTEXFRAGMENT = 0x0012,
    D3DXPT_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_TYPE;

typedef enum _D3DXREGISTER_SET {
    D3DXRS_BOOL = 0x0000,
    D3DXRS_INT4 = 0x0001,
    D3DXRS_FLOAT4 = 0x0002,
    D3DXRS_SAMPLER = 0x0003,
    D3DXRS_FORCE_DWORD = 0x7fffffff,
} D3DXREGISTER_SET;

typedef enum _D3DXPARAMETER_CLASS {
    D3DXPC_SCALAR = 0x0000,
    D3DXPC_VECTOR = 0x0001,
    D3DXPC_MATRIX_ROWS = 0x0002,
    D3DXPC_MATRIX_COLUMNS = 0x0003,
    D3DXPC_OBJECT = 0x0004,
    D3DXPC_STRUCT = 0x0005,
    D3DXPC_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_CLASS;

typedef struct _D3DXCONSTANT_DESC {
    LPCSTR Name;
    D3DXREGISTER_SET RegisterSet;
    UINT RegisterIndex;
    UINT RegisterCount;
    D3DXPARAMETER_CLASS Class;
    D3DXPARAMETER_TYPE Type;
    UINT Rows;
    UINT Columns;
    UINT Elements;
    UINT StructMembers;
    UINT Bytes;
    LPCVOID DefaultValue;
} D3DXCONSTANT_DESC, *LPD3DXCONSTANT_DESC;

struct ID3DXConstantTable : public ID3DXBuffer { /* Size=0x4 */
    /* 0x0000: fields for ID3DXBuffer */

    virtual LPVOID GetBufferPointer() = 0;
    virtual DWORD GetBufferSize() = 0;
    virtual HRESULT GetDesc(D3DXCONSTANTTABLE_DESC *pDesc);
    virtual HRESULT
    GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC *pDesc, UINT *pCount);
    virtual UINT GetSamplerIndex(D3DXHANDLE hConstant);
    virtual D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT Index);
    virtual D3DXHANDLE GetConstantByName(D3DXHANDLE hConstant, LPCSTR pName);
    virtual D3DXHANDLE GetConstantElement(D3DXHANDLE hConstant, UINT Index);
    virtual HRESULT SetDefaults(D3DDevice *pDevice);
    virtual HRESULT
    SetValue(D3DDevice *pDevice, D3DXHANDLE hConstant, LPCVOID pData, UINT Bytes);
    virtual HRESULT SetBool(D3DDevice *pDevice, D3DXHANDLE hConstant, BOOL b);
    virtual HRESULT
    SetBoolArray(D3DDevice *pDevice, D3DXHANDLE hConstant, const BOOL *pB, UINT Count);
    virtual HRESULT SetInt(D3DDevice *pDevice, D3DXHANDLE hConstant, INT n);
    virtual HRESULT
    SetIntArray(D3DDevice *pDevice, D3DXHANDLE hConstant, const INT *pn, UINT Count);
    virtual HRESULT SetFloat(D3DDevice *pDevice, D3DXHANDLE hConstant, FLOAT f);
    virtual HRESULT
    SetFloatArray(D3DDevice *pDevice, D3DXHANDLE hConstant, const FLOAT *pf, UINT Count);
    virtual HRESULT
    SetVector(D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXVECTOR4 *pVector);
    virtual HRESULT SetVectorArray(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXVECTOR4 *pVector, UINT Count
    );
    virtual HRESULT
    SetMatrix(D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX *pMatrix);
    virtual HRESULT SetMatrixArray(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX *pMatrix, UINT Count
    );
    virtual HRESULT SetMatrixPointerArray(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX **ppMatrix, UINT Count
    );
    virtual HRESULT SetMatrixTranspose(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX *pMatrix
    );
    virtual HRESULT SetMatrixTransposeArray(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX *pMatrix, UINT Count
    );
    virtual HRESULT SetMatrixTransposePointerArray(
        D3DDevice *pDevice, D3DXHANDLE hConstant, const D3DXMATRIX **ppMatrix, UINT Count
    );

    ID3DXConstantTable(const ID3DXConstantTable &);
    ID3DXConstantTable();
    ID3DXConstantTable &operator=(const ID3DXConstantTable &);
};

typedef struct D3DXMACRO {
    LPCSTR Name;
    LPCSTR Definition;
} D3DXMACRO, *LPD3DXMACRO;

typedef struct _D3DXSHADER_COMPILE_PARAMETERS { /* Size=0x30 */
    /* 0x0000 */ DWORD Flags;
    /* 0x0004 */ DWORD UPDBTimestamp;
    /* 0x0008 */ LPCSTR UPDBPath;
    /* 0x000c */ ID3DXBuffer *pUPDBBuffer;
    /* 0x0010 */ DWORD TempRegisterLimit;
    /* 0x0014 */ LPVOID pUPDBB;
    /* 0x0018 */ LPCSTR CpuFunctionName;
    /* 0x001c */ BOOL bXbox360ExtensionUsed;
    /* 0x0020 */ DWORD PixelShaderSamplerRegisterBase;
    /* 0x0024 */ DWORD PixelShaderSamplerRegisterCount;
    /* 0x0028 */ DWORD VertexShaderSamplerRegisterBase;
    /* 0x002c */ DWORD VertexShaderSamplerRegisterCount;
} D3DXSHADER_COMPILE_PARAMETERS;

HRESULT D3DXCompileShaderExA(
    LPCSTR pSrcData,
    UINT srcDataLen,
    const D3DXMACRO *pDefines,
    ID3DXInclude *pInclude,
    LPCSTR pFunctionName,
    LPCSTR pProfile,
    DWORD Flags,
    ID3DXBuffer **ppShader,
    ID3DXBuffer **ppErrorMsgs,
    ID3DXConstantTable **ppConstantTable,
    D3DXSHADER_COMPILE_PARAMETERS *pParameters
);

#ifdef __cplusplus
}
#endif
