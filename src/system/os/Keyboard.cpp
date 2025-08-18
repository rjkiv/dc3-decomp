#include "os/Keyboard.h"
#include "obj/Object.h"
#include "obj/Object.h"

Hmx::Object *gSource;
ObjPtr<Hmx::Object> gOverride(0, 0);

void KeyboardTerminateCommon() { RELEASE(gSource); }

void KeyboardSubscribe(Hmx::Object *o) {
    if (gSource) {
        gSource->AddSink(o);
    }
}

void KeyboardUnsubscribe(Hmx::Object *o) {
    if (gSource) {
        gSource->RemoveSink(o);
    }
}

void KeyboardInitCommon() {
    MILO_ASSERT(!gSource, 0x12);
    gSource = Hmx::Object::New<Hmx::Object>();
}

Hmx::Object *KeyboardOverride(Hmx::Object *o) {
    Hmx::Object *d = gOverride;
    gOverride = o;
    return d;
}

void KeyboardSendMsg(int key, bool shift, bool ctrl, bool alt) {
    KeyboardKeyMsg msg(key, shift, ctrl, alt);
    if (gOverride) {
        gOverride->Handle(msg, false);
    } else {
        gSource->Handle(msg, false);
    }
}
