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
        Vector2 minPos(mRect.x, mRect.y + mRect.h - 0.02f);
        rnd->AddScreenString(MakeString("%.3f", unk3c), minPos, mColorA);

        Vector2 maxPos(mRect.x, mRect.y);
        rnd->AddScreenString(MakeString("%.3f", unk40), maxPos, mColorA);
    }

    if (unk44 != FLT_MAX) {
        Hmx::Color color(1.0f, 1.0f, 1.0f, 1.0f);

        Vector2 p2(
            mRect.w + mRect.x,
            mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)))
        );

        Vector2 p1(
            mRect.x, mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)))
        );

        rnd->AddScreenLine(p1, p2, color, false);

        Vector2 textPos(
            mRect.x, mRect.y + mRect.h * (1.0f - clamp((unk44 - unk3c) / (unk40 - unk3c)))
        );

        rnd->AddScreenString(
            MakeString("%.3f", unk44), textPos, Hmx::Color(1.0f, 1.0f, 1.0f, 1.0f)
        );
    }

    Vector2 titlePos;

    titlePos.y = mRect.y;
    Hmx::Color color(1.0f, 1.0f, 1.0f, 1.0f);
    titlePos.x = mRect.x + 0.1f;

    rnd->AddScreenString(unk48.c_str(), titlePos, color);

    if (!mSamples.empty()) {
        auto it = mSamples.begin();
        int idx = 1;
        Vector2 prevPos(
            mRect.x + mRect.w * (1.0f - clamp(0.0f / (float)(unk38 - 1))),
            mRect.y + mRect.h * (1.0f - clamp((it->data - unk3c) / (unk40 - unk3c)))
        );

        ++it;

        for (; it != mSamples.end(); ++it) {
            Vector2 curPos(
                mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1))),
                mRect.y + mRect.h * (1.0f - clamp((it->data - unk3c) / (unk40 - unk3c)))
            );

            if (it->b) {
                Vector2 hl_p1(
                    mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1))),
                    mRect.y + mRect.h
                );

                Vector2 hl_p2(
                    mRect.x + mRect.w * (1.0f - clamp((float)idx / (float)(unk38 - 1))),
                    mRect.y
                );

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