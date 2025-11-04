#include "meta/StoreEnumeration.h"

XboxEnumeration::XboxEnumeration(int i, std::vector<unsigned long long> *mOfferIDCount)
    : unk18(i), unk1c(false) {}

XboxEnumeration::~XboxEnumeration() {}

bool XboxEnumeration::IsSuccess() const { return false; }

void XboxEnumeration::Start() {}

void XboxEnumeration::Poll() {}
