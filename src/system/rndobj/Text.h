#pragma once
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/FontBase.h"
#include "rndobj/Font.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class TextHolder {
public:
    TextHolder() {}
    virtual ~TextHolder() {}
    virtual void SetTextToken(Symbol) = 0;
    virtual void SetInt(int, bool) = 0;
};

class RndText : public virtual RndDrawable, public virtual RndTransformable {
public:
    enum Alignment {
        kCenter = 2,
        kTopLeft = 0x11,
        kTopCenter = 0x12,
        kTopRight = 0x14,
        kMiddleLeft = 0x21,
        kMiddleCenter = 0x22,
        kMiddleRight = 0x24,
        kBottomLeft = 0x41,
        kBottomCenter = 0x42,
        kBottomRight = 0x44
    };

    enum CapsMode {
        /** "Leave the text as is" */
        kCapsModeNone = 0,
        /** "Force text to all lower case" */
        kForceLower = 1,
        /** "Force text to all upper case" */
        kForceUpper = 2,
    };

    enum FitType {
        /** "Performs normal line wrapping if [width] is set" */
        kFitWrap = 0,
        // where 1 lol
        /** "Shrinks the text until it fits within [width] and [height].
            Note that this is a very expensive process, super slow,
            and so should never be used on dynamically changing text when in game" */
        kFitJust = 2,
        /** "Constrains the text to one line of [width] with ellipses" */
        kFitEllipsis = 3,
        /** "Continuous right-to-left scrolling. String start follows sring end" */
        kFitScrollMarqueeWrap = 4,
        /** "Right-to-left scroll - Reset to beginning after end scrolls off" */
        kFitScrollMarqueeReset = 5,
        /** "Reverse scroll direction whenever string end or beginning is reached" */
        kFitScrollPingPong = 6,
        /** "Continuous right-to-left scroll with wrapping and not care about string size.
            '\n' will be replaced with indentation." */
        kFitScrollMarqueeWrapAlways = 7
    };

    struct StyleInfo {
        StyleInfo()
            : mSize(30), mTextColor(1, 1, 1), mFontColorOverride(false),
              mFontColor(1, 1, 1), mItalics(0), mKerning(0), mZOffset(0) {}
        /** "Size of the text" */
        float mSize; // 0x0
        /** "Color of the text, put into mesh verts.
            Modifed by <color=r,g,b,a> markup .
            This will only work if the font mat has [prelit] set true
            and [use_environment] set false" */
        Hmx::Color mTextColor; // 0x4
        /** "If true, and if there's a font,
            you can change color and alpha during the draw" */
        bool mFontColorOverride; // 0x14
        /** "Color of the font during draw, can be changed dynamically" */
        Hmx::Color mFontColor; // 0x18
        /** "Defines the slant of the text, changed by <it> tag".
            Ranges from -5 to 5. */
        float mItalics; // 0x28
        /** "Extra kerning for the text" */
        float mKerning; // 0x2c
        /** "vertical offset as fraction of size" */
        float mZOffset; // 0x30
    };

    class Style {
    public:
        Style(Hmx::Object *owner);
        Style &operator=(const Style &s) {
            mFont = s.mFont;
            mBlacklight = s.mBlacklight;
            mInfo = s.mInfo;
            return *this;
        }

        StyleInfo mInfo; // 0x0
        /** "Font to use for this style" */
        ObjPtr<RndFontBase> mFont; // 0x34
        /** "draw in blacklight pass?" */
        bool mBlacklight; // 0x48
    };

    class StyleState {
        friend class RndText;

    public:
        StyleState(RndText *, float);

        StyleInfo mInfo; // 0x0
        Style *unk34; // 0x34
        int unk38; // 0x38
        float unk3c; // 0x3c
        bool unk40; // 0x40
    };

    // size 0x20
    class BlacklightPacket {
    public:
        RndMesh *unk0; // 0x0
        Hmx::Color unk4; // 0x4
        float unk14; // 0x14
        int unk18; // 0x18
        RndCam *unk1c; // 0x1c
    };

    // size 0x14
    class Line {
    public:
        std::vector<unsigned short> unk0; // 0x0
        float unkc;
        float unk10;
    };

