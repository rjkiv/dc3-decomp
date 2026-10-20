#include "ui/UIFontImporter.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Mat.h"
#include "rndobj/Tex.h"
#include "rndobj/Text.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UILabelDir.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include <vector>

#define HEIGHT_SD 480.0f
#define HEIGHT_HD 720.0f

float ConvertHeightOGToPctHeight(int i) { return fabsf(i / HEIGHT_SD); }
float ConvertHeightNGToPctHeight(int i) { return fabsf(i / HEIGHT_HD); }
int ConvertPctHeightToHeightNG(float f) { return -Round(f * HEIGHT_HD); }
int ConvertPctHeightToHeightOG(float f) { return -Round(f * HEIGHT_SD); }

UIFontImporter::UIFontImporter()
    : mIncludeUpperCase(1), mIncludeLowerCase(1), mIncludeNumbers(1),
      mIncludePunctuation(1), mIncludeUpperEuro(1), mIncludeLowerEuro(1),
      mIncludeRussian(0), mIncludePolish(0), mIncludeLocaleData(0), mIncludeDataFile(""),
      mFontName("Arial"), mFontPctSize(ConvertHeightNGToPctHeight(12)), mFontWeight(400),
      mIsFontItalic(false), mDropShadow(0), mDropShadowOpacity(128),
      mFontQuality(kFontQuality_AntiAliased), mPitchAndFamily(34), mFontCharset(0),
      mFontSupersample(kFontSuperSample_None), mTexPadLeft(0), mTexPadRight(0),
      mTexPadTop(0), mTexPadBottom(0), mFillWithSafeWhite(false),
      mImportSettingsFont(this), mBitmapPath("ui/image/"), mBitmapFileName("temp.bmp"),
      mGennedFonts(this), mKerningReference(this), mMatVariations(this),
      mHandmadeFont(this), mCheckNG(false), mLastGenWasNG(true) {
    static Symbol objects("objects");
    static Symbol default_bitmap_path("default_bitmap_path");
    DataArray *cfgArr =
        SystemConfig(objects, StaticClassName())->FindArray(default_bitmap_path, false);
    if (cfgArr) {
        mBitmapPath = cfgArr->Str(1);
    }
    GenerateBitmapFilename();
}

