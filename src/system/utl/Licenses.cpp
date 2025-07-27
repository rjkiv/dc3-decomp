#include "Licenses.h"

Licenses *Licenses::sFront = nullptr;
uint Licenses::sInited = 0;

CDECL Licenses::Licenses(const char *cc, Licenses::Requirement req) {
    unk_0x4 = cc;
    unk_0x0 = req;
    Licenses *next = nullptr;
    if (sInited != 0xFEEDBACC) {
        next = nullptr;
        sInited = 0xFEEDBACC;
    } else {
        next = sFront;
    }
    mNext = next;
    sFront = this;
}

void Licenses::PrintAll(void) {}
