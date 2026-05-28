#pragma once
#include "macros.h"
#include "xdk/XAUDIO2.h"

template <class T>
class StandardEffect : public ATG::CSampleXAPOBase<T, typename T::Params> {
public:
    StandardEffect() : CSampleXAPOBase(), unk_bool(false) {
        mEffect = new T(nullptr);
        T::Params params;
        params.unk0 = 0;
        SetParameters(&params, sizeof(T::Params));
    }
    virtual ~StandardEffect() { RELEASE(mEffect); }
    virtual void OnSetParameters(const typename T::Params &params) {
        mEffect->SetParameters(params);
    }
    virtual void DoProcess(
        const typename T::Params &params,
        float *__restrict buffer,
        unsigned int ui3,
        unsigned int numChans
    ) {
        if (!params.unk0) {
            mEffect->Process(buffer, ui3, numChans);
        } else if (!unk_bool) {
            mEffect->Reset();
        }
        unk_bool = params.unk0;
    }

private:
    T *mEffect;
    bool unk_bool;
};