BEGIN_HANDLERS(UIFontImporter)
    HANDLE(show_font_picker, OnShowFontPicker)
    HANDLE(generate, OnGenerate)
    HANDLE(generate_og, OnGenerateOG)
    HANDLE(generate_og, OnGenerateOG)
    HANDLE(generate_3d, OnGenerate3d)
    HANDLE(forget_gened_fonts, OnForgetGened)
    HANDLE(attach_to_importfont, OnAttachToImportFont)
    HANDLE(import_from_importfont, OnImportSettings)
    HANDLE(sync_with_resource, OnSyncWithResourceFile)
    HANDLE_EXPR(
        get_resources_file_list, ResourceDirBase::GetFileList("UILabel", "UILabelDir")
    )
    HANDLE(get_bitmap_path, OnGetGennedBitmapPath)
    HANDLE_ACTION(set_charset_utf8, OnSetCharsetUTF8(_msg->Str(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(UIFontImporter)
    SYNC_PROP(UPPER_CASE_A_Z, mIncludeUpperCase)
    SYNC_PROP(lower_case_a_z, mIncludeLowerCase)
    SYNC_PROP(numbers_0_9, mIncludeNumbers)
    SYNC_PROP(punctuation, mIncludePunctuation)
    SYNC_PROP(UPPER_EURO, mIncludeUpperEuro)
    SYNC_PROP(lower_euro, mIncludeLowerEuro)
    SYNC_PROP(russian, mIncludeRussian)
    SYNC_PROP(polish, mIncludePolish)
    SYNC_PROP(include_locale, mIncludeLocaleData)
    SYNC_PROP(include_file, mIncludeDataFile)
    SYNC_PROP_SET(plus, GetASCIIPlusChars(), ASCIItoWideVector(mPlusChars, _val.Str()))
    SYNC_PROP_SET(minus, GetASCIIMinusChars(), ASCIItoWideVector(mMinusChars, _val.Str()))
    SYNC_PROP(font_name, mFontName)
    SYNC_PROP_MODIFY(font_pct_size, mFontPctSize, GenerateBitmapFilename())
    SYNC_PROP_SET(
        font_point_size,
        mLastGenWasNG ? ConvertPctHeightToHeightNG(mFontPctSize)
                      : ConvertPctHeightToHeightOG(mFontPctSize),
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(-_val.Int())
                                     : ConvertHeightOGToPctHeight(-_val.Int())
    )
    SYNC_PROP_SET(
        font_pixel_size,
        mLastGenWasNG ? std::abs(ConvertPctHeightToHeightNG(mFontPctSize))
                      : std::abs(ConvertPctHeightToHeightOG(mFontPctSize)),
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(_val.Int())
                                     : ConvertHeightOGToPctHeight(_val.Int())
    )
    SYNC_PROP_MODIFY(weight, mFontWeight, GenerateBitmapFilename())
    SYNC_PROP_SET(
        bold,
        mFontWeight > 500,
        if (_val.Int() != 0) { mFontWeight = 800; } else { mFontWeight = 400; }

        GenerateBitmapFilename()
    )
    SYNC_PROP_MODIFY(italics, mIsFontItalic, GenerateBitmapFilename())
    SYNC_PROP_MODIFY(drop_shadow, mDropShadow, GenerateBitmapFilename())
    SYNC_PROP(drop_shadow_opacity, mDropShadowOpacity)
    SYNC_PROP(font_quality, (int &)mFontQuality)
    SYNC_PROP(pitch_and_family, mPitchAndFamily)
    SYNC_PROP(font_charset, mFontCharset)
    SYNC_PROP_MODIFY(font_supersample, (int &)mFontSupersample, GenerateBitmapFilename())
    SYNC_PROP(left, mTexPadLeft)
    SYNC_PROP(right, mTexPadRight)
    SYNC_PROP(top, mTexPadTop)
    SYNC_PROP(bottom, mTexPadBottom)
    SYNC_PROP(fill_with_safe_white, mFillWithSafeWhite)
    SYNC_PROP(font_to_import_from, mImportSettingsFont)
    SYNC_PROP(bitmap_save_path, mBitmapPath)
    SYNC_PROP(bitmap_save_name, mBitmapFileName)
    SYNC_PROP(gened_fonts, mGennedFonts)
    SYNC_PROP(reference_kerning, mKerningReference)
    SYNC_PROP_MODIFY(mat_variations, mMatVariations, SyncWithGennedFonts())
    SYNC_PROP_MODIFY(handmade_font, mHandmadeFont, HandmadeFontChanged())
    SYNC_PROP(resource_name, mSyncResource)
    SYNC_PROP(last_genned_ng, mLastGenWasNG)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(UIFontImporter)
    SAVE_REVS(10, 4)
    bs << mIncludeLowerCase;
    bs << mIncludeUpperCase;
    bs << mIncludeNumbers;
    bs << mIncludePunctuation;
    bs << mIncludeUpperEuro;
    bs << mIncludeLowerEuro;
    bs << mIncludeRussian;
    bs << mIncludePolish;
    bs << mIncludeLocaleData;
    bs << mIncludeDataFile;
    String utf8;
    WideVectorToUTF8(mPlusChars, utf8);
    bs << utf8;
    WideVectorToUTF8(mMinusChars, utf8);
    bs << utf8;
    bs << mFontName;
    bs << mFontPctSize;
    bs << mFontWeight;
    bs << mIsFontItalic;
    bs << mDropShadow;
    bs << mDropShadowOpacity;
    bs << mPitchAndFamily;
    bs << mFontQuality;
    bs << mFontCharset;
    bs << mFontSupersample;
    bs << mBitmapPath;
    bs << mBitmapFileName;
    bs << mTexPadLeft;
    bs << mTexPadRight;
    bs << mTexPadTop;
    bs << mTexPadBottom;
    bs << mFillWithSafeWhite;
    bs << mGennedFonts;
    bs << mKerningReference;
    bs << mMatVariations;
    bs << mHandmadeFont;
    bs << mSyncResource;
    bs << mLastGenWasNG;
END_SAVES

BEGIN_COPYS(UIFontImporter)
    CREATE_COPY(UIFontImporter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mIncludeLowerCase)
        COPY_MEMBER(mIncludeUpperCase)
        COPY_MEMBER(mIncludeNumbers)
        COPY_MEMBER(mIncludePunctuation)
        COPY_MEMBER(mIncludeUpperEuro)
        COPY_MEMBER(mIncludeLowerEuro)
        COPY_MEMBER(mIncludeRussian)
        COPY_MEMBER(mIncludePolish)
        COPY_MEMBER(mIncludeLocaleData)
        COPY_MEMBER(mIncludeDataFile)
        COPY_MEMBER(mPlusChars)
        COPY_MEMBER(mMinusChars)
        COPY_MEMBER(mFontName)
        COPY_MEMBER(mFontPctSize)
        COPY_MEMBER(mFontWeight)
        COPY_MEMBER(mIsFontItalic)
        COPY_MEMBER(mDropShadow)
        COPY_MEMBER(mDropShadowOpacity)
        COPY_MEMBER(mFontQuality)
        COPY_MEMBER(mPitchAndFamily)
        COPY_MEMBER(mFontQuality)
        COPY_MEMBER(mFontCharset)
        COPY_MEMBER(mBitmapPath)
        COPY_MEMBER(mBitmapFileName)
        COPY_MEMBER(mFontSupersample)
        COPY_MEMBER(mTexPadLeft)
        COPY_MEMBER(mTexPadRight)
        COPY_MEMBER(mTexPadTop)
        COPY_MEMBER(mTexPadBottom)
        COPY_MEMBER(mFillWithSafeWhite)
        COPY_MEMBER(mGennedFonts)
        COPY_MEMBER(mKerningReference)
        COPY_MEMBER(mMatVariations)
        COPY_MEMBER(mHandmadeFont)
        COPY_MEMBER(mSyncResource)
        COPY_MEMBER(mLastGenWasNG)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(10, 4)

BEGIN_LOADS(UIFontImporter)
    LOAD_REVS(bs)
    ASSERT_REVS(10, 4)
    d >> mIncludeLowerCase;
    d >> mIncludeUpperCase;
    d >> mIncludeNumbers;
    d >> mIncludePunctuation;
    d >> mIncludeUpperEuro;
    d >> mIncludeLowerEuro;
    if (d.altRev > 0) {
        d >> mIncludeRussian;
        d >> mIncludePolish;
        d >> mIncludeLocaleData;
        d >> mIncludeDataFile;
    }
    String plus;
    String minus;
    d >> plus;
    d >> minus;
    if (d.altRev < 4) {
        ASCIItoWideVector(mPlusChars, plus.c_str());
        ASCIItoWideVector(mMinusChars, minus.c_str());
    } else {
        UTF8toWideVector(mPlusChars, plus.c_str());
        UTF8toWideVector(mMinusChars, minus.c_str());
    }
    d >> mFontName;
    if (d.rev <= 4) {
        int height;
        d >> height;
        mFontPctSize = ConvertHeightNGToPctHeight(-height);
    } else {
        d >> mFontPctSize;
    }
    d >> mFontWeight;
    d >> mIsFontItalic;
    if (d.altRev > 1) {
        d >> mDropShadow;
    }
    if (d.altRev > 2) {
        d >> mDropShadowOpacity;
    }
    d >> mPitchAndFamily;
    d >> (int &)mFontQuality;
    d >> mFontCharset;
    if (d.rev > 1) {
        d >> (int &)mFontSupersample;
    }
    d >> mBitmapPath;
    d >> mBitmapFileName;
    d >> mTexPadLeft;
    d >> mTexPadRight;
    d >> mTexPadTop;
    d >> mTexPadBottom;
    d >> mFillWithSafeWhite;
    if (d.rev < 8) {
        d >> mImportSettingsFont;
    }
    if (d.rev > 2) {
        d >> mGennedFonts;
        d >> mKerningReference;
    }
    if (d.rev == 3) {
        ObjPtr<RndMat> mat(this);
        d >> mat;
    }
    if (d.rev > 3) {
        d >> mMatVariations;
    }
    if (d.rev > 5 && d.rev < 10) {
        ObjPtr<RndMat> mat(this);
        d >> mat;
    }
    if (d.rev > 6) {
        d >> mHandmadeFont;
    }
    if (d.rev > 7) {
        d >> mSyncResource;
    }
    if (d.rev > 8) {
        d >> mLastGenWasNG;
    }
    if (d.altRev == 1) {
        int x;
        d >> x;
    }
END_LOADS

void UIFontImporter::ImportSettingsFromFont(RndFontBase *font) {
    if (font && font->Type() == Symbol("imported_font")) {
        SetProperty("font_name", font->Property("font_name")->Str());
        SetProperty(
            "font_size", ConvertHeightNGToPctHeight(-font->Property("font_size")->Int())
        );
        SetProperty("weight", font->Property("weight")->Int());
        SetProperty("italics", font->Property("italics")->Int());
        SetProperty("drop_shadow", font->Property("drop_shadow")->Int());
        SetProperty("drop_shadow_opacity", font->Property("drop_shadow_opacity")->Int());
        SetProperty("left", font->Property("left")->Int());
        SetProperty("right", font->Property("right")->Int());
        SetProperty("top", font->Property("top")->Int());
        SetProperty("bottom", font->Property("bottom")->Int());
    } else {
        MILO_NOTIFY(
            "Can't import settings from Font because it doesnt have import_font type"
        );
    }
}

int UIFontImporter::GetMatVariationIdx(Symbol s) const {
    int size = NumMatVariations();
    for (int ret = 0; ret < size; ret++) {
        Symbol name = GetMatVariationName(ret);
        if (name == s) {
            return ret;
        }
    }
    return -1;
}

void UIFontImporter::AttachImporterToFont(RndFontBase *font) {
    if (font) {
        if (font->Dir() != Dir())
            MILO_NOTIFY(
                "Cannot attach font %s to font resource %s because its in a different dir.  Notify a programmer!"
            );
        else {
            mGennedFonts.clear();
            mMatVariations.clear();
            mGennedFonts.push_back(font);
            mKerningReference = font;
            ImportSettingsFromFont(font);
        }
    }
}

void UIFontImporter::GenerateBitmapFilename() {
    const char *mult = "";
    if (mFontSupersample == kFontSuperSample_2x)
        mult = "2x";
    else if (mFontSupersample == kFontSuperSample_4x)
        mult = "4x";

    class String s28(MakeString("%.2f", mFontPctSize * 100.0f));
    s28.ReplaceAll('.', '_');
    const char *s = mDropShadow ? "S" : "";
    const char *b = (mFontWeight > 500) ? "B" : "";
    const char *i = mIsFontItalic ? "I" : "";
    mBitmapFileName =
        MakeString("%s(%s)%s%s%s%s.bmp", mFontName.c_str(), s28.c_str(), i, b, s, mult);
    mBitmapFileName.ReplaceAll(' ', '_');
}

RndFontBase *UIFontImporter::FindFontForMat(RndMat *mat) const {
    if (mat) {
        static Symbol Font("Font");
        static Symbol Font3d("Font3d");
        FOREACH_OBJREF (it, mat) {
            Hmx::Object *owner = (*it).RefOwner();
            if (owner) {
                if (owner->ClassName() == Font) {
                    return dynamic_cast<RndFont *>(owner);
                }
                if (owner->ClassName() == Font3d)
                    return dynamic_cast<RndFont3d *>(owner);
            }
        }
    }
    return nullptr;
}

void UIFontImporter::OnSetCharsetUTF8(String const &s) {
    mIncludeLowerEuro = false;
    mIncludeUpperEuro = false;
    mIncludePunctuation = false;
    mIncludeNumbers = false;
    mIncludeLowerCase = false;
    mIncludeUpperCase = false;
    mIncludeLocaleData = false;
    mIncludePolish = false;
    mIncludeRussian = false;
    mIncludeDataFile = "";
    mMinusChars.clear();
    mPlusChars.clear();
    UTF8toWideVector(mPlusChars, s.c_str());
}

RndText *UIFontImporter::FindTextForFont(RndFontBase *font) const {
    if (font) {
        static Symbol Text("Text");
        FOREACH_OBJREF (it, font) {
            Hmx::Object *owner = it->RefOwner();
            if (owner && owner->ClassName() == Text) {
                RndText *text = dynamic_cast<RndText *>(owner);
                if (text->DefaultStyle().font == font) {
                    return text;
                }
            }
        }
    }
    return nullptr;
}

String UIFontImporter::GetASCIIPlusChars() {
    static String plusChars;
    plusChars = WideVectorToASCII(mPlusChars);
    return plusChars;
}

String UIFontImporter::GetASCIIMinusChars() {
    static String minusChars;
    minusChars = WideVectorToASCII(mMinusChars);
    return minusChars;
}

Symbol UIFontImporter::GetMatVariationName(unsigned int idx) const {
    if (idx >= mMatVariations.size()) {
        return Symbol();
    } else {
        auto it = mMatVariations.begin();
        for (int i = 0; i < idx; i++) {
            ++it;
        }
        return FileGetBase((*it)->Name());
    }
}

RndFontBase *UIFontImporter::GetGennedFont(unsigned int idx) const {
    if (idx >= mGennedFonts.size()) {
        return nullptr;
    } else {
        auto it = mGennedFonts.begin();
        for (int i = 0; i < idx; i++) {
            ++it;
        }
        return *it;
    }
}

const char *UIFontImporter::GetMatVariationName(RndFontBase *font) const {
    if (font && font->Mat()) {
        RndMat *mat = font->Mat();
        if (mGennedFonts.size() > 0) {
            if (mat == GetGennedFont(0)->Mat()) {
                return "";
            }
        }
        if (mMatVariations.size() != 0) {
            FOREACH (it, mMatVariations) {
                if (*it == mat) {
                    return FileGetBase(mat->Name());
                }
            }
            MILO_NOTIFY("%s not found in resource dir %s", PathName(font), PathName(this));
        }
    }
    return "";
}

RndMat *UIFontImporter::GetMatVariation(unsigned int idx) const {
    if (idx >= mMatVariations.size()) {
        return nullptr;
    } else {
        auto it = mMatVariations.begin();
        for (int i = 0; i != idx; i++) {
            ++it;
        }
        return *it;
    }
}

RndFontBase *UIFontImporter::GetGennedFont(Symbol s) const {
    if (s.Null()) {
        return *mGennedFonts.begin();
    } else {
        unsigned int idx = GetMatVariationIdx(s);
        if (idx == -1) {
            return nullptr;
        } else {
            return FindFontForMat(GetMatVariation(idx));
        }
    }
}

void UIFontImporter::SyncWithGennedFonts() {
    auto it = mGennedFonts.begin();
    for (int i = 0; it != mGennedFonts.end(); i++) {
        RndFontBase *cur = *it;
        bool b4 = false;
        if (i == 0) {
            b4 = true;
        } else {
            FOREACH (mit, mMatVariations) {
                if (cur->Mat() == *mit) {
                    b4 = true;
                }
            }
        }
        if (!b4) {
            cur->Mat();
            RndText *text = FindTextForFont(cur);
            it = mGennedFonts.erase(it);
            delete cur;
            if (text) {
                delete text;
            }
        } else {
            ++it;
        }
    }
}

void UIFontImporter::HandmadeFontChanged() {
    if (mHandmadeFont) {
        if (mGennedFonts.size() > 0) {
            RndFontBase *font = *mGennedFonts.begin();
            if (font != mHandmadeFont) {
                RndText *text = FindTextForFont(font);
                delete font;
                delete text;
            }
            mGennedFonts.Set(mGennedFonts.begin(), mHandmadeFont);
            for (auto it = NextItr(mGennedFonts.begin(), 1); it != mGennedFonts.end();
                 ++it) {
                if (*it == mHandmadeFont) {
                    mGennedFonts.erase(it);
                    break;
                }
            }
        } else {
            mGennedFonts.push_back(mHandmadeFont);
        }
        mKerningReference = mHandmadeFont;
        mIncludeLowerEuro = false;
        mIncludeUpperEuro = false;
        mIncludePunctuation = false;
        mIncludeNumbers = false;
        mIncludeLowerCase = false;
        mIncludeUpperCase = false;
        mIncludeLocaleData = false;
        mIncludePolish = false;
        mIncludeRussian = false;
        mIncludeDataFile = "";
        mMinusChars.clear();
        mPlusChars = mHandmadeFont->Chars();
    }
    if (IsHandmade()) {
        mHandmadeFont->ClassName() == RndFont3d::StaticClassName();
    }
}

DataNode UIFontImporter::OnGetGennedBitmapPath(DataArray *da) {
    if ((unsigned int)mGennedFonts.size() > 0) {
        RndFont *font = static_cast<RndFont *>(*mGennedFonts.begin());
        if (font && font->Mat(0) && font->Mat(0)->DiffuseTex()) {
            RndTex *tex = font->Mat(0)->DiffuseTex();
            if (tex) {
                return tex->File().c_str();
            }
        }
    }
    return "";
}

DataNode UIFontImporter::OnImportSettings(DataArray *da) {
    ImportSettingsFromFont(mImportSettingsFont);
    return 0;
}

DataNode UIFontImporter::OnForgetGened(DataArray *) {
    mGennedFonts.clear();
    return 0;
}

DataNode UIFontImporter::OnAttachToImportFont(DataArray *) {
    AttachImporterToFont(mImportSettingsFont);
    return 0;
}

DataNode UIFontImporter::OnGenerate(DataArray *a) {
    if (a->Size() >= 3) {
        a->Int(2);
    }
    return 0;
}

DataNode UIFontImporter::OnGenerate3d(DataArray *a) {
    if (a->Size() >= 3) {
        a->Int(2);
    }
    return 0;
}

DataNode UIFontImporter::OnGenerateOG(DataArray *a) {
    if (a->Size() >= 3) {
        a->Int(2);
    }
    return 0;
}

DataNode UIFontImporter::OnShowFontPicker(DataArray *) { return 0; }

DataNode UIFontImporter::OnSyncWithResourceFile(DataArray *a) {
    if (!mSyncResource.empty()) {
        FilePath path;
        if (ResourceDirBase::MakeResourcePath(
                path, "UILabel", "UILabelDir", mSyncResource.c_str()
            )) {
            ObjDirPtr<UILabelDir> labelDir;
            labelDir.LoadFile(path, false, true, kLoadFront, false);
            if (labelDir.IsLoaded()) {
                mIncludeLowerCase = labelDir->mIncludeLowerCase;
                mIncludeUpperCase = labelDir->mIncludeUpperCase;
                mIncludeNumbers = labelDir->mIncludeNumbers;
                mIncludePunctuation = labelDir->mIncludePunctuation;
                mIncludeUpperEuro = labelDir->mIncludeUpperEuro;
                mIncludeLowerEuro = labelDir->mIncludeLowerEuro;
                mIncludeRussian = labelDir->mIncludeRussian;
                mIncludePolish = labelDir->mIncludePolish;
                mIncludeLocaleData = labelDir->mIncludeLocaleData;
                mIncludeDataFile = labelDir->mIncludeDataFile;
                mPlusChars = labelDir->mPlusChars;
                mMinusChars = labelDir->mMinusChars;
                mFontName = labelDir->mFontName;
                mFontPctSize = labelDir->mFontPctSize;
                mFontWeight = labelDir->mFontWeight;
                mIsFontItalic = labelDir->mIsFontItalic;
                mDropShadow = labelDir->mDropShadow;
                mDropShadowOpacity = labelDir->mDropShadowOpacity;
                mFontQuality = labelDir->mFontQuality;
                mPitchAndFamily = labelDir->mPitchAndFamily;
                mFontQuality = labelDir->mFontQuality;
                mFontCharset = labelDir->mFontCharset;
                mBitmapPath = labelDir->mBitmapPath;
                mBitmapFileName = labelDir->mBitmapFileName;
                mFontSupersample = labelDir->mFontSupersample;
                mTexPadLeft = labelDir->mTexPadLeft;
                mTexPadRight = labelDir->mTexPadRight;
                mTexPadTop = labelDir->mTexPadTop;
                mTexPadBottom = labelDir->mTexPadBottom;
                mFillWithSafeWhite = labelDir->mFillWithSafeWhite;
                if (mKerningReference && labelDir->mKerningReference) {
                    std::vector<RndFontBase::KernInfo> kernInfo;
                    labelDir->mKerningReference->GetKerning(kernInfo);
                    mKerningReference->SetKerning(kernInfo);
                    mKerningReference->SetBaseKerning(
                        labelDir->mKerningReference->BaseKerning()
                    );
                }
            }
        }
    }
    return 0;
}
