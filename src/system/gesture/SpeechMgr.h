#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "xdk/NUI.h"
#include "xdk/nui/nuispeech.h"

class SpeechMgr : public Hmx::Object {
public:
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
    float SpeechConfThresh() const { return mSpeechConfThresh; }
    RndOverlay *Overlay() const { return mOverlay; }
    bool Enabled() const { return mEnabled; }
    int VoiceDirection() const { return mVoiceDirection; }

private:
    struct Grammar {
        bool FinishLoad(SpeechMgr *);
        void Unload();

        bool operator==(const Symbol s) { return mName == s; }

        Symbol mName; // 0x0
        String mFile; // 0x4
        NUI_SPEECH_GRAMMAR mGrammar; // 0x8
        bool mLoaded; // 0x14
    };

    bool GetSpeechLanguage(NUI_SPEECH_LANGUAGE &) const;
    float DetermineSpeechConfidenceThreshold() const;
    void Disable();
    void BeginRecognition();
    bool IsSpeechSupportable(NUI_SPEECH_LANGUAGE &) const;
    void Reload();
    void PrintSemanticTree(NUI_SPEECH_SEMANTICRESULT *, int);
    void ProcessRecoResult(NUI_SPEECH_RECORESULT *);

    NUI_SPEECH_LANGUAGE mLanguage; // 0x2c
    std::vector<Grammar> mGrammars; // 0x30
    bool mEnabled; // 0x3c
    bool mRecognizing; // 0x3d
    bool unk3e;
    bool mSpeechSupported; // 0x3f
    float mSpeechConfThresh; // 0x40
    int mVoiceDirection; // 0x44
    RndOverlay *mOverlay; // 0x48
};

extern SpeechMgr *TheSpeechMgr;

#include "obj/Msg.h"

DECLARE_MESSAGE(SpeechEnableMsg, "speech_enable")
SpeechEnableMsg(bool enabled) : Message(Type(), enabled) {}
END_MESSAGE

// #define SPEECH_RECO_MSG (speech_reco ($tags $confidence $rule_name))
DECLARE_MESSAGE(SpeechRecoMessage, "speech_reco")
SpeechRecoMessage(DataArrayPtr tags, float conf, Symbol rule)
    : Message(Type(), tags, conf, rule) {}
DataArray *Tags() const { return mData->Array(2); }
float Confidence() const { return mData->Float(3); }
Symbol RuleName() const { return mData->Sym(4); }
END_MESSAGE
