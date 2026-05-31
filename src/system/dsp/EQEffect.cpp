#include "dsp/EQEffect.h"
#include "dsp/mkfilter/filterdesign.h"
#include "math/Utl.h"
#include "os/Debug.h"
#include "xdk/xaudio2/xaudio2.h"
#include <cstring>

void EQEffect::SetParameter(int idx, float val) {
    bool dirty29 = false, dirty30 = false, dirty28 = false, dirty27 = false,
         dirty26 = false, dirty25 = false;
    switch (idx) {
    case 0: {
        float old_hfc = unk0;
        float new_hfc = Min(24000.f, val);
        new_hfc = Max(0.0f, new_hfc);
        if (new_hfc != old_hfc) {
            unk0 = new_hfc;
            dirty25 = true;
            dirty29 = true;
        }
    } break;
    case 1: {
        float old_hfg = unk4;
        float new_hfg = Min(42.0f, val);
        new_hfg = Max(-42.0f, new_hfg);
        if (new_hfg != old_hfg) {
            unk4 = new_hfg;
            dirty29 = true;
        }
    } break;
    case 2: {
        float old_mfc = unk8;
        float new_mfc = Min(24000.f, val);
        new_mfc = Max(0.0f, new_mfc);
        if (new_mfc != old_mfc) {
            unk8 = new_mfc;
            dirty25 = true;
            dirty30 = true;
        }
    } break;
    case 3: {
        float old_mfb = unkc;
        float new_mfb = Min(24000.f, val);
        new_mfb = Max(0.0f, new_mfb);
        if (new_mfb != old_mfb) {
            unkc = new_mfb;
            dirty30 = true;
        }
    } break;
    case 4: {
        float old_mfg = unk10;
        float new_mfg = Min(42.0f, val);
        new_mfg = Max(-42.0f, new_mfg);
        if (new_mfg != old_mfg) {
            unk10 = new_mfg;
            dirty30 = true;
        }
    } break;
    case 5: {
        float old_lfc = unk14;
        float new_lfc = Min(24000.f, val);
        new_lfc = Max(0.0f, new_lfc);
        if (new_lfc != old_lfc) {
            unk14 = new_lfc;
            dirty25 = true;
            dirty28 = true;
        }
    } break;
    case 6: {
        float old_lfg = unk18;
        float new_lfg = Min(42.0f, val);
        new_lfg = Max(-42.0f, new_lfg);
        if (new_lfg != old_lfg) {
            unk18 = new_lfg;
            dirty28 = true;
        }
    } break;
    case 7: {
        float old_lpc = unk1c;
        float new_lpc = Min(20000.0f, val);
        new_lpc = Max(20.0f, new_lpc);
        if (new_lpc != old_lpc) {
            unk1c = new_lpc;
            dirty27 = true;
        }
    } break;
    case 8: {
        float old_lpr = unk20;
        float new_lpr = Min(25.0f, val);
        new_lpr = Max(-25.0f, new_lpr);
        if (new_lpr != old_lpr) {
            unk20 = new_lpr;
            dirty27 = true;
        }
    } break;
    case 9: {
        float old_hpc = unk24;
        float new_hpc = Min(20000.0f, val);
        new_hpc = Max(20.0f, new_hpc);
        if (new_hpc != old_hpc) {
            unk24 = new_hpc;
            dirty26 = true;
        }
    } break;
    case 10: {
        float old_hpr = unk28;
        float new_hpr = Min(25.0f, val);
        new_hpr = Max(-25.0f, new_hpr);
        if (new_hpr != old_hpr) {
            unk28 = new_hpr;
            dirty26 = true;
        }
    } break;
    case 11: {
        unk2c = bool(val > 0.5f);
    } break;
    case 12: {
        float newtranstime = Min(5000.0f, val);
        newtranstime = Max(25.0f, newtranstime);
        unk30 = newtranstime;
        float new34;
        if (newtranstime != 0.0f) {
            new34 = powf(0.368000000, 1.0f / (48.0f * newtranstime));
        } else {
            new34 = 1.0f;
        }
        unk34 = new34;
    } break;
    default: {
        MILO_FAIL("bad parameter %i\n", idx);
    } break;
    }
    if (dirty29) { // recalculate high band
        unk3c = tan(6.544985e-5f * unk0);
        unk40 = pow(10, unk4 / 20);
        unk48 = (unk40 - 1.0f) / 2;
        unk38 = unk4c != 0.0f || unk48 != 0.0f;
        float f = unk4 > 0.0f ? unk3c : unk3c * unk40;
        unk50 = (f - 1.0f) / (f + 1.0f);
    } else if (dirty30) { // recalculate mid band
        unk5c = tan(6.544985e-5f * unkc);
        unk60 = pow(10, unk10 / 20);
        unk68 = (unk60 - 1.0f) / 2;
        unk70 = -cosf(unk8 * 0.0001308997f);
        unk54 = unk6c != 0.0f || unk68 != 0.0f;
        unk58 = unk10 > 0.0f ? (unk5c - 1.0f) / (unk5c + 1.0f)
                             : (unk5c - unk60) / (unk5c + unk60);
        unk70 *= 1.0f - unk58;
    } else if (dirty28) { // recalculate low band
        unk78 = tan(6.544985e-5f * unk14);
        unk7c = pow(10, unk18 / 20);
        unk84 = (unk88 - 1.0f) / 2;
        unk74 = unk88 != 0.0f || unk84 != 0.0f;
        unk8c = unk18 > 0.0f ? (unk78 - 1.0f) / (unk78 + 1.0f)
                             : (unk78 - unk7c) / (unk78 + unk7c);
    } else if (dirty27) { // recalculate low pass
        unk90 = unk1c < 19999;
        float f = unk1c / 24000;
        float f2 = pow(10, -unk20 / 20);
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
        unka4 = f30 * 2;
        f10 *= f31;
        f31 -= f10;
        unka0 = f30 * -2;
        unk94 = f31 / 4;
        unk98 = unka0 * 4;
        unk9c = unk94;
    } else if (dirty26) { // recalculate high pass
        unka8 = unk1c > 21;
        float f = unk1c / 24000;
        float f2 = pow(10, -unk28 / 20);
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
        unkbc = f30 * 2;
        f10 *= f9;
        f9 = f10 + f30;
        unka0 = f30 * -2;
        // unk94 = f31 / 4;
        unk98 = unka0 * 4;
        unk9c = unk94;
    }
    if (dirty25 && unk2c != 0.0f) {
        FILTER f;
        createFilter(
            FilterType(1), FilterBand(0), 0, unk14 / 48000.0f, unk14 / 48000.0f, &f, 2
        );
        unk_0x130 = f.unk_0x1000;
        if (f.unk_0x100C > 0) {
            _blkmov(&unk_0x15c, &f.pad[0x800], f.unk_0x100C * sizeof(float));
        }
        createFilter(
            FilterType(1), FilterBand(2), 0, unk14 / 48000.0f, unk0 / 48000.0f, &f, 2
        );
        unk_0x134 = f.unk_0x1000;
        if (f.unk_0x100C > 0) {
            _blkmov(&unk_0x15c, &f.pad[0x800], f.unk_0x100C * sizeof(float));
        }
        createFilter(
            FilterType(1), FilterBand(1), 0, unk0 / 48000.0f, unk0 / 48000.0f, &f, 2
        );
        unk_0x138 = f.unk_0x1000;
        if (f.unk_0x100C > 0) {
            _blkmov(&unk_0x15c, &f.pad[0x800], f.unk_0x100C * sizeof(float));
        }
        Reset();
    }
}

