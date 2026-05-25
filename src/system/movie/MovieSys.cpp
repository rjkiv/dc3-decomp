#include "movie/MovieSys.h"
#include "MovieSys.h"
#include "TexMovie.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

MovieSys::MovieSys() : mInitialized(false) {}

MovieSys::~MovieSys() {}

void MovieSys::Init() {
    if (mInitialized == false) {
        mInitialized = true;
        TexMovie::Init();
        TheDebug.AddExitCallback(Movie::Terminate);
    }
}

void MovieSys::Terminate() {
    if (mInitialized == false) {
        return;
    }
    mInitialized = false;
}

MovieImpl *MovieSys::CreateMovieImpl() { return new MovieImpl(); }
