#include "Licenses.h"
#include "os/Debug.h"

Licenses *Licenses::sFront = nullptr;
int Licenses::sInited = 0;

Licenses::Licenses(const char *cc, Licenses::Requirement req) {
    mName = cc;
    unk_0x0 = req;
    Licenses *next;
    int magic = 0xFEEDBACC;
    if (sInited != magic) {
        sInited = magic;
        next = nullptr;
    } else {
        next = sFront;
    }
    mNext = next;
    sFront = this;
}

void Licenses::PrintAll(void) {
    MILO_LOG("List of all Licenses:\n");
    Licenses *head = sFront;
    while (head != nullptr) {
        MILO_LOG(
            "   %s requirement: %s\n",
            head->mName,
            head->unk_0x0 == 0 ? "Notification" : "Do Not Distribute"
        );
        head = head->mNext;
    }
}
