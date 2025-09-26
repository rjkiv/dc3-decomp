#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/Stream.h"

class StreamPlayer : public Hmx::Object{
    public:
        StreamPlayer();
        void StopPlaying();
        virtual ~StreamPlayer();
        void PlayFile(char const *, float, float, bool);
        void Poll();
        virtual DataNode Handle(DataArray *, bool);

        float mMasterVol; //0x2c
        float mStreamVol; //0x30
        bool mLoop; //0x34
        bool mStarted; //0x35
        bool mPaused; //0x36
        Stream *mStream; //0x38
        

    private:
        void Delete();
        void Init();
};
