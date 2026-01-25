#pragma once
#include "char/CharClip.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "synth/Sound.h"
#include "utl/MemMgr.h"
#include "utl/TextStream.h"

/** "A full lipsync animation, basically a changing set of weights
    for a set of named visemes.  Sampled at 30hz" */
class CharLipSync : public Hmx::Object {
public:
    class Generator {
    public:
        struct Weight {
            unsigned char unk0;
            unsigned char unk1;
        };

        Generator() : mLipSync(nullptr), mLastCount(0) {}
        void Init(CharLipSync *);
        void AddWeight(int, float);
        void NextFrame();
        void Finish();

    protected:
        void RemoveViseme(int);

        CharLipSync *mLipSync; // 0x0
        int mLastCount; // 0x4
        std::vector<Weight> mWeights; // 0x8
    };

    class PlayBack {
    public:
        struct Weight {
            Weight() : unk0(nullptr) {}

            ObjPtr<CharClip> unk0;
            float unk14;
            float unk18;
            float unk1c;
        };
        PlayBack();
        void Set(CharLipSync *, ObjPtr<ObjectDir>);
        void SetClips(ObjPtr<ObjectDir>);
        void Reset();
        void Poll(float);

        MEM_OVERLOAD(PlayBack, 0x3F)

        std::vector<Weight> mWeights; // 0x0
        ObjPtr<CharLipSync> mLipSync; // 0xc
        ObjPtr<ObjectDir> mClips; // 0x20
        int mIndex; // 0x34
        int mOldIndex; // 0x38
        int mFrame; // 0x3c
    };

    // Hmx::Object
    virtual ~CharLipSync();
    OBJ_CLASSNAME(CharLipSync);
    OBJ_SET_TYPE(CharLipSync);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x1E)
    NEW_OBJ(CharLipSync)

    float Duration() { return (float)(mFrames - 1) / 30.0f; }
    void Print(TextStream &);
    void Parse(DataArray *);

    static void Init();
    static void Terminate();
    static void RegisterLipSync(CharLipSync *);
    static void UnregisterLipSync(CharLipSync *);
    static CharLipSync *FindLipSyncForSound(Sound *);

protected:
    CharLipSync();

    static std::map<Symbol, CharLipSync *> *sLipSyncMap;

    DataNode OnParse(DataArray *);
    DataNode OnParseArray(DataArray *);

    /** "viseme names" */
    std::vector<String> mVisemes; // 0x2c
    /** "how many keyframes" */
    int mFrames; // 0x38
    std::vector<unsigned char> mData; // 0x3c
};