    class FontMapBase {
    public:
        FontMapBase() : mBlacklight(false) {}
        virtual ~FontMapBase() {}
        virtual Symbol ClassName() const = 0;
        virtual void SetFont(RndFontBase *) = 0;
        virtual RndFontBase *Font() const = 0;
        virtual int NumMeshes() const = 0;
        virtual RndMesh *Mesh(int) const = 0;
        virtual int NumMaterials() const = 0;
        virtual RndMat *Material(int) const = 0;
        virtual void ResetDisplayableChars() = 0;
        virtual void IncrementDisplayableChars(unsigned short) = 0;
        virtual void AllocateMeshes(RndText *, int) = 0;
        virtual void CleanupSyncMeshes() = 0;
        virtual void SetupCharacter(
            unsigned short,
            float &,
            float,
            const StyleState &state,
            unsigned short,
            float circle,
            FitType fitType,
            float indentation
        ) = 0;
        virtual bool SupportsScrolling() const = 0;
        virtual void SetupScrolling() = 0;
        virtual void UpdateScrolling(float) = 0;

        MEM_OVERLOAD(FontMapBase, 0xD7);

        bool mBlacklight; // 0x4
    };

    // size 0x18
    class FontMap : public FontMapBase {
    public:
        // size 0x10
        class Page {
        public:
            Page() : mesh(nullptr) {}
            ~Page() {
                if (mesh) {
                    RELEASE(mesh);
                }
            }
            MEM_OVERLOAD(Page, 0x115);

            RndMesh *mesh; // 0x0
            int displayableChars; // 0x4
            RndMesh::Vert *unk8; // 0x8 - vert iterator/pointer?
            int unkc; // 0xc - mesh sync flags?
        };

        FontMap() : mFont(nullptr) {}
        virtual ~FontMap();
        virtual Symbol ClassName() const { return StaticClassName(); }
        virtual void SetFont(RndFontBase *);
        virtual RndFontBase *Font() const { return mFont; }
        virtual int NumMeshes() const { return mPages.size(); }
        virtual RndMesh *Mesh(int idx) const { return mPages[idx]->mesh; }
        virtual int NumMaterials() const { return mPages.size(); }
        virtual RndMat *Material(int idx) const { return mFont->Mat(idx); }
        virtual void ResetDisplayableChars();
        virtual void IncrementDisplayableChars(unsigned short);
        virtual void AllocateMeshes(RndText *, int);
        virtual void CleanupSyncMeshes();
        virtual void SetupCharacter(
            unsigned short,
            float &,
            float,
            const StyleState &,
            unsigned short,
            float,
            FitType,
            float
        );
        virtual bool SupportsScrolling() const { return true; }
        virtual void SetupScrolling();
        virtual void UpdateScrolling(float);

        static Symbol StaticClassName() {
            static Symbol name("FontMap");
            return name;
        }

        RndFont *mFont; // 0x8
        std::vector<Page *> mPages; // 0xc
    };

    // size 0x20
    class FontMap3d : public FontMapBase {
    public:
        FontMap3d() : mFont(nullptr), mDisplayableChars(0) {}
        virtual ~FontMap3d();
        virtual Symbol ClassName() const { return StaticClassName(); }
        virtual void SetFont(RndFontBase *);
        virtual RndFontBase *Font() const { return mFont; }
        virtual int NumMeshes() const { return mMeshes.size(); }
        virtual RndMesh *Mesh(int idx) const { return mMeshes[idx]; }
        virtual int NumMaterials() const { return mFont && mFont->Mat(); }
        virtual RndMat *Material(int i) const {
            MILO_ASSERT(i==0, 0x150);
            return mFont->Mat();
        }
        virtual void ResetDisplayableChars() { mDisplayableChars = 0; }
        virtual void IncrementDisplayableChars(unsigned short);
        virtual void AllocateMeshes(RndText *, int);
        virtual void CleanupSyncMeshes();
        virtual void SetupCharacter(
            unsigned short,
            float &,
            float,
            const StyleState &,
            unsigned short,
            float,
            FitType,
            float
        );
        virtual bool SupportsScrolling() const { return false; }
        virtual void SetupScrolling() {}
        virtual void UpdateScrolling(float) {}

        static Symbol StaticClassName() {
            static Symbol name("FontMap3d");
            return name;
        }

        RndFont3d *mFont; // 0x8
        int mDisplayableChars; // 0xc
        std::vector<RndMesh *> mMeshes; // 0x10
        std::vector<RndMesh *>::iterator mMeshItr; // 0x1c
    };

    // Hmx::Object
    virtual ~RndText();
    OBJ_CLASSNAME(Text);
    OBJ_SET_TYPE(Text);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void UpdateSphere();
    virtual float GetDistanceToPlane(const Plane &, Vector3 &);
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void Highlight();
    // RndText
    virtual Symbol TextToken() { return gNullStr; }

    OBJ_MEM_OVERLOAD(0x19);
    NEW_OBJ(RndText);

    String TextASCII() const;
    void SetTextASCII(const char *);
    void SetFixedLength(int);
    void ReFitTextScroll(String);
    void GetWidthHeightBox(Box &) const;
    float ComputeCharWidthsForText(String);
    void ConstructMeshes(const std::vector<RndText::Line> &, const Hmx::Rect &, float);

