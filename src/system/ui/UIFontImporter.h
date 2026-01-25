#pragma once
#include "obj/Object.h"
#include "rndobj/Font.h"
#include "rndobj/Text.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

/** "Class supporting font importing.  To be included in font resource file classes." */
class UIFontImporter : public virtual Hmx::Object {
public:
    enum FontQuality {
        kFontQuality_AntiAliased,
        kFontQuality_ClearType,
        kFontQuality_Default
    };

    enum FontSuperSample {
        kFontSuperSample_None,
        kFontSuperSample_2x,
        kFontSuperSample_4x
    };
    // Hmx::Object
    virtual ~UIFontImporter() {}
    OBJ_CLASSNAME(UIFontImporter);
    OBJ_SET_TYPE(UIFontImporter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    NEW_OBJ(UIFontImporter)
    OBJ_MEM_OVERLOAD(0x2A);

    void ImportSettingsFromFont(RndFontBase *);
    Symbol GetMatVariationName(unsigned int) const;
    char const *GetMatVariationName(RndFontBase *) const;
    int GetMatVariationIdx(Symbol) const;
    RndFontBase *GetGennedFont(Symbol) const;
    void AttachImporterToFont(RndFontBase *);

    int NumMatVariations() const { return mMatVariations.size(); }
    int NumGennedFonts() const { return mGennedFonts.size(); }
    bool HandMadeFontExists() const { return mHandmadeFont; }

protected:
    UIFontImporter();

    void GenerateBitmapFilename();
    String GetASCIIPlusChars();
    String GetASCIIMinusChars();
    void SyncWithGennedFonts();
    void HandmadeFontChanged();
    RndFontBase *FindFontForMat(RndMat *) const;
    DataNode OnShowFontPicker(DataArray *);
    DataNode OnGenerate(DataArray *);
    DataNode OnGenerateOG(DataArray *);
    DataNode OnGenerate3D(DataArray *);
    DataNode OnGetGennedBitmapPath(DataArray *);
    DataNode OnImportSettings(DataArray *);
    DataNode OnForgetGened(DataArray *);
    DataNode OnAttachToImportFont(DataArray *);
    void OnSetCharsetUTF8(String const &);
    DataNode OnSyncWithResourceFile(DataArray *);
    RndText *FindTextForFont(RndFontBase *) const;

    /** "include uppercase letters" */
    bool mUpperCaseAthroughZ; // 0x4
    /** "include lowercase letters" */
    bool mLowerCaseAthroughZ; // 0x5
    /** "include the number 0-9" */
    bool mNumbers0through9; // 0x6
    /** "include punctuation characters" */
    bool mPunctuation; // 0x7
    /** "include uppercase euro chars" */
    bool mUpperEuro; // 0x8
    /** "include lowercase euro chars" */
    bool mLowerEuro; // 0x9
    bool mRussian; // 0xa
    bool mPolish; // 0xb
    bool mIncludeLocale; // 0xc
    String mIncludeFile; // 0x10
    std::vector<unsigned short> mPlus; // 0x18
    std::vector<unsigned short> mMinus; // 0x24
    String mFontName; // 0x30
    float mFontPctSize; // 0x38
    int mFontWeight; // 0x3c
    bool mItalics; // 0x40
    int mDropShadow; // 0x44
    int mDropShadowOpacity; // 0x48
    int mFontQuality; // 0x4c
    int mPitchAndFamily; // 0x50
    int mFontCharset; // 0x54
    int mFontSupersample; // 0x58
    /** "pixels of padding on the left side of each character" */
    int mLeft; // 0x5c
    /** "pixels of padding on the left side of each character" */
    int mRight; // 0x60
    /** "pixels of padding on the left side of each character" */
    int mTop; // 0x64
    /** "pixels of padding on the left side of each character" */
    int mBottom; // 0x68
    bool mFillWithSafeWhite; // 0x6c
    ObjPtr<RndFont> mFontToImportFrom; // 0x70
    /** "path to save bitmap to (i.e. ui/image/)" */
    String mBitmapSavePath; // 0x84
    /** "name of the bitmap file (i.e. Arial(12).BMP)" */
    String mBitMapSaveName; // 0x8c
    ObjPtrList<RndFontBase> mGennedFonts; // 0x94
    ObjPtr<RndFontBase> mReferenceKerning; // 0xa8
    ObjPtrList<RndMat> mMatVariations; // 0xbc
    ObjPtr<RndFontBase> mHandmadeFont; // 0xd0
    bool mCheckNG; // 0xe4
    /** "You can pull in all the importer settings from another resource file by selecting
     * it above and hitting the sync button below" */
    String mSyncResource; // 0xe8
    /** "was the texture for this font last genned for an NG platform?" */
    bool mLastGenWasNG; // 0xf0
};
