#include "DebugGraph.h"
#include "rndobj/Graph.h"

void DebugGraph::AddData(float data, bool b) {
    Sample sample;
    sample.data = data;
    sample.b = b;
    mSamples.push_front(sample);

    if (mSamples.size() == unk38 + 1) {
        mSamples.pop_back();
    }
}

inline float clamp(float val) {
    float c = -val >= 0.0f ? 0.0f : val;
    return c - 1.0f >= 0.0f ? 1.0f : c;
}

void DebugGraph::Draw() {
    RndGraph *rnd = RndGraph::GetOneFrame();

    Hmx::Rect rect(mRect.x, mRect.y, mRect.w, mRect.h);
    rnd->AddRectFilled2D(rect, mColorB);

    if (unk50) {
        Vector2 minPos;
        minPos.x = mRect.x;
        minPos.y = mRect.y + mRect.h - 0.02f;
        rnd->AddScreenString(MakeString<float>("%.3f", unk3c), minPos, mColorA);

        Vector2 maxPos;
        maxPos.x = mRect.x;
        maxPos.y = mRect.y;
        rnd->AddScreenString(MakeString<float>("%.3f", unk40), maxPos, mColorA);
    }

    if (unk44 != FLT_MAX) {
        Vector2 p1;
        p1.x = mRect.x;
        p1.y = mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)));

        Vector2 p2;
        p2.x = mRect.x + mRect.w;
        p2.y = mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)));

        rnd->AddScreenLine(p1, p2, Hmx::Color(1.0f, 1.0f, 1.0f, 1.0f), false);

        Vector2 textPos;
        textPos.x = mRect.x;
        textPos.y = mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)));

        rnd->AddScreenString(
            MakeString<float>("%.3f", unk44), textPos, Hmx::Color(1.0f, 1.0f, 1.0f, 1.0f)
        );
    }

    Vector2 titlePos;
    titlePos.x = mRect.x + 0.1f;
    titlePos.y = mRect.y;
    rnd->AddScreenString(unk48.c_str(), titlePos, Hmx::Color(1.0f, 1.0f, 1.0f, 1.0f));

    if (!mSamples.empty()) {
        auto it = mSamples.begin();
        int idx = 1;
        Vector2 prevPos;
        prevPos.y =
            mRect.y + mRect.h * (1.0f - clamp((it->data - unk3c) / (unk40 - unk3c)));
        prevPos.x = mRect.x + mRect.w * (1.0f - clamp(0.0f / (float)(unk38 - 1)));

        ++it;

        for (; it != mSamples.end(); ++it) {
            Vector2 curPos;
            curPos.y =
                mRect.y + mRect.h * (1.0f - clamp((it->data - unk3c) / (unk40 - unk3c)));
            curPos.x =
                mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1)));

            if (it->b) {
                Vector2 hl_p1;
                hl_p1.y = mRect.y + mRect.h;

                Vector2 hl_p2;
                hl_p2.y = mRect.y;

                hl_p1.x =
                    mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1)));
                hl_p2.x =
                    mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1)));

                rnd->AddScreenLine(
                    hl_p2, hl_p1, Hmx::Color(1.0f, 1.0f, 1.0f, 1.0f), false
                );
            }
            rnd->AddScreenLine(curPos, prevPos, mColorA, false);
            prevPos = curPos;
            idx++;
        }
    }
}