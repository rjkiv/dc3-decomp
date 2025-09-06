#pragma once
#include "math/Color.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/FontBase.h"
#include "rndobj/Font.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"
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
        /** "Shrinks the text until it fits within [width] and [height].
            Note that this is a very expensive process, super slow,
            and so should never be used on dynamically changing text when in game" */
        kFitJust = 1,
        /** "Constrains the text to one line of [width] with ellipses" */
        kFitEllipsis = 2,
        /** "Continuous right-to-left scrolling. String start follows sring end" */
        kFitScrollMarqueeWrap = 3,
        /** "Right-to-left scroll - Reset to beginning after end scrolls off" */
        kFitScrollMarqueeReset = 4,
        /** "Reverse scroll direction whenever string end or beginning is reached" */
        kFitScrollPingPong = 5,
        /** "Continuous right-to-left scroll with wrapping and not care about string size.
            '\n' will be replaced with indentation." */
        kFitScrollMarqueeWrapAlways = 6
    };

    class Style {
    public:
        Style(Hmx::Object *);
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
        /** "Font to use for this style" */
        ObjPtr<RndFontBase> mFont; // 0x34
        /** "draw in blacklight pass?" */
        bool mBlacklight; // 0x48
    };

    class StyleState {
    public:
    };

    class FontMapBase {
    public:
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
            const StyleState &,
            unsigned short,
            float,
            FitType,
            float
        ) = 0;
        virtual bool SupportsScrolling() const = 0;
        virtual void SetupScrolling() = 0;
        virtual void UpdateScrolling(float) = 0;

        MEM_OVERLOAD(FontMapBase, 0xD7);
    };

    // size 0x18
    class FontMap : public FontMapBase {
    public:
        // size 0x10
        class Page {};

        // fill out virtual stuff

        bool unk4; // 0x4
        RndFont *mFont; // 0x8
        std::vector<Page *> mPages; // 0xc
    };

    // size 0x20
    class FontMap3d : public FontMapBase {};

    // Hmx::Object
    virtual ~RndText() {}
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
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndText
    virtual Symbol TextToken() { return gNullStr; }

    OBJ_MEM_OVERLOAD(0x19);
    NEW_OBJ(RndText);

    String TextASCII() const;
    void SetTextASCII(const char *);
    void SetFixedLength(int);

protected:
    RndText();

    /** "Text value" */
    String mText; // 0x8
    /** "Width of text until it wraps." Ranges from 0 to 10000. */
    float mWidth; // 0x10
    /** "Height of the text, used for [fit_type] kFitJust". Ranges from 0 to 1000. */
    float mHeight; // 0x14
    /** "Lay text around circle of this circumference. Negative values face other way." */
    float mCircle; // 0x18
    /** "Alignment option for the text" */
    Alignment mAlign; // 0x1c
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
    int unk44;
    int unk48;
    int unk4c;
    int unk50;
    int unk54;
    float unk58;
    float unk5c;
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
    float unkb4;
    float unkb8;
    float unkbc;
    float unkc0;
    int unkc4;
    float unkc8;
};
