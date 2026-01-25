#include "char/CharLipSync.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Msg.h"
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
        ts << "   " << mVisemes[i] << "\n";
    }
    ts << ")\n";
    ts << "(frames ; @ 30fps\n";
    int idx = 0;
    for (int i = 0; i < mFrames; i++) {
        int count = mData[idx++];
        for (int j = 0; j < count; j++) {
            int visemeIdx = mData[idx++];
            data[visemeIdx] = mData[idx++];
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

void CharLipSync::Generator::Finish() {
    mLipSync->mData.pop_back();
    std::vector<bool> bools;
    bools.resize(mLipSync->mVisemes.size());
    for (int i = 0; i < bools.size(); i++) {
        bools[i] = false;
    }

    std::vector<unsigned char> &data = mLipSync->mData;
    int idx = 0;
    for (int i = 0; i < mLipSync->mFrames; i++) {
        int count = data[idx++];
        MILO_ASSERT(count <= mLipSync->mVisemes.size(), 0x6A);
        for (int j = 0; j < count; j++) {
            int viseme = data[idx++];
            MILO_ASSERT(viseme < mLipSync->mVisemes.size(), 0x6E);
            if (data[idx++] != 0) {
                bools[viseme] = true;
            }
        }
    }

    for (int i = 0; i < bools.size();) {
        if (!bools[i]) {
            bools.erase(bools.begin() + i);
            RemoveViseme(i);
        } else {
            i++;
        }
    }
}

void CharLipSync::Generator::RemoveViseme(int visemeIdx) {
    mLipSync->mVisemes.erase(mLipSync->mVisemes.begin() + visemeIdx);

    std::vector<unsigned char> &data = mLipSync->mData;
    int cur = 0;
    for (int i = 0; i < mLipSync->mFrames; i++) {
        int count = data[cur++];
        for (int j = 0; j < count; j++) {
            if (data[cur] >= visemeIdx) {
                data[cur]--;
                MILO_ASSERT(data[cur] < mLipSync->mVisemes.size(), 0x96);
            }
            cur += 2;
        }
    }
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

void CharLipSync::PlayBack::Set(CharLipSync *lipsync, ObjPtr<ObjectDir> clips) {
    mClips = clips;
    mLipSync = lipsync;

    int numVisemes = mLipSync->mVisemes.size();
    mWeights.resize(numVisemes);

    for (int i = 0; i < mWeights.size(); i++) {
        ObjPtr<CharClip> &clip = mWeights[i].unk0;
        clip = mClips->Find<CharClip>(mLipSync->mVisemes[i].c_str(), false);
        if (!clip) {
            MILO_NOTIFY("could not find %s", mLipSync->mVisemes[i].c_str());
        }
    }

    static Message viseme_list("viseme_list");
    DataNode result = mLipSync->Handle(viseme_list, false);

    if (result.Type() == kDataArray) {
        DataArray *arr = result.Array(0);
        int arrSize = arr->Size();
        int newSize = numVisemes + arrSize;
        if (mWeights.size() != newSize) {
            mWeights.resize(newSize);
            for (int i = numVisemes; i < newSize; i++) {
                Symbol visemeSym = arr->Sym(i - numVisemes);
                ObjPtr<CharClip> &clip = mWeights[i].unk0;
                clip = mClips->Find<CharClip>(visemeSym.Str(), false);
            }
        }
    }
}

void CharLipSync::PlayBack::Poll(float time) {
    if (!mLipSync)
        return;

    static Message viseme_list("viseme_list");
    DataNode result = mLipSync->Handle(viseme_list, false);
    float zero = 0.0f;

    if (result.Type() == kDataArray) {
        int numVisemes = mLipSync->mVisemes.size();
        DataArray *arr = result.Array(0);
        int end = arr->Size() + numVisemes;
        if (numVisemes < end) {
            int visIdx = 0;
            float one = 1.0f;
            CharLipSync *ls = mLipSync;
            for (; numVisemes < end; visIdx++, numVisemes++) {
                Symbol visemeSym = result.Array(0)->Sym(visIdx);
                float weight = ls->Property(visemeSym, true)->Float(0);
                if ((unsigned int)numVisemes < mWeights.size()) {
                    mWeights[numVisemes].unk1c = Clamp(zero, one, weight);
                }
            }
        }
    }

    if (mLipSync->mFrames < 2) {
        return;
    }

    float frame = time * 30.0f;
    int frameIdx = (int)ceil(frame);
    float frac = frame - (float)(frameIdx - 1);

    if (frameIdx < 1) {
        frameIdx = 1;
        frac = zero;
    } else if (frameIdx >= mLipSync->mFrames - 1) {
        frameIdx = mLipSync->mFrames - 1;
        frac = 0.9999999f;
    }

    CharLipSync *lipSync = mLipSync;
    if (frameIdx < mFrame) {
        Reset();
    }

    if (mFrame < frameIdx) {
        float conv = 1.0f / 255.0f;
        do {
            mOldIndex = mIndex++;
            int count = lipSync->mData[mOldIndex];
            if (count != 0) {
                for (int i = count; i != 0; i--) {
                    int idx = lipSync->mData[mIndex++];
                    Weight &w = mWeights[idx];
                    w.unk14 = w.unk18;
                    int val = lipSync->mData[mIndex++];
                    w.unk18 = (float)val * conv;
                    w.unk1c = Interp(w.unk14, w.unk18, frac);
                }
            }
            mFrame++;
        } while (mFrame < frameIdx);
    } else if (mFrame >= 0 && mFrame == frameIdx) {
        int count = lipSync->mData[mOldIndex];
        if (count != 0) {
            int idx = mOldIndex + 1;
            for (int i = count; i != 0; i--) {
                int wIdx = lipSync->mData[idx];
                idx += 2;
                Weight &w = mWeights[wIdx];
                w.unk1c = Interp(w.unk14, w.unk18, frac);
            }
        }
    }
}
