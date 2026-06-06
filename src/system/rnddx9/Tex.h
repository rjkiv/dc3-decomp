#pragma once
#include "obj/Object.h"
#include "rnddx9/Object.h"
#include "rndobj/Tex.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"

class DxTex : public RndTex, public DxObject {
public:
    virtual ~DxTex();
    OBJ_CLASSNAME(Tex)
    OBJ_SET_TYPE(Tex)
    virtual void LockBitmap(RndBitmap &, int);
    virtual void UnlockBitmap();
    virtual void MakeDrawTarget();
    virtual void FinishDrawTarget();
    virtual void Compress(AlphaCompress);
    virtual bool TexelsLock(void *&);
    virtual unsigned int TexelsPitch() const;
    virtual void Select(int);
    virtual void PreDeviceReset();
    virtual void PostDeviceReset();

    static void Init();
    NEW_OBJ(DxTex)
    static void SetEDRamChecksEnabled(bool enabled) { sEDRamChecksEnabled = enabled; }

    void *StartCompress(AlphaCompress);
    void DoCompress(void *);
    void FinishCompress(void *);
    void SetDeviceTex(D3DTexture *);
    D3DSurface *GetRT();
    D3DSurface *GetDepthRT();
    D3DSurface *GetMovieSurface();
    void SwapMovieSurface();
    D3DTexture *Tex() const { return mTexture; }

private:
    static bool sEDRamChecksEnabled;

    void ResetSurfaces();
    void ResolveMipChain();
    D3DSurface *GetSurfaceLevel(int);

protected:
    DxTex();

    virtual void PresyncBitmap() { ResetSurfaces(); }
    virtual void SyncBitmap();

    D3DFORMAT mFormat; // 0x7c
    D3DTexture *mTexture; // 0x80
    int unk84;
    D3DSurface *mRenderTarget; // 0x88
    D3DSurface *mDepthRT; // 0x8c
    int unk90;
    D3DTexture *unk94[2];
    D3DLOCKED_RECT unk9c; // 0x9c
    D3DSurface *unka4; // 0xa4
    int unka8;
    bool unkac;
};
