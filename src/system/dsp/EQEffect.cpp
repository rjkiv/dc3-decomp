#include "dsp/EQEffect.h"
#include "dsp/mkfilter/filterdesign.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cstring>

void EQEffect::Process(float *samples, int sampct, int numChans) {
    if (mLRMode != 0.0f) {
        MILO_ASSERT(numChans <= 2, 120);
        if (numChans <= 0)
            return;
        for (int i = 0; i < sampct; i++) {
        }
        return;
    } else {
        MILO_ASSERT(numChans <= 2, 217);
        if (numChans <= 0)
            return;
        for (int i = 0; i < numChans; i++) {
        }
    }
}

void EQEffect::SetParameter(int idx, float val) {
    bool dirty29 = false, dirty30 = false, dirty28 = false, dirty27 = false,
         dirty26 = false, dirty25 = false;
    switch (idx) {
    case 0: {
        float old_hfc = mHighFreqCutoff;
        float new_hfc = Min(24000.f, val);
        new_hfc = Max(0.0f, new_hfc);
        if (new_hfc != old_hfc) {
            mHighFreqCutoff = new_hfc;
            dirty25 = true;
            dirty29 = true;
        }
    } break;
    case 1: {
        float old_hfg = mHighFreqGain;
        float new_hfg = Min(42.0f, val);
        new_hfg = Max(-42.0f, new_hfg);
        if (new_hfg != old_hfg) {
            mHighFreqGain = new_hfg;
            dirty29 = true;
        }
    } break;
    case 2: {
        float old_mfc = mMidFreqCutoff;
        float new_mfc = Min(24000.f, val);
        new_mfc = Max(0.0f, new_mfc);
        if (new_mfc != old_mfc) {
            mMidFreqCutoff = new_mfc;
            dirty25 = true;
            dirty30 = true;
        }
    } break;
    case 3: {
        float old_mfb = mMidFreqBandwidth;
        float new_mfb = Min(24000.f, val);
        new_mfb = Max(0.0f, new_mfb);
        if (new_mfb != old_mfb) {
            mMidFreqBandwidth = new_mfb;
            dirty30 = true;
        }
    } break;
    case 4: {
        float old_mfg = mMidFreqGain;
        float new_mfg = Min(42.0f, val);
        new_mfg = Max(-42.0f, new_mfg);
        if (new_mfg != old_mfg) {
            mMidFreqGain = new_mfg;
            dirty30 = true;
        }
    } break;
    case 5: {
        float old_lfc = mLowFreqCutoff;
        float new_lfc = Min(24000.f, val);
        new_lfc = Max(0.0f, new_lfc);
        if (new_lfc != old_lfc) {
            mLowFreqCutoff = new_lfc;
            dirty25 = true;
            dirty28 = true;
        }
    } break;
    case 6: {
        float old_lfg = mLowFreqGain;
        float new_lfg = Min(42.0f, val);
        new_lfg = Max(-42.0f, new_lfg);
        if (new_lfg != old_lfg) {
            mLowFreqGain = new_lfg;
            dirty28 = true;
        }
    } break;
    case 7: {
        float old_lpc = mLPFCutoff;
        float new_lpc = Min(20000.0f, val);
        new_lpc = Max(20.0f, new_lpc);
        if (new_lpc != old_lpc) {
            mLPFCutoff = new_lpc;
            dirty27 = true;
        }
    } break;
    case 8: {
        float old_lpr = mLPFReso;
        float new_lpr = Min(25.0f, val);
        new_lpr = Max(-25.0f, new_lpr);
        if (new_lpr != old_lpr) {
            mLPFReso = new_lpr;
            dirty27 = true;
        }
    } break;
    case 9: {
        float old_hpc = mHPFCutoff;
        float new_hpc = Min(20000.0f, val);
        new_hpc = Max(20.0f, new_hpc);
        if (new_hpc != old_hpc) {
            mHPFCutoff = new_hpc;
            dirty26 = true;
        }
    } break;
    case 10: {
        float old_hpr = mHPFReso;
        float new_hpr = Min(25.0f, val);
        new_hpr = Max(-25.0f, new_hpr);
        if (new_hpr != old_hpr) {
            mHPFReso = new_hpr;
            dirty26 = true;
        }
    } break;
    case 11: {
        mLRMode = bool(val > 0.5f);
    } break;
    case 12: {
        float newtranstime = Min(5000.0f, val);
        newtranstime = Max(25.0f, newtranstime);
        mTransitionTime = newtranstime;
        float new34;
        if (newtranstime != 0.0f) {
            new34 = powf(0.368000000, 1.0f / (48.0f * newtranstime));
        } else {
            new34 = 1.0f;
        }
        mTTk = new34;
    } break;
    default: {
        MILO_FAIL("bad parameter %i\n", idx);
    } break;
    }
    if (dirty29) { // recalculate high band
        mHF_k = tan(6.544985e-5f * mHighFreqCutoff);
        mHF_v0 = pow(10, mHighFreqGain / 20);
        mHF_h02 = (mHF_v0 - 1.0f) / 2;
        mComputeHF = mHF_h02TT != 0.0f || mHF_h02 != 0.0f;
        float f = mHighFreqGain > 0.0f ? mHF_k : mHF_k * mHF_v0;
        mHF_a = (f - 1.0f) / (f + 1.0f);
    } else if (dirty30) { // recalculate mid band
        mMF_k = tan(6.544985e-5f * mMidFreqBandwidth);
        mMF_v0 = pow(10, mMidFreqGain / 20);
        mMF_h02 = (mMF_v0 - 1.0f) / 2;
        mMF_d = -cosf(mMidFreqCutoff * 0.0001308997f);
        mComputeMF = mMF_h02TT != 0.0f || mMF_h02 != 0.0f;
        mMF_a = mMidFreqGain > 0.0f ? (mMF_k - 1.0f) / (mMF_k + 1.0f)
                             : (mMF_k - mMF_v0) / (mMF_k + mMF_v0);
        mMF_d *= 1.0f - mMF_a;
    } else if (dirty28) { // recalculate low band
        mLF_k = tan(6.544985e-5f * mLowFreqCutoff);
        mLF_v0 = pow(10, mLowFreqGain / 20);
        mLF_h02 = (mLF_h02TT - 1.0f) / 2;
        mComputeLF = mLF_h02TT != 0.0f || mLF_h02 != 0.0f;
        mLF_a = mLowFreqGain > 0.0f ? (mLF_k - 1.0f) / (mLF_k + 1.0f)
                             : (mLF_k - mLF_v0) / (mLF_k + mLF_v0);
    } else if (dirty27) { // recalculate low pass
        mComputeLPF = mLPFCutoff < 19999;
        float f = mLPFCutoff / 24000;
        float f2 = pow(10, -mLPFReso / 20);
        f *= float(PI);
        float f3 = sinf(f);
        f3 *= f2;
        f3 /= 2;
        float f13 = 1.0f - f3;
        f3 += 1.0f;
        f13 /= 2;
        float f30 = f13 / f3;
        float f31 = f30 + 0.5f;
        float f10 = cos(f);
        mLPF_B2 = f30 * 2;
        f10 *= f31;
        f31 -= f10;
        mLPF_B1 = f30 * -2;
        mLPF_A0 = f31 / 4;
        mLPF_A1 = mLPF_B1 * 4;
        mLPF_A2 = mLPF_A0;
    } else if (dirty26) { // recalculate high pass
        mComputeHPF = mLPFCutoff > 21;
        float f = mLPFCutoff / 24000;
        float f2 = pow(10, -mHPFReso / 20);
        f *= float(PI);
        float f3 = sinf(f);
        f3 *= f2;
        f3 /= 2;
        float f13 = 1.0f - f3;
        f3 += 1.0f;
        f13 /= 2;
        float f30 = f13 / f3;
        float f10 = cos(f);
        float f9 = f30 + 0.5f;
        mHPF_B2 = f30 * 2;
        f10 *= f9;
        f9 = f10 + f30;
        mLPF_B1 = f30 * -2;
        // mLPF_A0 = f31 / 4;
        mLPF_A1 = mLPF_B1 * 4;
        mLPF_A2 = mLPF_A0;
    }
    if (dirty25 && mLRMode != 0.0f) {
        FILTER f;
        createFilter(
            FilterType(1), FilterBand(0), 0, mLowFreqCutoff / 48000.0f, mLowFreqCutoff / 48000.0f, &f, 2
        );
        mLPGain = f.gain;
        if (f.numpoles > 0) {
            _blkmov(&mLRB, &f.ycoeffs, f.numpoles * sizeof(float));
        }
        createFilter(
            FilterType(1), FilterBand(2), 0, mLowFreqCutoff / 48000.0f, mHighFreqCutoff / 48000.0f, &f, 2
        );
        mBPGain = f.gain;
        if (f.numpoles > 0) {
            _blkmov(&mLRB, &f.ycoeffs, f.numpoles * sizeof(float));
        }
        createFilter(
            FilterType(1), FilterBand(1), 0, mHighFreqCutoff / 48000.0f, mHighFreqCutoff / 48000.0f, &f, 2
        );
        mHPGain = f.gain;
        if (f.numpoles > 0) {
            _blkmov(&mLRB, &f.ycoeffs, f.numpoles * sizeof(float));
        }
        Reset();
    }
}

