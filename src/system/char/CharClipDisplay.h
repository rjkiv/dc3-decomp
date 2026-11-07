#pragma once
#include "char/CharClip.h"
#include "math/Color.h"
#include "obj/Dir.h"
#include "obj/Object.h"

struct CharClipDisplay {
public:
    CharClipDisplay()
        : mClip(0), mStart(0), mEnd(0), mLeft(0), mRight(0), mTextWidth(0), mTop(0),
          mBeat(0), mWeight(0), mIndent(0) {
        mText[0] = 0;
    }

    void SetText(const char *text);
    float GetX(float beat) const;
    void GetXY(Vector2 &xy, float beat) const;
    void SetStartEnd(float start, float end, bool center);
    void DrawBlend(float blendBeat, float blendWidth);
    void DrawBeatString(const char *s, float beat, const Hmx::Color &col);
    void DrawCursor();
    void SetClip(CharClip *clip, bool center);
    void DrawBeatString(float beat, const Hmx::Color &col);
    void DrawTrack();
    void SetBeat(float beat) { mBeat = beat; }
    void SetWeight(float weight) { mWeight = weight; }
    void SetTop(float top) { mTop = top; }
    void SetIndent(float indent) { mIndent = indent; }
    float GetY() const { return mTop; }

    /** "Zoom value for the highlight display" */
    static float sZoom;
    static float Em() { return sEm; }
    static float LineSpacing();
    static float TrackMargin();
    static void Init(ObjectDir *);
    static Hmx::Object *FindSource(Hmx::Object *obj);

protected:
    static float sEm;
    static ObjectDir *sDir;

    // protected: float LeftEdge() const;
    // protected: float RightEdge() const;
    // protected: float XToBeat(float) const;

    CharClip *mClip; // 0x0
    float mStart; // 0x4
    float mEnd; // 0x8
    float mLeft; // 0xc
    float mRight; // 0x10
    float mTextWidth; // 0x14
    float mTop; // 0x18
    float mBeat; // 0x1c
    float mWeight; // 0x20
    char mText[64]; // 0x24
    float mIndent; // 0x64
};
