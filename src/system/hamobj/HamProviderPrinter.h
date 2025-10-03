#pragma once
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"

class HamProviderPrinter : public Hmx::Object {
public:
    HamProviderPrinter();
    // Hmx::Object
    virtual ~HamProviderPrinter() {}
    virtual DataNode Handle(DataArray *, bool);

    DataNode OnMsg(const Message &);
};
