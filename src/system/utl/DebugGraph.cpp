#include "DebugGraph.h"

void DebugGraph::AddData(float data, bool b)  {
    Sample sample;
    sample.data = data;
    sample.b = b;
    mSamples.push_front(sample);

    if (mSamples.size() == unk38 + 1) {
        mSamples.pop_back();
    }
}