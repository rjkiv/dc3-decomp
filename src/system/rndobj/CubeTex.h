#pragma once
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "rndobj/Bitmap.h"
#include "utl/MemMgr.h"

/** "This class implements a cube map intended for environment mapping." */
class RndCubeTex : public Hmx::Object {
public:
    enum CubeFace {
        kCubeFaceRight = 0x0000,
        kCubeFaceLeft = 0x0001,
        kCubeFaceTop = 0x0002,
        kCubeFaceBottom = 0x0003,
        kCubeFaceFront = 0x0004,
        kCubeFaceBack = 0x0005,
        kNumCubeFaces = 0x0006,
    };

    struct CubeTexProperties {
        CubeTexProperties() { Init(); }
        CubeTexProperties &operator=(const CubeTexProperties &t) {
            mWidth = t.mWidth;
            mHeight = t.mHeight;
            mBpp = t.mBpp;
            mNumMips = t.mNumMips;
            mOrder = t.mOrder;
            return *this;
        }
        void Init() { mOrder = mNumMips = mBpp = mHeight = mWidth = 0; }
        void Set(const RndBitmap &bmp);
        void Load(BinStream &d);
        void Save(BinStream &d);

        int mWidth, mHeight, mBpp, mNumMips, mOrder;
    };

    OBJ_CLASSNAME(CubeTex);
    OBJ_SET_TYPE(CubeTex);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    virtual void Select(int sampler) {}
    virtual void Reset() {}
    virtual void Sync() {}

    OBJ_MEM_OVERLOAD(0x17);
    NEW_OBJ(RndCubeTex)
    static void Init() { REGISTER_OBJ_FACTORY(RndCubeTex) }

    void SetBitmap(CubeFace f, const FilePath &file, bool update);
    void UpdateFace(CubeFace f);
    RndBitmap &GetBitmap(CubeFace cf) { return mBitmap[cf]; }

private:
    bool LoadBitmap(const FilePath &file, RndBitmap &bmp);
    bool ValidateBitmapProperties(std::vector<CubeFace> &invalidFaces);
    void Update();

protected:
    RndCubeTex();

    CubeTexProperties mBaseProps; // 0x2c
    CubeTexProperties mFaceProps[kNumCubeFaces]; // 0x40
    FilePath mFile[kNumCubeFaces]; // 0xb8
    RndBitmap mBitmap[kNumCubeFaces]; // 0xe8
};