EQEffect::EQEffect(IXAudioBatchAllocator *) {
    mComputeHF = false;
    mComputeMF = false;
    mComputeLF = false;
    mHighFreqCutoff = 12000.0f;
    mComputeLPF = false;
    mHighFreqGain = 0;
    mComputeHPF = false;
    mMidFreqCutoff = 8000.0f;
    mMidFreqBandwidth = 1000.0f;
    mMidFreqGain = 0;
    mLowFreqCutoff = 2000.0f;
    mLowFreqGain = 0;
    mLPFCutoff = 20000.0f;
    mLPFReso = 0;
    mHPFCutoff = 20.0f;
    mHPFReso = 0;
    mLRMode = 0;
    mTransitionTime = 25.0f;
    mHF_k = 0;
    mHF_v0 = 0;
    mHF_v0TT = 0;
    mHF_h02 = 0;
    mHF_h02TT = 0;
    mHF_a = 0;
    mMF_a = 0;
    mMF_k = 0;
    mMF_v0 = 0;
    mMF_v0TT = 0;
    mMF_h02 = 0;
    mMF_h02TT = 0;
    mMF_d = 0;
    mLF_k = 0;
    mLF_v0 = 0;
    mLF_v0TT = 0;
    mLF_h02 = 0;
    mLF_h02TT = 0;
    mLF_a = 0;
    mLPF_A0 = 0;
    mLPF_A1 = 0;
    mLPF_A2 = 0;
    mLPF_B1 = 0;
    mLPF_B2 = 0;
    mHPF_A0 = 0;
    mHPF_A1 = 0;
    mHPF_A2 = 0;
    mHPF_B1 = 0;
    mHPF_B2 = 0;
    Reset();
}

