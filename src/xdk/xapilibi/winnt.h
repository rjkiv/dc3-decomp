#pragma once
#include "../win_types.h"
#include "minwinbase.h"
#include "wtypesbase.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STATUS_WAIT_0 0x00000000L
#define LANG_NEUTRAL 0x00

typedef struct _CONTEXT { /* Size=0xa40 */
    /* 0x0000 */ DWORD ContextFlags;
    /* 0x0004 */ DWORD Msr;
    /* 0x0008 */ DWORD Iar;
    /* 0x000c */ DWORD Lr;
    /* 0x0010 */ ULARGE_INTEGER Ctr;
    /* 0x0018 */ ULARGE_INTEGER Gpr0;
    /* 0x0020 */ ULARGE_INTEGER Gpr1;
    /* 0x0028 */ ULARGE_INTEGER Gpr2;
    /* 0x0030 */ ULARGE_INTEGER Gpr3;
    /* 0x0038 */ ULARGE_INTEGER Gpr4;
    /* 0x0040 */ ULARGE_INTEGER Gpr5;
    /* 0x0048 */ ULARGE_INTEGER Gpr6;
    /* 0x0050 */ ULARGE_INTEGER Gpr7;
    /* 0x0058 */ ULARGE_INTEGER Gpr8;
    /* 0x0060 */ ULARGE_INTEGER Gpr9;
    /* 0x0068 */ ULARGE_INTEGER Gpr10;
    /* 0x0070 */ ULARGE_INTEGER Gpr11;
    /* 0x0078 */ ULARGE_INTEGER Gpr12;
    /* 0x0080 */ ULARGE_INTEGER Gpr13;
    /* 0x0088 */ ULARGE_INTEGER Gpr14;
    /* 0x0090 */ ULARGE_INTEGER Gpr15;
    /* 0x0098 */ ULARGE_INTEGER Gpr16;
    /* 0x00a0 */ ULARGE_INTEGER Gpr17;
    /* 0x00a8 */ ULARGE_INTEGER Gpr18;
    /* 0x00b0 */ ULARGE_INTEGER Gpr19;
    /* 0x00b8 */ ULARGE_INTEGER Gpr20;
    /* 0x00c0 */ ULARGE_INTEGER Gpr21;
    /* 0x00c8 */ ULARGE_INTEGER Gpr22;
    /* 0x00d0 */ ULARGE_INTEGER Gpr23;
    /* 0x00d8 */ ULARGE_INTEGER Gpr24;
    /* 0x00e0 */ ULARGE_INTEGER Gpr25;
    /* 0x00e8 */ ULARGE_INTEGER Gpr26;
    /* 0x00f0 */ ULARGE_INTEGER Gpr27;
    /* 0x00f8 */ ULARGE_INTEGER Gpr28;
    /* 0x0100 */ ULARGE_INTEGER Gpr29;
    /* 0x0108 */ ULARGE_INTEGER Gpr30;
    /* 0x0110 */ ULARGE_INTEGER Gpr31;
    /* 0x0118 */ DWORD Cr;
    /* 0x011c */ DWORD Xer;
    /* 0x0120 */ double Fpscr;
    /* 0x0128 */ double Fpr0;
    /* 0x0130 */ double Fpr1;
    /* 0x0138 */ double Fpr2;
    /* 0x0140 */ double Fpr3;
    /* 0x0148 */ double Fpr4;
    /* 0x0150 */ double Fpr5;
    /* 0x0158 */ double Fpr6;
    /* 0x0160 */ double Fpr7;
    /* 0x0168 */ double Fpr8;
    /* 0x0170 */ double Fpr9;
    /* 0x0178 */ double Fpr10;
    /* 0x0180 */ double Fpr11;
    /* 0x0188 */ double Fpr12;
    /* 0x0190 */ double Fpr13;
    /* 0x0198 */ double Fpr14;
    /* 0x01a0 */ double Fpr15;
    /* 0x01a8 */ double Fpr16;
    /* 0x01b0 */ double Fpr17;
    /* 0x01b8 */ double Fpr18;
    /* 0x01c0 */ double Fpr19;
    /* 0x01c8 */ double Fpr20;
    /* 0x01d0 */ double Fpr21;
    /* 0x01d8 */ double Fpr22;
    /* 0x01e0 */ double Fpr23;
    /* 0x01e8 */ double Fpr24;
    /* 0x01f0 */ double Fpr25;
    /* 0x01f8 */ double Fpr26;
    /* 0x0200 */ double Fpr27;
    /* 0x0208 */ double Fpr28;
    /* 0x0210 */ double Fpr29;
    /* 0x0218 */ double Fpr30;
    /* 0x0220 */ double Fpr31;
    /* 0x0228 */ DWORD UserModeControl;
    /* 0x022c */ DWORD Fill;
    /* 0x0230 */ float Vscr[4];
    /* 0x0240 */ float Vr0[4];
    /* 0x0250 */ float Vr1[4];
    /* 0x0260 */ float Vr2[4];
    /* 0x0270 */ float Vr3[4];
    /* 0x0280 */ float Vr4[4];
    /* 0x0290 */ float Vr5[4];
    /* 0x02a0 */ float Vr6[4];
    /* 0x02b0 */ float Vr7[4];
    /* 0x02c0 */ float Vr8[4];
    /* 0x02d0 */ float Vr9[4];
    /* 0x02e0 */ float Vr10[4];
    /* 0x02f0 */ float Vr11[4];
    /* 0x0300 */ float Vr12[4];
    /* 0x0310 */ float Vr13[4];
    /* 0x0320 */ float Vr14[4];
    /* 0x0330 */ float Vr15[4];
    /* 0x0340 */ float Vr16[4];
    /* 0x0350 */ float Vr17[4];
    /* 0x0360 */ float Vr18[4];
    /* 0x0370 */ float Vr19[4];
    /* 0x0380 */ float Vr20[4];
    /* 0x0390 */ float Vr21[4];
    /* 0x03a0 */ float Vr22[4];
    /* 0x03b0 */ float Vr23[4];
    /* 0x03c0 */ float Vr24[4];
    /* 0x03d0 */ float Vr25[4];
    /* 0x03e0 */ float Vr26[4];
    /* 0x03f0 */ float Vr27[4];
    /* 0x0400 */ float Vr28[4];
    /* 0x0410 */ float Vr29[4];
    /* 0x0420 */ float Vr30[4];
    /* 0x0430 */ float Vr31[4];
    /* 0x0440 */ float Vr32[4];
    /* 0x0450 */ float Vr33[4];
    /* 0x0460 */ float Vr34[4];
    /* 0x0470 */ float Vr35[4];
    /* 0x0480 */ float Vr36[4];
    /* 0x0490 */ float Vr37[4];
    /* 0x04a0 */ float Vr38[4];
    /* 0x04b0 */ float Vr39[4];
    /* 0x04c0 */ float Vr40[4];
    /* 0x04d0 */ float Vr41[4];
    /* 0x04e0 */ float Vr42[4];
    /* 0x04f0 */ float Vr43[4];
    /* 0x0500 */ float Vr44[4];
    /* 0x0510 */ float Vr45[4];
    /* 0x0520 */ float Vr46[4];
    /* 0x0530 */ float Vr47[4];
    /* 0x0540 */ float Vr48[4];
    /* 0x0550 */ float Vr49[4];
    /* 0x0560 */ float Vr50[4];
    /* 0x0570 */ float Vr51[4];
    /* 0x0580 */ float Vr52[4];
    /* 0x0590 */ float Vr53[4];
    /* 0x05a0 */ float Vr54[4];
    /* 0x05b0 */ float Vr55[4];
    /* 0x05c0 */ float Vr56[4];
    /* 0x05d0 */ float Vr57[4];
    /* 0x05e0 */ float Vr58[4];
    /* 0x05f0 */ float Vr59[4];
    /* 0x0600 */ float Vr60[4];
    /* 0x0610 */ float Vr61[4];
    /* 0x0620 */ float Vr62[4];
    /* 0x0630 */ float Vr63[4];
    /* 0x0640 */ float Vr64[4];
    /* 0x0650 */ float Vr65[4];
    /* 0x0660 */ float Vr66[4];
    /* 0x0670 */ float Vr67[4];
    /* 0x0680 */ float Vr68[4];
    /* 0x0690 */ float Vr69[4];
    /* 0x06a0 */ float Vr70[4];
    /* 0x06b0 */ float Vr71[4];
    /* 0x06c0 */ float Vr72[4];
    /* 0x06d0 */ float Vr73[4];
    /* 0x06e0 */ float Vr74[4];
    /* 0x06f0 */ float Vr75[4];
    /* 0x0700 */ float Vr76[4];
    /* 0x0710 */ float Vr77[4];
    /* 0x0720 */ float Vr78[4];
    /* 0x0730 */ float Vr79[4];
    /* 0x0740 */ float Vr80[4];
    /* 0x0750 */ float Vr81[4];
    /* 0x0760 */ float Vr82[4];
    /* 0x0770 */ float Vr83[4];
    /* 0x0780 */ float Vr84[4];
    /* 0x0790 */ float Vr85[4];
    /* 0x07a0 */ float Vr86[4];
    /* 0x07b0 */ float Vr87[4];
    /* 0x07c0 */ float Vr88[4];
    /* 0x07d0 */ float Vr89[4];
    /* 0x07e0 */ float Vr90[4];
    /* 0x07f0 */ float Vr91[4];
    /* 0x0800 */ float Vr92[4];
    /* 0x0810 */ float Vr93[4];
    /* 0x0820 */ float Vr94[4];
    /* 0x0830 */ float Vr95[4];
    /* 0x0840 */ float Vr96[4];
    /* 0x0850 */ float Vr97[4];
    /* 0x0860 */ float Vr98[4];
    /* 0x0870 */ float Vr99[4];
    /* 0x0880 */ float Vr100[4];
    /* 0x0890 */ float Vr101[4];
    /* 0x08a0 */ float Vr102[4];
    /* 0x08b0 */ float Vr103[4];
    /* 0x08c0 */ float Vr104[4];
    /* 0x08d0 */ float Vr105[4];
    /* 0x08e0 */ float Vr106[4];
    /* 0x08f0 */ float Vr107[4];
    /* 0x0900 */ float Vr108[4];
    /* 0x0910 */ float Vr109[4];
    /* 0x0920 */ float Vr110[4];
    /* 0x0930 */ float Vr111[4];
    /* 0x0940 */ float Vr112[4];
    /* 0x0950 */ float Vr113[4];
    /* 0x0960 */ float Vr114[4];
    /* 0x0970 */ float Vr115[4];
    /* 0x0980 */ float Vr116[4];
    /* 0x0990 */ float Vr117[4];
    /* 0x09a0 */ float Vr118[4];
    /* 0x09b0 */ float Vr119[4];
    /* 0x09c0 */ float Vr120[4];
    /* 0x09d0 */ float Vr121[4];
    /* 0x09e0 */ float Vr122[4];
    /* 0x09f0 */ float Vr123[4];
    /* 0x0a00 */ float Vr124[4];
    /* 0x0a10 */ float Vr125[4];
    /* 0x0a20 */ float Vr126[4];
    /* 0x0a30 */ float Vr127[4];
} CONTEXT, *PCONTEXT;

#define EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef LONG TOP_LEVEL_EXCEPTION_FILTER(EXCEPTION_POINTERS *);
typedef TOP_LEVEL_EXCEPTION_FILTER *LPTOP_LEVEL_EXCEPTION_FILTER;

#ifdef __cplusplus
}
#endif
