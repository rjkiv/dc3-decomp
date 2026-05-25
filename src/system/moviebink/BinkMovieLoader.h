#pragma once
#include "utl/Loader.h"
#include "os/File.h"
#include "moviebink/BinkMovieImpl.h"

class BinkMovieLoader : public Loader {
    typedef void (BinkMovieLoader::*BinkMovieLoaderStateFunc)(void);

public:
    BinkMovieLoader(const FilePath &, LoaderPos, class BinkMovieImpl *);
    virtual ~BinkMovieLoader() { delete mFile; }
    virtual const char *DebugText() { return MakeString("ML: %s", LoaderFile().c_str()); }
    virtual bool IsLoaded() const { return mState == &BinkMovieLoader::DoneLoading; }
    virtual const char *StateName() const { return "MovieLoader"; }

protected:
    virtual void PollLoading() { (this->*mState)(); }

private:
    void OpenFile();
    void LoadFile() {
        MILO_ASSERT(mFile, 0x48);
        int bytes;
        if (mFile->ReadDone(bytes)) {
            if (!mFile->Fail()) {
                mImpl->DiscContentionCheck(this);
            }
            mState = &BinkMovieLoader::DoneLoading;
        }
    }
    void DoneLoading();

    File *mFile; // 0x1c
    BinkMovieLoaderStateFunc mState; // 0x20
    char mBuffer[0x20]; // 0x24
    class BinkMovieImpl *mImpl; // 0x44
};
