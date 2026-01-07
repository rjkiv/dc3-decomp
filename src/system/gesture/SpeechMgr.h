#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "xdk/NUI.h"

class SpeechMgr : public Hmx::Object {
public:
    struct Grammar {
        bool FinishLoad(SpeechMgr *);
        void Unload();

        bool operator==(const Symbol &s) { return mName == s; }

        Symbol mName; // 0x0
        String unk4;
        NUI_SPEECH_GRAMMAR mGrammar; // 0x8
        bool unk14;
    };
    SpeechMgr(const DataArray *);
    virtual ~SpeechMgr();
    virtual DataNode Handle(DataArray *, bool);

    bool IsRecognizing() const;
    void DisableAndUnloadGrammars();
    void CreateGrammar(Symbol);
    void EnableAndLoadGrammars(const DataArray *);
    void Enable(bool);
    String GetSpeechLanguageDir() const;
    void SetGrammarState(Symbol, bool);
    void InitGrammars(const DataArray *);
    void LoadGrammar(Symbol, const char *, bool);
    void Poll();
    bool OnIsSpeechSupportable() const;
    void ForceLanguageSupport();
    void SetRecognizing(bool);
    void SetRule(Symbol, Symbol, bool);
    bool HasGrammar(Symbol);
    void UnloadGrammar(Symbol);
    void AddDynamicRule(Symbol, const char *, void **);
    void AddDynamicRuleWord(Symbol, const char *, const char *, void **, void **);
    void CommitGrammar(Symbol);

    bool SpeechSupported() const { return mSpeechSupported; }
    bool Recognizing() const { return mRecognizing; }

private:
    bool GetSpeechLanguage(NUI_SPEECH_LANGUAGE &) const;
    float DetermineSpeechConfidenceThreshold() const;
    void Disable();

    NUI_SPEECH_LANGUAGE mLanguage; // 0x2c
    std::vector<Grammar> mGrammars; // 0x30
    bool mEnabled; // 0x3c
    bool mRecognizing; // 0x3d
    bool unk3e;
    bool mSpeechSupported; // 0x3f
    float mSpeechConfThresh; // 0x40
    int unk44;
    RndOverlay *mOverlay; // 0x48
};

extern SpeechMgr *TheSpeechMgr;

#include "obj/Msg.h"

DECLARE_MESSAGE(SpeechEnableMsg, "speech_enable")
SpeechEnableMsg(bool enabled) : Message(Type(), enabled) {}
END_MESSAGE

DECLARE_MESSAGE(SpeechRecoMessage, "speech_reco")
// arg 0 is a DataArray*
// arg 1 is a float
END_MESSAGE
