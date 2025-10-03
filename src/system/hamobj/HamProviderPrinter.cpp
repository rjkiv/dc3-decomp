#include "hamobj/HamProviderPrinter.h"
#include "flow/PropertyEventProvider.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"

HamProviderPrinter::HamProviderPrinter() { TheHamProvider->AddSink(this); }

DataNode HamProviderPrinter::OnMsg(const Message &msg) {
    if (DataVariable("hamprovider_print").Int()) {
        Symbol type = msg.Type();
        static Symbol downbeat("downbeat");
        static Symbol halfbeat("halfbeat");
        static Symbol quarterbeat("quarterbeat");
        static Symbol midi_snare("midi_snare");
        static Symbol midi_hat("midi_hat");
        static Symbol midi_kick("midi_kick");
        static Symbol midi_ride("midi_ride");
        static Symbol midi_open_hat("midi_open_hat");
        static Symbol midi_crash("midi_crash");
        static Symbol midi_tom_hi("midi_tom_hi");
        static Symbol midi_tom_low("midi_tom_low");
        static Symbol midi_tom_lo("midi_tom_lo");
        static Symbol midi_tom_mid("midi_tom_mid");
        static Symbol midi_hat_open("midi_hat_open");
        static Symbol on_beat_change("on_beat_change");
        if (type != downbeat && type != halfbeat && type != quarterbeat
            && type != midi_snare && type != midi_hat && type != midi_kick
            && type != midi_ride && type != midi_open_hat && type != midi_crash
            && type != midi_tom_hi && type != midi_tom_low && type != midi_tom_lo
            && type != midi_tom_mid && type != midi_hat_open && type != on_beat_change
            && *type.Str() != '\0') {
            MILO_LOG("HamProvider: %s\n", type);
        }
    }
    return DataNode(kDataUnhandled, 0);
}

BEGIN_HANDLERS(HamProviderPrinter)
    _HANDLE_CHECKED(OnMsg(Message(_msg)))
END_HANDLERS
