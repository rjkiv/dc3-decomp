#include "FxSendReverb.h"
#include "FxSend.h"
#include "xdk/XAUDIO2.h"

FxSendReverb360::FxSendReverb360() : FxSend360(this) {}

FxSendReverb360::~FxSendReverb360() {}

struct PresetConfig {
    Symbol name;
    XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset;
};

void FxSendReverb360::SyncEffectParams(IXAudio2SubmixVoice *voice) const {
    static PresetConfig sConfigs[] = {
        { "default", XAUDIO2FX_I3DL2_PRESET_DEFAULT },
        { "generic", XAUDIO2FX_I3DL2_PRESET_GENERIC },
        { "padded_cell", XAUDIO2FX_I3DL2_PRESET_PADDEDCELL },
        { "room", XAUDIO2FX_I3DL2_PRESET_ROOM },
        { "bath_room", XAUDIO2FX_I3DL2_PRESET_BATHROOM },
        { "living_room", XAUDIO2FX_I3DL2_PRESET_LIVINGROOM },
        { "stone_room", XAUDIO2FX_I3DL2_PRESET_STONEROOM },
        { "auditorium", XAUDIO2FX_I3DL2_PRESET_AUDITORIUM },
        { "concert_hall", XAUDIO2FX_I3DL2_PRESET_CONCERTHALL },
        { "cave", XAUDIO2FX_I3DL2_PRESET_CAVE },
        { "arena", XAUDIO2FX_I3DL2_PRESET_ARENA },
        { "hangar", XAUDIO2FX_I3DL2_PRESET_HANGAR },
        { "carpeted_hallway", XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY },
        { "hallway", XAUDIO2FX_I3DL2_PRESET_HALLWAY },
        { "stone_corridor", XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR },
        { "alley", XAUDIO2FX_I3DL2_PRESET_ALLEY },
        { "forest", XAUDIO2FX_I3DL2_PRESET_FOREST },
        { "city", XAUDIO2FX_I3DL2_PRESET_CITY },
        { "mountains", XAUDIO2FX_I3DL2_PRESET_MOUNTAINS },
        { "quarry", XAUDIO2FX_I3DL2_PRESET_QUARRY },
        { "plain", XAUDIO2FX_I3DL2_PRESET_PLAIN },
        { "parking_lot", XAUDIO2FX_I3DL2_PRESET_PARKINGLOT },
        { "sewer_pipe", XAUDIO2FX_I3DL2_PRESET_SEWERPIPE },
        { "underwater", XAUDIO2FX_I3DL2_PRESET_UNDERWATER },
        { "small_room", XAUDIO2FX_I3DL2_PRESET_SMALLROOM },
        { "medium_room", XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM },
        { "large_room", XAUDIO2FX_I3DL2_PRESET_LARGEROOM },
        { "medium_hall", XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL },
        { "large_hall", XAUDIO2FX_I3DL2_PRESET_LARGEHALL },
        { "plate", XAUDIO2FX_I3DL2_PRESET_PLATE }
    };
    int idx = 0;
    for (; idx < DIM(sConfigs); idx++) {
        if (sConfigs[idx].name == mEnvironmentPreset) {
            break;
        }
    }
    if (idx == DIM(sConfigs)) {
        MILO_FAIL("Unexpected environment preset.");
    }
    XAUDIO2FX_REVERB_PARAMETERS native;
    ReverbConvertI3DL2ToNative(&sConfigs[idx].preset, &native);
    voice->SetEffectParameters(0, &native, sizeof(native), 0);
}

IUnknown *FxSendReverb360::CreateFx() {
    IUnknown *fx;
    XAudio2CreateReverb(&fx);
    return fx;
}
