#include "char/CharClipDisplay.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"

float CharClipDisplay::sEm;
ObjectDir *CharClipDisplay::sDir;

void CharClipDisplay::Init(ObjectDir *dir) {
    sDir = dir;
    sEm = TheRnd.DrawString("", Vector2(0, 0), Hmx::Color(1.0f, 0.0f, 0.0f), false).y;
}

void CharClipDisplay::SetClip(CharClip *clip, bool b) {
    unk0 = clip;
    SetText(clip->Name());
    SetStartEnd(clip->StartBeat(), clip->EndBeat(), b);
}

void CharClipDisplay::SetText(const char *text) {
    strcpy(unk24, text);
    unk14 = TheRnd.DrawString(text, Vector2(0, 0), Hmx::Color(1.0f, 0.0f, 0.0f), false).x
        + sEm;
}

float CharClipDisplay::LineSpacing() { return sEm * 2.0f; }

float CharClipDisplay::GetX(float f) const {
    float f1 = unkc;
    float f2;
    if (unk10 > f1)
        f2 = unk10 - f1;

    float f3 = unk64 + unk14 * sEm * 3.0f;
    return ((TheRnd.Width() - sEm * 3.0f) - f3) * ((f - f1) / f2) + f3;
}

Hmx::Object *CharClipDisplay::FindSource(Hmx::Object *obj) {
    for (ObjDirItr<Hmx::Object> it(ObjectDir::Main(), false); it != nullptr; ++it) {
        if (it->Sinks() != nullptr && it->Sinks()->HasSink(obj)) {
            return it;
        }
    }
    return nullptr;
}

void CharClipDisplay::DrawBeatString(char const *c, float f1, Hmx::Color const &color) {
    // pulls stuff from xdk too
    TheRnd.DrawString(c, Vector2(GetX(f1), f1), color, true);
}