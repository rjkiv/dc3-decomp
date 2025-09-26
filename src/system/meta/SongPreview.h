#pragma once
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/VirtualKeyboard.h"
#include "synth/Faders.h"
#include "utl/Symbol.h"
#include "synth/Stream.h"

class SongPreview : public ContentMgr::Callback, public Hmx::Object{
    public:
        enum State {
            kIdle = 0,
            kMountingSong = 1,
            kPreparingSong = 2,
            kDeletingSong = 3,
            kPlayingSong = 4,
            kFadingOutSong = 5,
        };

        SongPreview(class SongMgr const &);
        virtual void ContentMounted(char const *, char const *);
        virtual void ContentFailed(char const *);
        //virtual ObjPtr<class TexMovie>::ObjPtr<class TexMovie>(void);
        virtual DataNode Handle(DataArray *, bool);
        //virtual void ObjRefConcrete<class TexMovie, class ObjectDir>::SetObj();
        virtual ~SongPreview();
        
        
        bool IsWaitingToDelete() const;
        bool IsFadingOut() const;
        void SetMusicVol(float);
        void Init();
        void Terminate();
        void Start(class Symbol, class TexMovie *);
        void PreparePreview();
        void Poll();
        DataNode OnStart(DataArray *);

    private:
        void DetachFader(Fader*);
        void PrepareFaders(class SongInfo const *);
        void PrepareSong(Symbol);
        //class Hmx::Object * ObjRefConcrete<class TexMovie, class ObjDir>::SetObj(class Hmx::Object*);
    
    const SongMgr &mSongMgr; //0x30
    int unk34;
    int unk44;
    Fader* mFader;
    float unk60;
    float unk68;
    bool unk4c;
    int unk50;
    int unk54;
    int unk58;
    int unk5c;
    int unk70;
    Symbol mSong;
    Symbol mSongContent;
    float unk7c;
    float unk80;
    float unk84;
    bool unk8c;
    float unk88;
    bool unk8d;
    bool unk8e;
    
    
};
