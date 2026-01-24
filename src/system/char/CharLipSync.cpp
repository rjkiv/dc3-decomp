#include "char/CharLipSync.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "utl/TextStream.h"

std::map<Symbol, CharLipSync *> *CharLipSync::sLipSyncMap;

CharLipSync::CharLipSync() : mFrames(0) {}
CharLipSync::~CharLipSync() { UnregisterLipSync(this); }

BEGIN_HANDLERS(CharLipSync)
    HANDLE(parse, OnParse)
    HANDLE(parse_array, OnParseArray)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharLipSync)
    {
        static Symbol _s("frames");
        if (sym == _s && (_op & kPropGet))
            return PropSync(mFrames, _val, _prop, _i + 1, _op);
    }
    SYNC_PROP_SET(duration, Duration(), )

    {
        static Symbol _s("visemes");
        if (sym == _s && (_op & (kPropGet | kPropSize)))
            return PropSync(mVisemes, _val, _prop, _i + 1, _op);
    }
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharLipSync)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mVisemes;
    bs << mFrames;
    bs << mData;
END_SAVES

BEGIN_COPYS(CharLipSync)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharLipSync)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mVisemes)
        COPY_MEMBER(mFrames)
        COPY_MEMBER(mData)
    END_COPYING_MEMBERS
END_COPYS

void CharLipSync::Print(TextStream &ts) {
    std::vector<unsigned char> data;
    data.resize(mVisemes.size());
    for (int i = 0; i < mVisemes.size(); i++) {
        data[i] = 0;
    }
    ts << "; song: " << PathName(this) << "\n";
    ts << "(visemes\n";
    for (int i = 0; i < mVisemes.size(); i++) {
        ts << "   " << mVisemes[i];
    }
    ts << ")\n";
    ts << "(frames ; @ 30fps\n";
    for (int i = 0; i < mFrames; i++) {
        for (int j = 0; j < mData[i]; j++) {
            data[data[j]] = data[j + 1];
        }
        ts << "   ( ";
        for (int j = 0; j < mVisemes.size(); j++) {
            ts << data[j] * 0.003921568859368563f << " ";
        }
        ts << ")\n";
    }
    ts << ")\n";
}

void CharLipSync::Init() { sLipSyncMap = new std::map<Symbol, CharLipSync *>(); }
void CharLipSync::Terminate() { RELEASE(sLipSyncMap); }

void CharLipSync::RegisterLipSync(CharLipSync *sync) {
    if (sLipSyncMap) {
        (*sLipSyncMap)[sync->Name()] = sync;
    }
}

void CharLipSync::UnregisterLipSync(CharLipSync *sync) {
    if (sLipSyncMap) {
        sLipSyncMap->erase(sync->Name());
    }
}

DataNode CharLipSync::OnParse(DataArray *a) {
    FilePath path(a->Str(2));
    DataArray *data = DataReadFile(path.c_str(), true);
    if (data) {
        Parse(data);
        data->Release();
    }
    return 0;
}

DataNode CharLipSync::OnParseArray(DataArray *a) {
    DataArray *data = a->Array(2);
    if (data) {
        Parse(data);
        data->Release();
    }
    return 0;
}

void CharLipSync::Parse(DataArray *data) {
    DataArray *visemeArr = data->FindArray("visemes");
    mVisemes.resize(visemeArr->Size() - 1);
    for (int i = 1; i < visemeArr->Size(); i++) {
        mVisemes[i - 1] = visemeArr->Str(i);
    }
    Generator gen;
    gen.Init(this);
    DataArray *frameArr = data->FindArray("frames");
    for (int i = 1; i < frameArr->Size(); i++) {
        DataArray *curArr = frameArr->Array(i);
        for (int j = 0; j < curArr->Size(); j++) {
            gen.AddWeight(j, curArr->Float(j));
        }
        gen.NextFrame();
    }
    gen.Finish();
    Print(TheDebug);
}

void CharLipSync::Generator::Init(CharLipSync *sync) {
    mLipSync = sync;
    mLipSync->mData.resize(0);
    mWeights.resize(mLipSync->mVisemes.size());
    for (int i = 0; i < mWeights.size(); i++) {
        mWeights[i].unk0 = 0;
        mWeights[i].unk1 = 0;
    }
    mLastCount = mLipSync->mData.size();
    mLipSync->mData.push_back(0);
    mLipSync->mFrames = 0;
}

void CharLipSync::Generator::NextFrame() {
    int count = mLipSync->mData.size() - 1 - mLastCount;
    MILO_ASSERT(count >= 0 && count < 256, 0x53);
    mLipSync->mData[mLastCount] = count;
    mLastCount = mLipSync->mData.size();
    mLipSync->mData.push_back(0);
    mLipSync->mFrames++;
}

CharLipSync *CharLipSync::FindLipSyncForSound(Sound *sound) {
    if (sLipSyncMap) {
        String name(sound->Name());
        int ext = name.find_last_of('.');
        if (ext >= 0) {
            name.resize(ext);
            name += ".lipsync";
            return (*sLipSyncMap)[name.c_str()];
        }
    }
    return nullptr;
}
