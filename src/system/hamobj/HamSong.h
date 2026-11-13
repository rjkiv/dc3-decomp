#pragma once
#include "beatmatch/HxMaster.h"
#include "beatmatch/HxSongData.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Song.h"
#include "utl/Symbol.h"

class HamSong : public Song {
public:
    // Hmx::Object
    OBJ_CLASSNAME(HamSong);
    OBJ_SET_TYPE(HamSong);

    static int mPreferStreaming;

    NEW_OBJ(HamSong)
    REGISTER_OBJ(HamSong)

private:
    // Song
    virtual void Unload();
    virtual void CreateSong(Symbol, DataArray *, HxSongData **, HxMaster **);
};
