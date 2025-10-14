#include "movie/MovieSys.h"
#include "MovieImpl.h"
#include "MovieSys.h"
#include "TexMovie.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

MovieSys::MovieSys() : isInitalized(false) {}

MovieSys::~MovieSys() {}

void MovieSys::Init() {
    if (isInitalized == false) {
        isInitalized = true;
        TexMovie::Init();
        TheDebug.AddExitCallback(Movie::Terminate);
    }
}

void MovieSys::Terminate() {
    if (isInitalized == false) {
        return;
    }
    isInitalized = false;
}

MovieImpl *MovieSys::CreateMovieImpl() { return new MovieImpl(); }
