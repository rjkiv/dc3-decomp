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

float ConvertHeightOGToPctHeight(int i) { return std::fabs(-i / HEIGHT_SD); }
float ConvertHeightNGToPctHeight(int i) { return std::fabs(-i / HEIGHT_HD); }
int ConvertPctHeightToHeightNG(float f) { return -Round(f * HEIGHT_HD); }
int ConvertPctHeightToHeightOG(float f) { return -Round(f * HEIGHT_SD); }

UIFontImporter::UIFontImporter()
    : mUpperCaseAthroughZ(1), mLowerCaseAthroughZ(1), mNumbers0through9(1),
      mPunctuation(1), mUpperEuro(1), mLowerEuro(1), mRussian(0), mPolish(0),
      mIncludeLocale(0), mIncludeFile(""), mFontName("Arial"),
      mFontPctSize(ConvertHeightNGToPctHeight(12)), mFontWeight(400), mItalics(false),
      mDropShadow(0), mDropShadowOpacity(128), mFontQuality(0), mPitchAndFamily(34),
      mFontCharset(0), mFontSupersample(0), mLeft(0), mRight(0), mTop(0), mBottom(0),
      mFillWithSafeWhite(false), mFontToImportFrom(this), mBitmapSavePath("ui/image/"),
      mBitMapSaveName("temp.bmp"), mGennedFonts(this), mReferenceKerning(this),
      mMatVariations(this), mHandmadeFont(this), mCheckNG(false), mLastGenWasNG(true) {
    static Symbol objects("objects");
    static Symbol default_bitmap_path("default_bitmap_path");
    DataArray *cfgArr =
        SystemConfig(objects, StaticClassName())->FindArray(default_bitmap_path, false);
    if (cfgArr) {
        mBitmapSavePath = cfgArr->Str(1);
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
    SYNC_PROP(UPPER_CASE_A_Z, mUpperCaseAthroughZ)
    SYNC_PROP(lower_case_a_z, mLowerCaseAthroughZ)
    SYNC_PROP(numbers_0_9, mNumbers0through9)
    SYNC_PROP(punctuation, mPunctuation)
    SYNC_PROP(UPPER_EURO, mUpperEuro)
    SYNC_PROP(lower_euro, mLowerEuro)
    SYNC_PROP(russian, mRussian)
    SYNC_PROP(polish, mPolish)
    SYNC_PROP(include_locale, mIncludeLocale)
    SYNC_PROP(include_file, mIncludeFile)
    SYNC_PROP_SET(plus, GetASCIIPlusChars(), ASCIItoWideVector(mPlus, _val.Str()))
    SYNC_PROP_SET(minus, GetASCIIMinusChars(), ASCIItoWideVector(mMinus, _val.Str()))
    SYNC_PROP(font_name, mFontName)
    SYNC_PROP_MODIFY(font_pct_size, mFontPctSize, GenerateBitmapFilename())
    SYNC_PROP_SET(
        font_point_size,
        mLastGenWasNG ? ConvertPctHeightToHeightNG(mFontPctSize)
                      : ConvertPctHeightToHeightOG(mFontPctSize),
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(_val.Int())
                                     : ConvertHeightOGToPctHeight(_val.Int())
    )
    SYNC_PROP_SET(
        font_pixel_size,
        std::abs(
            mLastGenWasNG ? ConvertPctHeightToHeightNG(mFontPctSize)
                          : ConvertPctHeightToHeightOG(mFontPctSize)
        ),
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(_val.Int())
                                     : ConvertHeightOGToPctHeight(_val.Int())
    )
    SYNC_PROP_MODIFY(weight, mFontWeight, GenerateBitmapFilename())
    SYNC_PROP_SET(
        bold, std::abs(mFontWeight), mFontWeight = _val.Int() != 0 ? 800 : 400;
        GenerateBitmapFilename()
    )
    SYNC_PROP_MODIFY(italics, mItalics, GenerateBitmapFilename())
    SYNC_PROP_MODIFY(drop_shadow, mDropShadow, GenerateBitmapFilename())
    SYNC_PROP(drop_shadow_opacity, mDropShadowOpacity)
    SYNC_PROP(font_quality, (int &)mFontQuality)
    SYNC_PROP(pitch_and_family, mPitchAndFamily)
    SYNC_PROP(font_charset, mFontCharset)
    SYNC_PROP_MODIFY(font_supersample, (int &)mFontSupersample, GenerateBitmapFilename())
    SYNC_PROP(left, mLeft)
    SYNC_PROP(right, mRight)
    SYNC_PROP(top, mTop)
    SYNC_PROP(bottom, mBottom)
    SYNC_PROP(fill_with_safe_white, mFillWithSafeWhite)
    SYNC_PROP(font_to_import_from, mFontToImportFrom)
    SYNC_PROP(bitmap_save_path, mBitmapSavePath)
    SYNC_PROP(bitmap_save_name, mBitMapSaveName)
    SYNC_PROP(gened_fonts, mGennedFonts)
    SYNC_PROP(reference_kerning, mReferenceKerning)
    SYNC_PROP_MODIFY(mat_variations, mMatVariations, SyncWithGennedFonts())
    SYNC_PROP_MODIFY(handmade_font, mHandmadeFont, HandmadeFontChanged())
    SYNC_PROP(resource_name, mSyncResource)
    SYNC_PROP(last_genned_ng, mLastGenWasNG)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(UIFontImporter)
    SAVE_REVS(10, 4)
    bs << mLowerCaseAthroughZ;
    bs << mUpperCaseAthroughZ;
    bs << mNumbers0through9;
    bs << mPunctuation;
    bs << mUpperEuro;
    bs << mLowerEuro;
    bs << mRussian;
    bs << mPolish;
    bs << mIncludeLocale;
    bs << mIncludeFile;
    String utf8;
    WideVectorToUTF8(mPlus, utf8);
    bs << utf8;
    WideVectorToUTF8(mMinus, utf8);
    bs << utf8;
    bs << mFontName;
    bs << mFontPctSize;
    bs << mFontWeight;
    bs << mItalics;
    bs << mDropShadow;
    bs << mDropShadowOpacity;
    bs << mPitchAndFamily;
    bs << mFontQuality;
    bs << mFontCharset;
    bs << mFontSupersample;
    bs << mBitmapSavePath;
    bs << mBitMapSaveName;
    bs << mLeft;
    bs << mRight;
    bs << mTop;
    bs << mBottom;
    bs << mFillWithSafeWhite;
    bs << mGennedFonts;
    bs << mReferenceKerning;
    bs << mMatVariations;
    bs << mHandmadeFont;
    bs << mSyncResource;
    bs << mLastGenWasNG;
END_SAVES

BEGIN_COPYS(UIFontImporter)
    CREATE_COPY(UIFontImporter)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mLowerCaseAthroughZ)
        COPY_MEMBER(mUpperCaseAthroughZ)
        COPY_MEMBER(mNumbers0through9)
        COPY_MEMBER(mPunctuation)
        COPY_MEMBER(mUpperEuro)
        COPY_MEMBER(mLowerEuro)
        COPY_MEMBER(mRussian)
        COPY_MEMBER(mPolish)
        COPY_MEMBER(mIncludeLocale)
        COPY_MEMBER(mIncludeFile)
        COPY_MEMBER(mPlus)
        COPY_MEMBER(mMinus)
        COPY_MEMBER(mFontName)
        COPY_MEMBER(mFontPctSize)
        COPY_MEMBER(mFontWeight)
        COPY_MEMBER(mItalics)
        COPY_MEMBER(mDropShadow)
        COPY_MEMBER(mDropShadowOpacity)
        COPY_MEMBER(mFontQuality)
        COPY_MEMBER(mPitchAndFamily)
        COPY_MEMBER(mFontQuality)
        COPY_MEMBER(mFontCharset)
        COPY_MEMBER(mBitmapSavePath)
        COPY_MEMBER(mBitMapSaveName)
        COPY_MEMBER(mFontSupersample)
        COPY_MEMBER(mLeft)
        COPY_MEMBER(mRight)
        COPY_MEMBER(mTop)
        COPY_MEMBER(mBottom)
        COPY_MEMBER(mFillWithSafeWhite)
        COPY_MEMBER(mGennedFonts)
        COPY_MEMBER(mReferenceKerning)
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
    d >> mLowerCaseAthroughZ;
    d >> mUpperCaseAthroughZ;
    d >> mNumbers0through9;
    d >> mPunctuation;
    d >> mUpperEuro;
    d >> mLowerEuro;
    if (d.rev > 0) {
        d >> mRussian;
        d >> mPolish;
        d >> mIncludeLocale;
        d >> mIncludeFile;
    }
    String plus;
    String minus;
    d >> plus;
    d >> minus;
    if (d.rev < 4) {
        ASCIItoWideVector(mPlus, plus.c_str());
        ASCIItoWideVector(mMinus, minus.c_str());
    } else {
        UTF8toWideVector(mPlus, plus.c_str());
        UTF8toWideVector(mMinus, minus.c_str());
    }
    d >> mFontName;
    if (d.rev <= 4) {
        int height;
        d >> height;
        mFontPctSize = ConvertHeightNGToPctHeight(height);
    } else {
        d >> mFontPctSize;
    }
    d >> mFontWeight;
    d >> mItalics;
    if (d.rev > 1) {
        d >> mDropShadow;
    }
    if (d.rev > 2) {
        d >> mDropShadowOpacity;
    }
    d >> mPitchAndFamily;
    d >> mFontQuality;
    d >> mFontCharset;
    if (d.rev > 1) {
        d >> mFontSupersample;
    }
    d >> mBitmapSavePath;
    d >> mBitMapSaveName;
    d >> mLeft;
    d >> mRight;
    d >> mTop;
    d >> mBottom;
    d >> mFillWithSafeWhite;
    if (d.rev < 8) {
        d >> mFontToImportFrom;
    }
    if (d.rev > 2) {
        d >> mGennedFonts;
        d >> mReferenceKerning;
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
            "font_size", ConvertHeightNGToPctHeight(font->Property("font_size")->Int())
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
            mReferenceKerning = font;
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
    const char *i = mItalics ? "I" : "";
    mBitMapSaveName =
        MakeString("%s(%s)%s%s%s%s.bmp", mFontName.c_str(), s28.c_str(), i, b, s, mult);
    mBitMapSaveName.ReplaceAll(' ', '_');
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
    mLowerEuro = false;
    mUpperEuro = false;
    mPunctuation = false;
    mNumbers0through9 = false;
    mLowerCaseAthroughZ = false;
    mUpperCaseAthroughZ = false;
    mIncludeLocale = false;
    mPolish = false;
    mRussian = false;
    mIncludeFile = "";
    mMinus.clear();
    mPlus.clear();
    UTF8toWideVector(mPlus, s.c_str());
}

RndText *UIFontImporter::FindTextForFont(RndFontBase *font) const {
    if (font) {
        static Symbol Text("Text");
        FOREACH_OBJREF (it, font) {
            Hmx::Object *owner = it->RefOwner();
            if (owner && owner->ClassName() == Text) {
                RndText *text = dynamic_cast<RndText *>(owner);
                if (text->Styles()[0].mFont == font) {
                    return text;
                }
            }
        }
    }
    return nullptr;
}

String UIFontImporter::GetASCIIPlusChars() {
    static String plusChars;
    plusChars = WideVectorToASCII(mPlus);
    return plusChars;
}

String UIFontImporter::GetASCIIMinusChars() {
    static String minusChars;
    minusChars = WideVectorToASCII(mMinus);
    return minusChars;
}

Symbol UIFontImporter::GetMatVariationName(unsigned int ui) const {
    if (ui >= mMatVariations.size()) {
        return Symbol();
    } else {
        auto it = mMatVariations.begin();
        for (int i = 0; i < ui; i++) {
            ++it;
        }
        return FileGetBase((*it)->Name());
    }
}

const char *UIFontImporter::GetMatVariationName(RndFontBase *font) const {
    if (font && font->Mat()) {
        RndMat *mat = font->Mat();
        if (mGennedFonts.size() > 0) {
            RndFontBase *front =
                mGennedFonts.size() != 0 ? *mGennedFonts.begin() : nullptr;
            if (mat == front->Mat()) {
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

RndFontBase *UIFontImporter::GetGennedFont(Symbol s) const {
    if (s.Null()) {
        return *mGennedFonts.begin();
    } else {
        int idx = GetMatVariationIdx(s);
        if (idx == -1) {
            return nullptr;
        } else {
            RndMat *mat;
            if (idx >= mMatVariations.size()) {
                mat = nullptr;
            } else {
                auto it = mMatVariations.begin();
                for (int i = 0; i != idx; i++) {
                    ++it;
                }
                mat = *it;
            }
            return FindFontForMat(mat);
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
            // <?>
            RndFontBase *next = *mGennedFonts.begin();
            next = mHandmadeFont;
            // </?>
            FOREACH (it, mGennedFonts) {
                if (*it == mHandmadeFont) {
                    mGennedFonts.erase(it);
                    break;
                }
            }
        } else {
            mGennedFonts.push_back(mHandmadeFont);
        }
        mReferenceKerning = mHandmadeFont;
        mLowerEuro = false;
        mUpperEuro = false;
        mPunctuation = false;
        mNumbers0through9 = false;
        mLowerCaseAthroughZ = false;
        mUpperCaseAthroughZ = false;
        mIncludeLocale = false;
        mPolish = false;
        mRussian = false;
        mIncludeFile = "";
        mMinus.clear();
        mPlus = mHandmadeFont->Chars();
    }
    if (mHandmadeFont) {
        RndFont3d::StaticClassName();
        mHandmadeFont->ClassName();
    }
}

DataNode UIFontImporter::OnGetGennedBitmapPath(DataArray *da) {
    if ((unsigned int)mGennedFonts.size() > 0) {
        RndFont *font = static_cast<RndFont *>(*mGennedFonts.begin());
        if (font && font->Mat(0) && font->Mat(0)->GetDiffuseTex()) {
            RndTex *tex = font->Mat(0)->GetDiffuseTex();
            if (tex) {
                return tex->File().c_str();
            }
        }
    }
    return "";
}

DataNode UIFontImporter::OnImportSettings(DataArray *da) {
    ImportSettingsFromFont(mFontToImportFrom);
    return 0;
}

DataNode UIFontImporter::OnForgetGened(DataArray *) {
    mGennedFonts.clear();
    return 0;
}

DataNode UIFontImporter::OnAttachToImportFont(DataArray *) {
    AttachImporterToFont(mFontToImportFrom);
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
                mLowerCaseAthroughZ = labelDir->mLowerCaseAthroughZ;
                mUpperCaseAthroughZ = labelDir->mUpperCaseAthroughZ;
                mNumbers0through9 = labelDir->mNumbers0through9;
                mPunctuation = labelDir->mPunctuation;
                mUpperEuro = labelDir->mUpperEuro;
                mLowerEuro = labelDir->mLowerEuro;
                mRussian = labelDir->mRussian;
                mPolish = labelDir->mPolish;
                mIncludeLocale = labelDir->mIncludeLocale;
                mIncludeFile = labelDir->mIncludeFile;
                mPlus = labelDir->mPlus;
                mMinus = labelDir->mMinus;
                mFontName = labelDir->mFontName;
                mFontPctSize = labelDir->mFontPctSize;
                mFontWeight = labelDir->mFontWeight;
                mItalics = labelDir->mItalics;
                mDropShadow = labelDir->mDropShadow;
                mDropShadowOpacity = labelDir->mDropShadowOpacity;
                mFontQuality = labelDir->mFontQuality;
                mPitchAndFamily = labelDir->mPitchAndFamily;
                mFontQuality = labelDir->mFontQuality;
                mFontCharset = labelDir->mFontCharset;
                mBitmapSavePath = labelDir->mBitmapSavePath;
                mBitMapSaveName = labelDir->mBitMapSaveName;
                mFontSupersample = labelDir->mFontSupersample;
                mLeft = labelDir->mLeft;
                mRight = labelDir->mRight;
                mTop = labelDir->mTop;
                mBottom = labelDir->mBottom;
                mFillWithSafeWhite = labelDir->mFillWithSafeWhite;
                if (mReferenceKerning && labelDir->mReferenceKerning) {
                    std::vector<RndFontBase::KernInfo> kernInfo;
                    labelDir->mReferenceKerning->GetKerning(kernInfo);
                    mReferenceKerning->SetKerning(kernInfo);
                    mReferenceKerning->SetBaseKerning(
                        labelDir->mReferenceKerning->BaseKerning()
                    );
                }
            }
        }
    }
    return 0;
}
