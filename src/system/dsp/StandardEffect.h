#pragma once
#include "macros.h"
#include "xdk/XAUDIO2.h"

template <class T>
class StandardEffect : public ATG::CSampleXAPOBase<T, typename T::Params> {
public:
    StandardEffect() : CSampleXAPOBase(), mBypass(false) {
        mEffect = new T(nullptr);
        T::Params params;
        params.bypass = false;
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
        if (!params.bypass) {
            mEffect->Process(buffer, ui3, numChans);
        } else if (!mBypass) {
            mEffect->Reset();
        }
        mBypass = params.bypass;
    }

private:
    T *mEffect;
    bool mBypass;
};
