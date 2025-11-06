#include "ui/UIFontImporter.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/System.h"
#include "utl/UTF8.h"

#define HEIGHT_SD 480.0f
#define HEIGHT_HD 720.0f

float ConvertHeightOGToPctHeight(int i) { return std::fabs(i / HEIGHT_SD); }
float ConvertHeightNGToPctHeight(int i) { return std::fabs(i / HEIGHT_HD); }
int ConvertPctHeightToHeightNG(float f) { return -Round(f * HEIGHT_HD); }
int ConvertPctHeightToHeightOG(float f) { return -Round(f * HEIGHT_SD); }

UIFontImporter::UIFontImporter()
    : mUpperCaseAthroughZ(1), mLowerCaseAthroughZ(1), mNumbers0through9(1),
      mPunctuation(1), mUpperEuro(1), mLowerEuro(1), mRussian(0), mPolish(0),
      mIncludeLocale(0), mIncludeFile(""), mFontName("Arial"),
      mFontPctSize(ConvertHeightNGToPctHeight(-12)), mFontWeight(400), mItalics(false),
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
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(-_val.Int())
                                     : ConvertHeightOGToPctHeight(-_val.Int())
    )
    SYNC_PROP_SET(
        font_pixel_size,
        std::abs(
            mLastGenWasNG ? ConvertPctHeightToHeightNG(mFontPctSize)
                          : ConvertPctHeightToHeightOG(mFontPctSize)
        ),
        mFontPctSize = mLastGenWasNG ? ConvertHeightNGToPctHeight(-_val.Int())
                                     : ConvertHeightOGToPctHeight(-_val.Int())
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
