#include "char/CharLipSync.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "rndobj/PropAnim.h"
#include "utl/TextStream.h"

std::map<Symbol, CharLipSync *> *CharLipSync::sLipSyncMap;

#pragma region Generator

void CharLipSync::Generator::Init(CharLipSync *sync) {
    mLipSync = sync;
    mLipSync->mData.resize(0);
    mWeights.resize(mLipSync->mVisemes.size());
    for (int i = 0; i < mWeights.size(); i++) {
        mWeights[i].last = 0;
        mWeights[i].current = 0;
    }
    mLastCount = mLipSync->mData.size();
    mLipSync->mData.push_back(0);
    mLipSync->mFrames = 0;
}

void CharLipSync::Generator::NextFrame() {
    int count = (mLipSync->mData.size() - 1 - mLastCount) / 2;
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

    const std::vector<unsigned char> &data = mLipSync->mData;
    int idx = 0;
    for (int i = 0; i < mLipSync->mFrames; i++) {
        unsigned char count = data[idx++];
        MILO_ASSERT(count <= mLipSync->mVisemes.size(), 0x6A);
        for (int j = 0; j < count; j++) {
            unsigned char viseme = data[idx++];
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
        unsigned char count = data[cur++];
        for (int j = 0; j < count; j++) {
            if (data[cur] >= visemeIdx) {
                data[cur]--;
                MILO_ASSERT(data[cur] < mLipSync->mVisemes.size(), 0x96);
            }
            cur += 2;
        }
    }
}

void CharLipSync::Generator::AddWeight(int i1, float f2) {
    unsigned char clamped = Clamp<float>(0.0f, 255.0f, f2 * 255.0f + 0.5f);
    if (mWeights[i1].last != clamped || mWeights[i1].current != clamped) {
        mLipSync->mData.push_back(i1);
        mLipSync->mData.push_back(clamped);
        mWeights[i1].last = mWeights[i1].current;
        mWeights[i1].current = clamped;
    }
}

#pragma endregion
#pragma region PlayBack

CharLipSync::PlayBack::PlayBack()
    : mLipSync(nullptr), mClips(nullptr), mIndex(0), mOldIndex(0), mFrame(-1) {}

void CharLipSync::PlayBack::Reset() {
    mIndex = 0;
    mFrame = -1;
    for (int i = 0; i < mWeights.size(); i++) {
        Weight &cur = mWeights[i];
        cur.next = 0;
        cur.current = 0;
        cur.last = 0;
    }
}

void CharLipSync::PlayBack::Set(CharLipSync *lipsync, ObjPtr<ObjectDir> clips) {
    mClips = clips;
    mLipSync = lipsync;

    int numVisemes = mLipSync->mVisemes.size();
    mWeights.resize(numVisemes);

    for (int i = 0; i < mWeights.size(); i++) {
        ObjPtr<CharClip> &clip = mWeights[i].clip;
        clip = mClips->Find<CharClip>(mLipSync->mVisemes[i].c_str(), false);
        if (!clip) {
            MILO_NOTIFY("could not find %s", mLipSync->mVisemes[i].c_str());
        }
    }

    static Message viseme_list("viseme_list");
    DataNode result = mLipSync->Handle(viseme_list, false);
    if (result.Type() == kDataArray) {
        int newSize = numVisemes + result.Array()->Size();
        if (mWeights.size() != newSize) {
            mWeights.resize(newSize);
            for (int i = 0; i + numVisemes < newSize; i++) {
                Symbol visemeSym = result.Array()->Sym(i);
                ObjPtr<CharClip> &clip = mWeights[i + numVisemes].clip;
                clip = mClips->Find<CharClip>(visemeSym.Str(), false);
            }
        }
    }
}

void CharLipSync::PlayBack::SetClips(ObjPtr<ObjectDir> clips) {
    if (!mLipSync) {
        return;
    } else {
        mClips = clips;
        static Message viseme_list("viseme_list");
        DataNode result = mLipSync->Handle(viseme_list, false);
        if (result.Type() == kDataArray) {
            int aSize = result.Array()->Size();
            if (mWeights.size() == aSize) {
                mWeights.resize(aSize);
                for (int i = 0; i < aSize; i++) {
                    Symbol visemeSym = result.Array()->Sym(i);
                    ObjPtr<CharClip> &clip = mWeights[i].clip;
                    clip = mClips->Find<CharClip>(visemeSym.Str(), false);
                }
            }
        }
    }
}

void CharLipSync::PlayBack::Poll(float frame) {
    if (mLipSync) {
        static Message viseme_list("viseme_list");
        DataNode result = mLipSync->Handle(viseme_list, false);
        if (result.Type() == kDataArray) {
            int numVisemes = mLipSync->mVisemes.size();
            int newSize = numVisemes + result.Array()->Size();
            for (int i = numVisemes; i < newSize; i++) {
                float fprop =
                    mLipSync->Property(result.Array()->Sym(i - numVisemes))->Float();
                if (i < mWeights.size()) {
                    mWeights[i].current = Clamp(0.0f, 1.0f, fprop);
                }
            }
        }
        if (mLipSync->mFrames < 2) {
            return;
        } else {
            float mult = frame * 30.0f;
            float ceiled = ceilf(mult);
            int i8 = ceiled;
            float f14 = mult - (float)(i8 - 1);
            if (i8 < 1) {
                i8 = 1;
                f14 = 0;
            } else if (i8 >= mLipSync->mFrames - 1) {
                i8 = mLipSync->mFrames - 1;
                f14 = 0.99999988f;
            }
            CharLipSync *lipsync = mLipSync;
            if (i8 < mFrame) {
                Reset();
            }
            if (mFrame < i8) {
                for (; mFrame < i8; mFrame++) {
                    mOldIndex = mIndex++;
                    unsigned char count = lipsync->mData[mOldIndex];
                    for (int j = 0; j != count; j++) {
                        Weight &cur = mWeights[lipsync->mData[mIndex++]];
                        cur.last = cur.next;
                        cur.next = (float)lipsync->mData[mIndex++] * 0.003921569f;
                        cur.current = Interp(cur.last, cur.next, f14);
                    }
                }
            } else if (mFrame >= 0 && mFrame == i8) {
                int idx = mOldIndex;
                unsigned char count = lipsync->mData[idx++];
                for (int i = 0; i != count; i++) {
                    Weight &cur = mWeights[lipsync->mData[idx++]];
                    cur.current = Interp(cur.last, cur.next, f14);
                }
            }
        }
    }
}

#pragma endregion
#pragma region CharLipSync

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

INIT_REVS(2, 0)

BEGIN_LOADS(CharLipSync)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mVisemes;
    d >> mFrames;
    d >> mData;
    if (d.rev == 1) {
        ObjPtr<RndPropAnim> mPropAnim(this);
        d >> mPropAnim;
    }
    RegisterLipSync(this);
END_LOADS

void CharLipSync::Print(TextStream &ts) {
    std::vector<unsigned char> data;
    data.resize(mVisemes.size());
    for (int i = 0; i < mVisemes.size(); i++) {
        data[i] = 0;
    }
    ts << "; song: " << PathName(this) << "\n";
    ts << "(visemes\n";
    for (int i = 0; i < mVisemes.size(); i++) {
        const FixedString &str = mVisemes[i];
        ts << "   " << str << "\n";
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

#pragma endregion
