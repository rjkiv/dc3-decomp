#pragma once
#include "obj/Object.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
#include "rndobj/Text.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

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

/** "Class supporting font importing.  To be included in font resource file classes." */
class UIFontImporter : public virtual Hmx::Object {
public:
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

    void ImportSettingsFromFont(RndFontBase *f);
    Symbol GetMatVariationName(unsigned int idx) const;
    const char *GetMatVariationName(RndFontBase *font) const;
    int GetMatVariationIdx(Symbol name) const;
    RndFontBase *GetGennedFont(Symbol matName) const;
    RndFontBase *GetGennedFont(unsigned int idx) const;
    // public: RndText* GetGennedText(Symbol) const;
    RndMat *GetMatVariation(unsigned int idx) const;
    void AttachImporterToFont(RndFontBase *f);

    int NumMatVariations() const { return mMatVariations.size(); }
    bool IsHandmade() const { return mHandmadeFont; }

protected:
    UIFontImporter();

    void GenerateBitmapFilename();
    String GetASCIIPlusChars();
    String GetASCIIMinusChars();
    void SyncWithGennedFonts();
    void HandmadeFontChanged();
    RndFontBase *FindFontForMat(RndMat *mat) const;
    DataNode OnShowFontPicker(DataArray *);
    DataNode OnGenerate(DataArray *);
    DataNode OnGenerateOG(DataArray *);
    DataNode OnGenerate3d(DataArray *);
    DataNode OnGetGennedBitmapPath(DataArray *);
    DataNode OnImportSettings(DataArray *);
    DataNode OnForgetGened(DataArray *);
    DataNode OnAttachToImportFont(DataArray *);
    void OnSetCharsetUTF8(const String &utf8chars);
    DataNode OnSyncWithResourceFile(DataArray *);
    RndText *FindTextForFont(RndFontBase *font) const;

    /** "include uppercase letters" */
    bool mIncludeUpperCase; // 0x4
    /** "include lowercase letters" */
    bool mIncludeLowerCase; // 0x5
    /** "include the number 0-9" */
    bool mIncludeNumbers; // 0x6
    /** "include punctuation characters" */
    bool mIncludePunctuation; // 0x7
    /** "include uppercase euro chars" */
    bool mIncludeUpperEuro; // 0x8
    /** "include lowercase euro chars" */
    bool mIncludeLowerEuro; // 0x9
    bool mIncludeRussian; // 0xa
    bool mIncludePolish; // 0xb
    bool mIncludeLocaleData; // 0xc
    String mIncludeDataFile; // 0x10
    std::vector<unsigned short> mPlusChars; // 0x18
    std::vector<unsigned short> mMinusChars; // 0x24
    String mFontName; // 0x30
    float mFontPctSize; // 0x38
    int mFontWeight; // 0x3c
    bool mIsFontItalic; // 0x40
    int mDropShadow; // 0x44
    int mDropShadowOpacity; // 0x48
    FontQuality mFontQuality; // 0x4c
    int mPitchAndFamily; // 0x50
    int mFontCharset; // 0x54
    FontSuperSample mFontSupersample; // 0x58
    /** "pixels of padding on the left side of each character" */
    int mTexPadLeft; // 0x5c
    /** "pixels of padding on the left side of each character" */
    int mTexPadRight; // 0x60
    /** "pixels of padding on the left side of each character" */
    int mTexPadTop; // 0x64
    /** "pixels of padding on the left side of each character" */
    int mTexPadBottom; // 0x68
    bool mFillWithSafeWhite; // 0x6c
    ObjPtr<RndFont> mImportSettingsFont; // 0x70
    /** "path to save bitmap to (i.e. ui/image/)" */
    String mBitmapPath; // 0x84
    /** "name of the bitmap file (i.e. Arial(12).BMP)" */
    String mBitmapFileName; // 0x8c
    ObjPtrList<RndFontBase> mGennedFonts; // 0x94
    ObjPtr<RndFontBase> mKerningReference; // 0xa8
    ObjPtrList<RndMat> mMatVariations; // 0xbc
    ObjPtr<RndFontBase> mHandmadeFont; // 0xd0
    bool mCheckNG; // 0xe4
    /** "You can pull in all the importer settings from another resource file by selecting
     * it above and hitting the sync button below" */
    String mSyncResource; // 0xe8
    /** "was the texture for this font last genned for an NG platform?" */
    bool mLastGenWasNG; // 0xf0
};
