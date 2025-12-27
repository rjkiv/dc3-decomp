#include "hamobj/HamSong.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamSongData.h"
#include "meta/DataArraySongInfo.h"
#include "midi/MidiParserMgr.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Object.h"

void HamSong::Unload() {
    Song::Unload();
    RELEASE(TheMidiParserMgr);
}

void HamSong::CreateSong(Symbol s, DataArray *a, HxSongData **sd, HxMaster **m) {
    HamSongData *hsd = new HamSongData();
    *sd = hsd;
    DataMacroWarning(false);
    MidiParserMgr *mgr = new MidiParserMgr(nullptr, "noname");
    HamMaster *master = new HamMaster(hsd, TheMidiParserMgr);
    *m = master;
    TheMaster = master;
    TheMaster->SetName("master", ObjectDir::Main());
    TheMaster->GetAudio()->SetName("audio", ObjectDir::Main());
    DataMacroWarning(true);
    DataArraySongInfo info(a, nullptr, s);
    bool streamBool;
    if (mPreferStreaming) {
        streamBool = true;
    } else {
        streamBool = gMiloTool != 0;
    }
    master->Load(&info, streamBool, 0, true, (HamSongDataValidate)0, nullptr);
}