    static void Init();
    static void DrawBlacklight();
    static void ClearBlacklight();
    static void SetBlacklightModeEnabled(bool b) { sBlacklightModeEnabled = b; }
    static bool IsBlacklightModeEnabled() { return sBlacklightModeEnabled; }

    int GetTextSize() const { return Max<int>(mFixedLength, mText.length()); }
    void SetCapsMode(CapsMode c) { mCapsMode = c; }
    void UpdateText();
    void SetText(const char *);
    int FontMapIndex(RndFontBase *, bool);
    float ComputeHeight(int, float, float &);
    int NumStyles() const { return mStyles.size(); }
    float Width() const { return mWidth; }
    FitType GetFitType() const { return mFitType; }
    void SetFitType(FitType f) { mFitType = f; }
    void SetUnk78(Hmx::Object *o) { unk78 = o; };
    float DrawRectWidth() const { return mDrawRect.w; }
    ObjVector<Style> &Styles() { return mStyles; }
    const String &RawText() const { return mText; }
    float Indentation() const { return mIndentation; }

protected:
    RndText();

    void DoBasicMarkup();
    void BuildFontMaps(bool);
    void ClearFixedLength() {
        if (mFixedLength != 0) {
            mFixedLength = 0;
        }
    }
    const unsigned short *
    ParseMarkup(const unsigned short *, StyleState &, unsigned short &);
    void SizeCheck();
    void FitTextScroll();
    int ConvertTextToWide(const char *, std::vector<unsigned short> &);
    int OnComputeCharWidths(const unsigned short *, float *, bool);

    static void QueueBlacklightPacket(RndMesh *, float, int);
    static FontMapBase *AcquireFontMap(RndFontBase *);
    static void DrawMesh(RndMesh *, float, int);
    static bool sBlacklightModeEnabled;
    static int sBlacklightPacketCount;
    static std::vector<BlacklightPacket> sBlacklightPacketPool;
    static std::list<FontMapBase *> sFontMapCache;

    /** "Text value" */
    String mText; // 0x8
    /** "Width of text until it wraps." Ranges from 0 to 10000. */
    float mWidth; // 0x10
    /** "Height of the text, used for [fit_type] kFitJust". Ranges from 0 to 1000. */
    float mHeight; // 0x14
    /** "Lay text around circle of this circumference. Negative values face other way." */
    float mCircle; // 0x18
    /** "Alignment option for the text" */
    Alignment mAlignment; // 0x1c
    FitType mFitType; // 0x20
    /** "Defines the CAPS mode for the text" */
    CapsMode mCapsMode; // 0x24
    /** "Vertical distance between lines". Ranges from -5 to 5. */
    float mLeading; // 0x28
    /** "Number of character maximum for the text,
        if non-zero makes underlying mesh mutable, so updates are faster" */
    int mFixedLength; // 0x2c
    /** "Support markup or not.
        In the text, use <alt>, <alt2>, <alt3>, etc to use the higher styles,
        <sup> to get a super script, <nobreak> for preventing linebreaks in a block,
        <it> for italics, <gtr> for Bryn's guitar chord formatting.
        Example: Hit <it>Back</it> <alt>B</alt> to continue<sup>TM</sup> " */
    bool mMarkup; // 0x30
    /** "Support basic markup or not. It converts \\p to double-quotes.
        Furthur support can be added." */
    bool mBasicMarkup; // 0x31
    /** "If scrolling oversized text - delay this many seconds before starting" */
    float mScrollDelay; // 0x34
    /** "If scrolling oversized text - scroll this many characters per second" */
    float mScrollRate; // 0x38
    /** "If scrolling oversized text - delay this many seconds between scrolls.
        When the fit type is kFitScrollMarqueeWrapAlways, this value will be ignored." */
    float mScrollPause; // 0x3c
    bool unk40;
    float unk44;
    float unk48;
    int unk4c;
    float unk50;
    float unk54;
    float unk58;
    int unk5c;
    int unk60;
    /** "Space between continuous scrolling messages.
        This value is only considered when the fit type
        is set to kFitScrollMarqueeWrapAlways." */
    float mIndentation; // 0x64
    std::list<float> unk68;
    std::list<float> unk70;
    ObjPtr<Hmx::Object> unk78;
    float unk8c;
    int unk90;
    int unk94;
    /** "The different styles this text can have" */
    ObjVector<Style> mStyles; // 0x98
    std::vector<FontMapBase *> mFontMaps; // 0xa8
    Hmx::Rect mDrawRect; // 0xb4
    int unkc4; // 0xc4 - num lines?
    float unkc8;
};
