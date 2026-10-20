#pragma once
#include "math/Geo.h"
#include "obj/Object.h"
#include "rndobj/Bitmap.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class KerningTable {
public:
    class Entry {
    public:
        Entry *next; // 0x0
        int key; // 0x4
        float kerning; // 0x8
    };
    KerningTable();
    ~KerningTable();
    float Kerning(unsigned short, unsigned short);
    void GetKerning(std::vector<RndFontBase::KernInfo> &) const;
    void SetKerning(const std::vector<RndFontBase::KernInfo> &, RndFontBase *);
    Entry *Find(unsigned short, unsigned short);
    void Save(BinStream &);
    void Load(BinStreamRev &, RndFontBase *);
    bool Valid(const RndFontBase::KernInfo &, RndFontBase *);

    int Key(unsigned short us0, unsigned short us2) {
        return (us0 & 0xFFFF) | ((us2 << 0x10) & 0xFFFF0000);
    }
    int Size() const { return mNumEntries * sizeof(Entry) + 0x88; }
    int TableIndex(unsigned short us0, unsigned short us2) { return (us0 ^ us2) & 0x1F; }

    MEM_OVERLOAD(KerningTable, 0x162);

    int mNumEntries; // 0x0
    Entry *mEntries; // 0x4
    Entry *mTable[32]; // 0x8
};

class RndFont : public RndFontBase {
public:
    struct CharInfo {
        int page; // 0x0
        float normX; // 0x4
        float normY; // 0x8
        float charWidth; // 0xc
        // how much horizontal space this character takes up
        float charAdvance; // 0x10
    };

    virtual ~RndFont();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Font);
    OBJ_SET_TYPE(Font);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual float CharWidth(unsigned short) const;
    virtual bool CharAdvance(unsigned short, unsigned short, float &) const;
    virtual float CharAdvance(unsigned short) const;
    virtual bool CharDefined(unsigned short) const;
    virtual float AspectRatio() const { return mCellSize.y / mCellSize.x; }
    virtual RndMat *Mat() const {
        if (mMats.size() > 0)
            return mMats[0];
        else
            return nullptr;
    }
    virtual const RndFontBase *DataOwner() const { return mTextureOwner; }
    virtual float FontUnit() const { return mCellSize.x; }
    virtual void Print() const;

    OBJ_MEM_OVERLOAD(0x7C)
    NEW_OBJ(RndFont)
    REGISTER_OBJ(RndFont)

    RndMat *Mat(int page) const;
    RndTex *ValidTexture(int page) const;
    void SetCellSize(float, float);
    int CharPage(unsigned short a) const;
    void BleedTest();
    bool CharWidthAdvanceCoords(
        unsigned short c,
        float &width,
        float &advance,
        Vector2 &topLeft,
        Vector2 &bottomRight
    ) const;
    int NumMats() const { return mMats.size(); }
    float DeprecatedSize() const { return mDeprecatedSize; }

protected:
    RndFont();
    virtual bool HasChar(unsigned short w) const;
    virtual void SetASCIIChars(String s);

    void UpdateChars();
    void SetBitmapSize(const Vector2 &cellSize);
    void SetCharInfo(CharInfo *info, RndBitmap &a, const Vector2 &pos, int page);

    ObjPtrVec<RndMat> mMats; // 0x44
    ObjOwnerPtr<RndFont> mTextureOwner; // 0x60
    std::map<unsigned short, CharInfo> mCharInfos; // 0x74
    Vector2 mCellSize; // 0x8c
    float mDeprecatedSize; // 0x94
    std::vector<Vector2> mNormCellSize; // 0x98
    bool mPacked; // 0xa4
};

class RndFont3d : public RndFontBase {
public:
    struct CharInfo {
        CharInfo(Hmx::Object *owner) : mesh(owner) {}

        Box bbox; // 0x0
        float charAdvance; // 0x20
        ObjPtr<RndMesh> mesh; // 0x24
        bool handmade; // 0x38

        MEM_OVERLOAD(CharInfo, 0x12A);
    };
    virtual ~RndFont3d() { Clear(); }
    OBJ_CLASSNAME(Font3d);
    OBJ_SET_TYPE(Font3d);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual float CharWidth(unsigned short) const;
    virtual bool CharAdvance(unsigned short, unsigned short, float &) const;
    virtual float CharAdvance(unsigned short) const;
    virtual float Kerning(unsigned short, unsigned short) const;
    virtual float AspectRatio() const;
    virtual RndMat *Mat() const { return mMat; }
    virtual const RndFontBase *DataOwner() const { return mCharGeomOwner; }
    virtual float FontUnit() const { return mCharGeomOwner->mCellSize.x; }
    virtual float FontUnitInverse() const { return mCharGeomOwner->mNormalize.x; }

    CharInfo *GetCharInfo(unsigned short c) const;
    bool CharWidthAdvanceMesh(
        unsigned short c, float &width, float &advance, RndMesh **mesh
    ) const;
    Vector3 CharOriginOffset() const;

    OBJ_MEM_OVERLOAD(0x10A)
    NEW_OBJ(RndFont3d)
    REGISTER_OBJ(RndFont3d)

protected:
    RndFont3d();

    virtual bool HasChar(unsigned short w) const;

    void Clear();

    ObjPtr<RndMat> mMat; // 0x44
    ObjOwnerPtr<RndFont3d> mCharGeomOwner; // 0x58
    Vector3 mCellSize; // 0x6c
    Vector3 mNormalize; // 0x7c
    Vector3 mCharOriginOffset; // 0x8c
    std::map<unsigned short, CharInfo *> mCharInfos; // 0x9c
};

class BitmapLocker {
public:
    BitmapLocker(RndFont *f, int page);
    ~BitmapLocker();
    void LoadPage(int page);

    RndBitmap *Bitmap() const { return mPbm; }

protected:
    RndFont *mFont; // 0x0
    RndTex *mTexture; // 0x4
    RndBitmap *mPbm; // 0x8
    RndBitmap mBm; // 0xc
};
