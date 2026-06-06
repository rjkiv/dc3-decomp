#pragma once
#include "rndobj/ShaderMgr.h"

class DxShaderMgr : public RndShaderMgr {
public:
    DxShaderMgr() {}
    virtual void PreInit();
    virtual void Terminate();
    virtual void SetVConstant(VShaderConstant, RndTex *);
    virtual void SetVConstant(VShaderConstant, const Vector4 &); // 0x24
    virtual void SetVConstant(VShaderConstant, const float *__restrict, unsigned int);
    virtual void SetVConstant(VShaderConstant, int);
    virtual void SetVConstant(VShaderConstant, bool);
    virtual void SetVConstant(VShaderConstant, const Hmx::Matrix4 &); // 0x18
    virtual void SetVConstant4x3(VShaderConstant, const Hmx::Matrix4 &);
    virtual void SetPConstant(PShaderConstant, const Hmx::Matrix4 &);
    virtual void SetPConstant(PShaderConstant, RndCubeTex *);
    virtual void SetPConstant(PShaderConstant, const Vector4 &); // 0x40
    virtual void SetPConstant(PShaderConstant, RndTex *); // 0x3c
    virtual void SetPConstant(PShaderConstant, int);
    virtual void SetPConstant(PShaderConstant, bool);
    virtual void SetPConstant4x3(PShaderConstant, const Hmx::Matrix4 &);

protected:
    virtual void LoadShaderFile(FileStream &);

private:
    virtual RndShaderProgram *NewShaderProgram();
};

extern DxShaderMgr TheDxShaderMgr;
