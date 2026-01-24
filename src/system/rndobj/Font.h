#pragma once
#include "math/Geo.h"
#include "obj/Object.h"
#include "rndobj/Bitmap.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
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
        int unk0; // 0x0 - page?
        float unk4;
        float unk8;
        float charWidth; // 0xc
        float unk10;
        float unk14;
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
            return (RndMat *)mMats[0];
        else
            return nullptr;
    }
    virtual const RndFontBase *DataOwner() const { return mTextureOwner; }
    virtual float FontUnit() const { return mCellSize.x; }
    virtual void Print() const;

    OBJ_MEM_OVERLOAD(0x7C)
    NEW_OBJ(RndFont)
    static void Init() { REGISTER_OBJ_FACTORY(RndFont) }

    RndMat *Mat(int) const;
    RndTex *ValidTexture(int) const;
    void SetCellSize(float, float);
    int CharPage(unsigned short) const;
    void BleedTest();
    bool
    CharWidthAdvanceCoords(unsigned short, float &, float &, Vector2 &, Vector2 &) const;
    int NumMats() const { return mMats.size(); }
    float DeprecatedSize() const { return mDeprecatedSize; }

protected:
    RndFont();
    virtual bool HasChar(unsigned short) const;
    virtual void SetASCIIChars(String);

    void UpdateChars();

    ObjPtrVec<RndMat> mMats; // 0x44
    ObjOwnerPtr<RndFont> mTextureOwner; // 0x60
    std::map<unsigned short, CharInfo> mCharInfoMap; // 0x74
    Vector2 mCellSize; // 0x8c
    float mDeprecatedSize; // 0x94
    std::vector<Vector2> unk98; // 0x98
    bool mPacked; // 0xa4
};

class RndFont3d : public RndFontBase {
public:
    struct CharInfo {
        ~CharInfo() {}

        Box unk0;
        int unk20;
        // ObjPtr<RndMesh> unk24;
        bool unk38;

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
    virtual RndMat *Mat() const;
    virtual const RndFontBase *DataOwner() const;
    virtual float FontUnit() const { return mTextureOwner->unk6c.x; }
    virtual float FontUnitInverse() const { return mTextureOwner->unk7c.x; }

    OBJ_MEM_OVERLOAD(0x10A)
    NEW_OBJ(RndFont3d)
    static void Init() { REGISTER_OBJ_FACTORY(RndFont3d) }

protected:
    RndFont3d();

    virtual bool HasChar(unsigned short) const;

    void Clear();

    ObjPtr<RndMat> unk44; // 0x44
    ObjOwnerPtr<RndFont3d> mTextureOwner; // 0x58
    Vector3 unk6c; // 0x6c
    Vector3 unk7c; // 0x7c
    Vector3 unk8c; // 0x8c
    std::map<unsigned short, CharInfo *> mCharInfoMap; // 0x9c
};

class BitmapLocker {
public:
    BitmapLocker(RndFont *, int);
    ~BitmapLocker();
    void LoadPage(int);

private:
    RndFont *mFont; // 0x0
    RndTex *mTex; // 0x4
    RndBitmap *unk8; // 0x8
    RndBitmap unkc; // 0xc
};
