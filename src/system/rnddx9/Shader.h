#pragma once
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderProgram.h"
#include "xdk/D3D9.h"

// An DX9 node in a RndShaderMgr::ShaderTree.
// Contains vertex and pixel shaders.
class DxShader : public RndShaderProgram {
public:
    DxShader()
        : mVShader(0), mPShader(0), mMinOverall(-1), mMaxOverall(-1), mPreCreated(0) {}
    virtual ~DxShader();
    virtual void Select(bool vertexOnly);
    virtual void Copy(const RndShaderProgram &src);
    virtual void EstimatedCost(float &min, float &max);
    virtual void SetShaders(D3DVertexShader *, D3DPixelShader *);

    static void *operator new(unsigned int) { return TheShaderMgr.AllocShader(); }

protected:
    virtual RndShaderBuffer *NewBuffer(unsigned int numBufferBytes);
    virtual bool
    Compile(ShaderType, const ShaderOptions &, RndShaderBuffer *&, RndShaderBuffer *&);
    virtual void CreateVertexShader(RndShaderBuffer &vertexBuffer);
    virtual void CreatePixelShader(RndShaderBuffer &pixelBuffer, ShaderType);

    D3DVertexShader *mVShader; // 0x20
    D3DPixelShader *mPShader; // 0x24
    float mMinOverall; // 0x28
    float mMaxOverall; // 0x2c
    bool mPreCreated; // 0x30
};
