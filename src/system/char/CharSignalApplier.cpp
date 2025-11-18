#include "char/CharSignalApplier.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharSignalApplier::CharSignalApplier()
    : unk28(0), unk2c(-1.0f), unk30(1.0f), unk34(false), unk38(0.1f), unk3c(0),
      unk40(this) {}

BEGIN_PROPSYNCS(CharSignalApplier)
    // SYNC_PROP(bone_ops, unk40)
    SYNC_PROP(signal, unk2c)
    SYNC_PROP(do_smoothing, unk34)
    SYNC_PROP(smooth_increment, unk38)
    SYNC_PROP(signal_min, unk2c)
    SYNC_PROP(signal_max, unk30)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharSignalApplier)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << unk2c;
    bs << unk30;
    bs << unk34 << unk38;
END_SAVES

BEGIN_COPYS(CharSignalApplier)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY_AS(CharSignalApplier, c)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(unk28)
        COPY_MEMBER(unk2c)
        COPY_MEMBER(unk30)
        COPY_MEMBER(unk34)
        COPY_MEMBER(unk38)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharSignalApplier)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    bs >> unk2c >> unk30 >> unk34 >> unk38;
END_LOADS

void CharSignalApplier::Poll() {}

void CharSignalApplier::PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &) {
}
