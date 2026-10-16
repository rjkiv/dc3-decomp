#include "rndobj/Overlay.h"
#include "os/System.h"
#include "rndobj/Rnd.h"
#include "utl/Std.h"

bool RndOverlay::sTopAligned = true;
std::list<RndOverlay *> RndOverlay::sOverlays;

RndOverlay::RndOverlay(const DataArray *cfg)
    : mShowing(0), mLines(), mLine(), mBackColor(0.0f, 0.0f, 0.0f, 0.2f),
      mTextColor(1, 1, 1, 1), mCursorChar(-1), mCallback(0), mTimer(), mTimeout(0.0f),
      mModal(0), mDumpCount(0) {
    mName = cfg->Str(0);
    int lines = 1;
    cfg->FindData("lines", lines, false);
    SetLines(lines);
    cfg->FindData("showing", mShowing, false);
    cfg->FindData("color", mBackColor, false);
    cfg->FindData("modal", mModal, false);
    cfg->FindData("text_color", mTextColor, false);
}

void RndOverlay::Print(const char *cc) {
    for (const char *p = cc; *p != '\0'; p++) {
        if (mLine == mLines.end()) {
            String str;
            mLines.pop_front();
            mLines.push_back(str);
            mLine = PrevItr(mLines.end());
            mLine->reserve(127);
        }
        if (*p == '\n') {
            ++mLine;
        } else
            *mLine += *p;
    }
}

float RndOverlay::Height() const {
    float numLines = mLines.size();
    return TheRnd.DrawStringScreen("", Vector2(0, 0), mTextColor, true).y * numLines
        + 0.0268f;
}

void RndOverlay::Clear() {
    FOREACH (it, mLines) {
        it->erase();
    }
    mLine = mLines.begin();
    mCursorChar = -1;
}

static const float OFFSET_X = 0.025f;
static const float OFFSET_Y = 0.0134f;

float RndOverlay::Draw(float topY) {
    if (mTimeout > 0 && mShowing) {
        if (SystemMs() > mTimeout) {
            mShowing = false;
            mTimeout = 0;
        }
    }
    if (!mShowing) {
        return topY;
    } else if (mCallback) {
        float updated = mCallback->UpdateOverlay(this, topY);
        if (updated != topY) {
            return updated;
        }
    }
    Hmx::Rect rect(0, topY, 1, Height());
    TheRnd.DrawRectScreen(rect, mBackColor, TheRnd.OverlayMat(), nullptr, nullptr);
    Vector2 pos(OFFSET_X, OFFSET_Y + topY);
    if (mCursorChar > -1 && !mLines.empty()) {
        String str4c;
        for (int i = 0; i < mCursorChar; i++) {
            str4c += " ";
        }
        str4c += String("_");
        TheRnd.DrawStringScreen(
            str4c.c_str(), Vector2(pos.x, pos.y + 0.005f), mTextColor, true
        );
    }
    FOREACH (it, mLines) {
        pos.y = TheRnd.DrawStringScreen(it->c_str(), pos, mTextColor, true).y;
    }
    if (mDumpCount > 0) {
        mDumpCount--;
        FOREACH (it, mLines) {
            TheDebug << it->c_str() << "\n";
        }
    }
    return rect.y + rect.h;
}

void RndOverlay::SetLines(int lines) {
    MILO_ASSERT(lines >= 1, 0x72);
    if (mLines.size() != lines) {
        mLines.resize(lines);
        mLine = mLines.begin();
    }
}

String &RndOverlay::CurrentLine() {
    if (mLine == mLines.end()) {
        String newstr;
        mLines.pop_front();
        mLines.push_back(newstr);
        mLine = PrevItr(mLines.end());
        mLine->reserve(127);
    }
    return *mLine;
}

void RndOverlay::SetTimeout(float seconds) { mTimeout = seconds * 1000.0f + SystemMs(); }

RndOverlay *RndOverlay::Find(Symbol name, bool fail) {
    FOREACH (it, sOverlays) {
        if (name == (*it)->mName)
            return *it;
    }
    if (fail) {
        MILO_FAIL("Could not find overlay \"%s\"", name);
    }
    return nullptr;
}

void RndOverlay::TogglePosition() { sTopAligned = !sTopAligned; }

void RndOverlay::Init() {
    DataArray *cfg = SystemConfig("rnd");
    DataArray *overlaysArr = cfg->FindArray("overlays");
    for (int i = 1; i < overlaysArr->Size(); i++) {
        sOverlays.push_back(new RndOverlay(overlaysArr->Array(i)));
    }
}

void RndOverlay::Terminate() {
    for (std::list<RndOverlay *>::iterator i = sOverlays.begin(); i != sOverlays.end();) {
        delete *i;
        i = sOverlays.erase(i);
    }
}

void RndOverlay::DrawAll(bool b) {
    float toUse = sTopAligned ? 0.0212f : 0.9788f;
    FOREACH (it, sOverlays) {
        RndOverlay *cur = *it;
        if (!b || cur->mModal) {
            if (sTopAligned)
                toUse = cur->Draw(toUse);
            else if (cur->Showing()) {
                toUse -= cur->Height();
                cur->Draw(toUse);
            }
        }
    }
}