void EQEffect::SetParameters(const EQEffect::Params &params) {
    SetParameter(0, params.highFreqCutoff);
    SetParameter(1, params.highFreqGain);
    SetParameter(2, params.midFreqCutoff);
    SetParameter(3, params.midFreqBandwidth);
    SetParameter(4, params.midFreqGain);
    SetParameter(5, params.lowFreqCutoff);
    SetParameter(6, params.lowFreqGain);
    SetParameter(7, params.lowPassCutoff);
    SetParameter(8, params.lowPassReso);
    SetParameter(9, params.highPassCutoff);
    SetParameter(10, params.highPassReso);
    SetParameter(11, params.lrMode);
    SetParameter(12, params.transitionTime);
}

void EQEffect::Reset() {
    mMF_xn2[1] = mMF_xn1[1] = 0.0f;
    for (int i = 0; i < 2; i++) {
    }
    mHF_v0TT = mHF_v0;
    mMF_v0TT = mMF_v0;
    mLF_v0TT = mLF_v0;
    mHF_h02TT = mHF_h02;
    mMF_h02TT = mMF_h02;
    mLF_h02TT = mLF_h02;
    if (mTransitionTime != 0.0f) {
        mTTk = powf(0.368, 1.0f / (mTransitionTime * 48.0f));
    } else {
        mTTk = 1.0f;
    }
}