EQEffect::EQEffect(IXAudioBatchAllocator *) {
    unk38 = false;
    unk54 = false;
    unk74 = false;
    unk0 = 12000.0f;
    unk90 = false;
    unk4 = 0;
    unka8 = false;
    unk8 = 8000.0f;
    unkc = 1000.0f;
    unk10 = 0;
    unk14 = 2000.0f;
    unk18 = 0;
    unk1c = 20000.0f;
    unk20 = 0;
    unk24 = 20.0f;
    unk28 = 0;
    unk2c = 0;
    unk30 = 25.0f;
    unk3c = 0;
    unk40 = 0;
    unk44 = 0;
    unk48 = 0;
    unk4c = 0;
    unk50 = 0;
    unk58 = 0;
    unk5c = 0;
    unk60 = 0;
    unk64 = 0;
    unk68 = 0;
    unk6c = 0;
    unk70 = 0;
    unk78 = 0;
    unk7c = 0;
    unk80 = 0;
    unk84 = 0;
    unk88 = 0;
    unk8c = 0;
    unk94 = 0;
    unk98 = 0;
    unk9c = 0;
    unka0 = 0;
    unka4 = 0;
    unkac = 0;
    unkb0 = 0;
    unkb4 = 0;
    unkb8 = 0;
    unkbc = 0;
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
