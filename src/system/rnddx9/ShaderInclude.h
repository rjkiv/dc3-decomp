#pragma once
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShaderProgram.h"
#include "utl/MemMgr.h"
#include "xdk/D3DX9.h"
#include "xdk/XAPILIB.h"
#include "xdk/d3dx9/d3dx9mesh.h"

// Basically a wrapper around an ID3DXBuffer.
// https://learn.microsoft.com/en-us/windows/win32/direct3d9/id3dxbuffer
class DxShaderBuffer : public RndShaderBuffer {
public:
    DxShaderBuffer() : mBuffer(0) {}
    DxShaderBuffer(unsigned int numBytes) {
        if (numBytes != 0) {
            HRESULT res = D3DXCreateBuffer(numBytes, &mBuffer);
            if (res != ERROR_SUCCESS) {
                MILO_FAIL("File: %s Line: %d Error: %X\n", __FILE__, 0x65, res);
            }
        } else {
            mBuffer = nullptr;
        }
    }
    virtual ~DxShaderBuffer() {
        if (mBuffer) {
            mBuffer->Release();
            mBuffer = nullptr;
        }
    }
    virtual void *Storage() {
        if (mBuffer) {
            return mBuffer->GetBufferPointer();
        } else
            return nullptr;
    }
    virtual unsigned int Size() const {
        if (mBuffer) {
            return mBuffer->GetBufferSize();
        } else
            return 0;
    }

private:
    ID3DXBuffer *mBuffer; // 0x4
};

// https://learn.microsoft.com/en-us/windows/win32/direct3d9/id3dxinclude
class DxShaderInclude : public ID3DXInclude {
public:
    virtual HRESULT Open(
        D3DXINCLUDE_TYPE IncludeType,
        LPCSTR pFileName,
        LPCVOID pParentData,
        LPCVOID *ppData,
        UINT *pBytes,
        LPSTR pFullPath,
        DWORD cbFullPath
    ) {
        String str(ShaderSourcePath(pFileName));
        if (pFullPath && cbFullPath != 0) {
            strncpy(pFullPath, str.c_str(), cbFullPath - 1);
            pFullPath[cbFullPath - 1] = '\0';
        }
        File *file = NewFile(str.c_str(), FILE_OPEN_NOARK | FILE_OPEN_READ);
        if (!file) {
            MILO_NOTIFY("Could not find shader file '%s'.", str);
            return ERROR_FILE_NOT_FOUND;
        } else {
            *pBytes = file->Size();
            *ppData = MemAlloc(*pBytes, __FILE__, 0x44, "shader compile buffer");
            file->Read((void *)*ppData, *pBytes);
            delete file;
            return ERROR_SUCCESS;
        }
    }

    virtual HRESULT Close(LPCVOID pData) {
        MemFree((void *)pData, __FILE__, 0x4D, "shader compile buffer");
        return ERROR_SUCCESS;
    }
    virtual HRESULT Open(
        D3DXINCLUDE_TYPE IncludeType,
        LPCSTR pFileName,
        LPCVOID pParentData,
        LPCVOID *ppData,
        UINT *pBytes
    ) {
        return Open(IncludeType, pFileName, pParentData, ppData, pBytes, nullptr, 0);
    }
};
