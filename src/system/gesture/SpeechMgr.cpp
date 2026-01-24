#include "gesture/SpeechMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/Overlay.h"
#include "stdlib.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include "xdk/NUI.h"
#include "xdk/XAPILIB.h"
#include "xdk/nui/nuispeech.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xapilibi/xbase.h"
#include <string>
#include <cstdlib>

SpeechMgr *TheSpeechMgr;
ULONG gGrammarID;

const char *WstrToANSI(const LPCWSTR &wstr) {
    static CHAR sBuffer[1024];
    int ret =
        WideCharToMultiByte(0, 0, wstr, -1, sBuffer, sizeof(sBuffer), nullptr, nullptr);
    MILO_ASSERT(ret > 0, 0x2A);
    return sBuffer;
}

std::wstring ANSItoWstr(const char *ansi) {
    size_t ret;
    WCHAR wstr[1024];
    mbstowcs_s(&ret, wstr, 1024, ansi, strlen(ansi));
    return std::wstring(wstr);
}

#pragma region Grammar

void SpeechMgr::Grammar::Unload() {
    if (mLoaded) {
        MILO_ASSERT(!TheSpeechMgr->IsRecognizing(), 0xC4);
        AutoGlitchReport report(50, "SpeechMgr::Grammar::Unload");
        HRESULT res = NuiSpeechUnloadGrammar(&mGrammar);
        MILO_ASSERT_FMT(
            SUCCEEDED(res), "NuiSpeechUnloadGrammar failed with error 0x%08x", res
        );
    }
    mLoaded = false;
}

bool SpeechMgr::Grammar::FinishLoad(SpeechMgr *mgr) {
    if (!mgr->Enabled()) {
        return false;
    }
    MILO_ASSERT(!TheSpeechMgr->IsRecognizing(), 0x90);
    const char *ansi = mFile.c_str();
    HRESULT res = NuiSpeechLoadGrammar(
        ANSItoWstr(ansi).c_str(), gGrammarID++, NUI_SPEECH_LOADOPTIONS_STATIC, &mGrammar
    );
    mLoaded = false;
    if (res == E_INVALIDARG) {
        MILO_NOTIFY("NuiSpeechLoadGrammar (E_INVALIDARG)");
    } else if (res == (HRESULT)0x80045052) {
        MILO_NOTIFY("Grammar %s is not localized properly for this language", ansi);
        mgr->Disable();
    } else if (res == (HRESULT)0x80045024) {
        MILO_NOTIFY("Grammar %s has an invalid import", ansi);
    } else if (res == (HRESULT)0x8004503a) {
        MILO_NOTIFY("Grammar %s has an element that was not found", ansi);
    } else if (res == (HRESULT)0x8004502a) {
        MILO_NOTIFY("Grammar %s has no rules", ansi);
    } else if (res != ERROR_SUCCESS) {
        MILO_NOTIFY("NuiSpeechLoadGrammar failed 0x%x", res);
    } else {
        mLoaded = true;
    }
    return mLoaded;
}

#pragma endregion
#pragma region SpeechMgr

SpeechMgr::SpeechMgr(const DataArray *)
    : mEnabled(0), mRecognizing(0), unk3e(0), mOverlay(RndOverlay::Find("speech_mgr")) {
    TheSpeechMgr = this;
    SetName("speech_mgr", ObjectDir::Main());
    mSpeechSupported = GetSpeechLanguage(mLanguage);
    mSpeechConfThresh = DetermineSpeechConfidenceThreshold();
    mOverlay->SetLines(16);
    mVoiceDirection = -1;
}

SpeechMgr::~SpeechMgr() {
    TheSpeechMgr = nullptr;
    DisableAndUnloadGrammars();
}

BEGIN_HANDLERS(SpeechMgr)
    HANDLE_EXPR(is_recognizing, IsRecognizing())
    HANDLE_ACTION(set_recognizing, SetRecognizing(_msg->Int(2)))
    HANDLE_ACTION(begin_recognition, BeginRecognition())
    HANDLE_ACTION(set_rule, SetRule(_msg->Sym(2), _msg->Sym(3), _msg->Int(4)))
    HANDLE_ACTION(
        emulate_input, NuiSpeechEmulateRecognition(ANSItoWstr(_msg->Str(2)).c_str(), 1)
    )
    HANDLE_EXPR(get_voice_direction, mVoiceDirection)
    HANDLE_EXPR(has_language_support, mSpeechSupported)
    HANDLE_EXPR(get_confidence_threshold, mSpeechConfThresh)
    HANDLE_ACTION(force_language_support, ForceLanguageSupport())
    HANDLE_EXPR(is_speech_supportable, OnIsSpeechSupportable())
    HANDLE_ACTION(disable, Disable())
    HANDLE_ACTION(reload, Reload())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool SpeechMgr::IsRecognizing() const { return mRecognizing; }

bool SpeechMgr::OnIsSpeechSupportable() const {
    NUI_SPEECH_LANGUAGE lang;
    return IsSpeechSupportable(lang);
}

void SpeechMgr::CreateGrammar(Symbol name) {
    for (int i = 0; i < mGrammars.size(); i++) {
        MILO_ASSERT(name != mGrammars[i].mName, 0x3A2);
    }
    Grammar grammar;
    grammar.mLoaded = false;
    grammar.mName = name;
    grammar.mFile = "";
    HRESULT res = NuiSpeechCreateGrammar(gGrammarID++, &grammar.mGrammar);
    MILO_ASSERT_FMT(
        SUCCEEDED(res), "NuiSpeechCreateGrammar failed with error 0x%08x", res
    );
    mGrammars.push_back(grammar);
}

void SpeechMgr::EnableAndLoadGrammars(const DataArray *cfg) {
    if (!mEnabled) {
        Enable(false);
        if (mEnabled) {
            DataArray *grammarArr = cfg->FindArray("grammars");
            for (int i = 1; i < grammarArr->Size(); i++) {
                DataArray *curArr = grammarArr->Array(i);
                Grammar grammar;
                grammar.mName = curArr->Sym(0);
                String file =
                    String("grammar/") + GetSpeechLanguageDir() + "/" + curArr->Str(1);
                if (FileExists(file.c_str(), FILE_OPEN_NOARK, &grammar.mFile)
                    && grammar.FinishLoad(this)) {
                    mGrammars.push_back(grammar);
                    SetGrammarState(grammar.mName, false);
                }
            }
        }
        SpeechEnableMsg msg(mEnabled);
        Export(msg, false);
    }
}

void SpeechMgr::DisableAndUnloadGrammars() {
    static bool gDisableGrammars = false;
    static SpeechEnableMsg speechEnableMsg(gDisableGrammars);
    Export(speechEnableMsg, false);
    FOREACH (it, mGrammars) {
        it->Unload();
    }
    mGrammars.clear();
    if (mEnabled) {
        Disable();
    }
}

void SpeechMgr::Enable(bool english) {
    if (mSpeechSupported) {
        MILO_ASSERT(mEnabled == false, 0x13F);
        NUI_SPEECH_INIT_PROPERTIES props;
        memset(&props, 0, sizeof(NUI_SPEECH_INIT_PROPERTIES));
        if (english) {
            props.Language = NUI_SPEECH_LANGUAGE_EN_US;
        } else {
            props.Language = mLanguage;
        }
        props.MicrophoneType = NUI_SPEECH_KINECT;
        MemForceNewOperatorAlign(16);
        HRESULT enableRes = NuiSpeechEnable(&props, 3);
        MemForceNewOperatorAlign(0);
        if (enableRes != ERROR_SUCCESS) {
            if (enableRes == E_INVALIDARG) {
                MILO_FAIL("NuiSpeechEnable failed E_INVALIDARG");
            } else if (enableRes == E_NUI_DATABASE_NOT_FOUND) {
                MILO_FAIL("NuiSpeechEnable failed E_NUI_DATABASE_NOT_FOUND");
            } else if (enableRes == E_NUI_DATABASE_VERSION_MISMATCH) {
                MILO_FAIL("NuiSpeechEnable failed E_NUI_DATABASE_VERSION_MISMATCH");
            } else {
                MILO_LOG("NuiSpeechEnable failed with error 0x%x (no Kinect?)\n", enableRes);
            }
        } else {
            mEnabled = true;
            HRESULT interestRes = NuiSpeechSetEventInterest(0x40);
            MILO_ASSERT_FMT(
                SUCCEEDED(interestRes),
                "NuiSpeechSetEventInterest failed with error 0x%08x",
                interestRes
            );
        }
    }
}

void SpeechMgr::Disable() {
    if (mEnabled) {
        HRESULT stopRes = NuiSpeechStopRecognition();
        MILO_ASSERT_FMT(
            SUCCEEDED(stopRes),
            "NuiSpeechStopRecognition failed with error 0x%08x",
            stopRes
        );
        HRESULT disableRes = NuiSpeechDisable();
        MILO_ASSERT_FMT(
            SUCCEEDED(disableRes), "NuiSpeechDisable failed with error 0x%08x", disableRes
        );
        mEnabled = false;
    }
}

void SpeechMgr::InitGrammars(const DataArray *cfg) {
    if (mSpeechSupported)
        EnableAndLoadGrammars(cfg);
}

void SpeechMgr::LoadGrammar(Symbol name, const char *filename, bool) {
    MILO_ASSERT(std::find(mGrammars.begin(), mGrammars.end(), name) == mGrammars.end(), 0x263);
    MILO_ASSERT(mEnabled, 0x264);
    Grammar grammar;
    grammar.mLoaded = false;
    grammar.mName = name;
    if (FileExists(filename, FILE_OPEN_NOARK, &grammar.mFile)
        && grammar.FinishLoad(this)) {
        mGrammars.push_back(grammar);
    }
}

float SpeechMgr::DetermineSpeechConfidenceThreshold() const {
    float conf = 0.20f;
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    DWORD locale = ULSystemLocale();
    if (lang == "eng"
        && (locale == XC_LOCALE_AUSTRALIA || locale == XC_LOCALE_NEW_ZEALAND)) {
        conf = 0.15f;
    }
    if (lang == "fre") {
        if (locale == XC_LOCALE_CANADA) {
            conf = 0.15f;
        } else {
            conf = 0.30f;
        }
    }
    if (lang == "ita") {
        conf = 0.05f;
    }
    if (lang == "deu") {
        conf = 0.25f;
    }
    if (lang == "esl") {
        conf = 0.10f;
    }
    if (lang == "mex") {
        conf = 0.10f;
    }
    if (lang == "jpn") {
        conf = 0.10f;
    }
    return conf;
}

String SpeechMgr::GetSpeechLanguageDir() const {
    switch (mLanguage) {
    case NUI_SPEECH_LANGUAGE_EN_US:
        return "en-us";
    case NUI_SPEECH_LANGUAGE_FR_CA:
        return "fr-ca";
    case NUI_SPEECH_LANGUAGE_EN_GB:
        return "en-gb";
    case NUI_SPEECH_LANGUAGE_ES_MX:
        return "es-mx";
    case NUI_SPEECH_LANGUAGE_JA_JP:
        return "ja-jp";
    case NUI_SPEECH_LANGUAGE_FR_FR:
        return "fr-fr";
    case NUI_SPEECH_LANGUAGE_ES_ES:
        return "es-es";
    case NUI_SPEECH_LANGUAGE_DE_DE:
        return "de-de";
    case NUI_SPEECH_LANGUAGE_IT_IT:
        return "it-it";
    case NUI_SPEECH_LANGUAGE_EN_AU:
        return "en-au";
    default:
        MILO_ASSERT(false, 0x25A);
        return gNullStr;
    }
}

void SpeechMgr::SetRecognizing(bool recognizing) {
    if (mEnabled && unk3e) {
        mRecognizing = recognizing;
        if (recognizing) {
            HRESULT res = NuiSpeechStartRecognition();
            MILO_ASSERT_FMT(
                SUCCEEDED(res), "NuiSpeechStartRecognition failed with error 0x%08x", res
            );
        } else {
            HRESULT res = NuiSpeechStopRecognition();
            if (res == E_SPEECH_UNINITIALIZED) {
                MILO_NOTIFY("Speech is not initialized");
            } else {
                MILO_ASSERT_FMT(
                    SUCCEEDED(res),
                    "NuiSpeechStopRecognition failed with error 0x%08x",
                    res
                );
            }
        }
    }
}

void SpeechMgr::BeginRecognition() {
    if (!unk3e) {
        MILO_ASSERT(!mRecognizing, 0x2E2);
        unk3e = true;
        SetRecognizing(true);
    }
}

void SpeechMgr::ForceLanguageSupport() {
    if (!mEnabled) {
        mSpeechSupported = IsSpeechSupportable(mLanguage);
        if (mSpeechSupported) {
            EnableAndLoadGrammars(SystemConfig("kinect")->FindArray("speech"));
        }
    }
}

void SpeechMgr::Reload() {
    DisableAndUnloadGrammars();
    mSpeechSupported = IsSpeechSupportable(mLanguage);
    if (mSpeechSupported) {
        EnableAndLoadGrammars(SystemConfig("kinect", "speech"));
    } else {
        SpeechEnableMsg msg(false);
        Export(msg, false);
    }
}

bool SpeechMgr::GetSpeechLanguage(NUI_SPEECH_LANGUAGE &nuiLanguage) const {
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    DWORD locale = ULSystemLocale();
    if (lang == "eng") {
        if (locale == XC_LOCALE_AUSTRALIA || locale == XC_LOCALE_NEW_ZEALAND) {
            nuiLanguage = NUI_SPEECH_LANGUAGE_EN_AU;
            return true;
        } else if (locale == XC_LOCALE_IRELAND || locale == XC_LOCALE_GREAT_BRITAIN) {
            nuiLanguage = NUI_SPEECH_LANGUAGE_EN_GB;
            return true;
        } else if (locale == XC_LOCALE_UNITED_STATES || locale == XC_LOCALE_CANADA) {
            nuiLanguage = NUI_SPEECH_LANGUAGE_EN_US;
            return true;
        } else {
            return false;
        }
    } else if (lang == "fre") {
        if (locale == XC_LOCALE_CANADA) {
            nuiLanguage = NUI_SPEECH_LANGUAGE_FR_CA;
            return true;
        } else if (locale == XC_LOCALE_FRANCE) {
            nuiLanguage = NUI_SPEECH_LANGUAGE_FR_FR;
            return true;
        } else {
            return false;
        }
    } else if (lang == "ita" && locale == 0x13) {
        nuiLanguage = NUI_SPEECH_LANGUAGE_IT_IT;
        return true;
    } else if (lang == "deu" && locale == 0xD) {
        nuiLanguage = NUI_SPEECH_LANGUAGE_DE_DE;
        return true;
    } else if (lang == "esl" && locale == 0x1F) {
        nuiLanguage = NUI_SPEECH_LANGUAGE_ES_ES;
        return true;
    } else if (lang == "mex" && locale == 0x16) {
        nuiLanguage = NUI_SPEECH_LANGUAGE_ES_MX;
        return true;
    } else if (lang == "jpn" && locale == 0x14) {
        nuiLanguage = NUI_SPEECH_LANGUAGE_JA_JP;
        return true;
    } else {
        return false;
    }
}

bool SpeechMgr::IsSpeechSupportable(NUI_SPEECH_LANGUAGE &nuiLanguage) const {
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    DWORD locale = ULSystemLocale();
    if (locale != 0x17 && locale != 0x20) {
        if (lang == "eng") {
            if (locale == XC_LOCALE_BELGIUM
                || (locale == XC_LOCALE_HONG_KONG && HongKongExceptionMet())) {
                return false;
            } else if (locale == XC_LOCALE_AUSTRALIA || locale == XC_LOCALE_NEW_ZEALAND) {
                nuiLanguage = NUI_SPEECH_LANGUAGE_EN_AU;
            } else if (locale == XC_LOCALE_IRELAND || locale == XC_LOCALE_GREAT_BRITAIN) {
                nuiLanguage = NUI_SPEECH_LANGUAGE_EN_GB;
            } else {
                nuiLanguage = NUI_SPEECH_LANGUAGE_EN_US;
            }
            return true;
        } else if (lang == "fre") {
            if (locale == XC_LOCALE_CANADA) {
                nuiLanguage = NUI_SPEECH_LANGUAGE_FR_CA;
            } else {
                nuiLanguage = NUI_SPEECH_LANGUAGE_FR_FR;
            }
            return true;
        } else if (lang == "ita") {
            nuiLanguage = NUI_SPEECH_LANGUAGE_IT_IT;
            return true;
        } else if (lang == "deu") {
            nuiLanguage = NUI_SPEECH_LANGUAGE_DE_DE;
            return true;
        } else if (lang == "esl") {
            nuiLanguage = NUI_SPEECH_LANGUAGE_ES_ES;
            return true;
        } else if (lang == "mex") {
            nuiLanguage = NUI_SPEECH_LANGUAGE_ES_MX;
            return true;
        } else if (lang == "jpn") {
            nuiLanguage = NUI_SPEECH_LANGUAGE_JA_JP;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void SpeechMgr::PrintSemanticTree(NUI_SPEECH_SEMANTICRESULT *sr, int i2) {
    do {
        MILO_ASSERT(sr, 0x306);
        for (int i = 0; i < i2; i++) {
            mOverlay->Print("  ");
        }
        mOverlay->Print(MakeString("tag: %s\n", WstrToANSI(sr->pcwszValue)));
        if (sr->pFirstChild) {
            PrintSemanticTree(sr->pFirstChild, i2 + 1);
        }
        sr = sr->pNextSibling;
    } while (sr);
}

void SpeechMgr::CommitGrammar(Symbol name) {
    auto it = std::find(mGrammars.begin(), mGrammars.end(), name);
    MILO_ASSERT(it != mGrammars.end(), 0x3FB);
    HRESULT res = NuiSpeechCommitGrammar(&it->mGrammar);
    MILO_ASSERT_FMT(
        SUCCEEDED(res), "NuiSpeechCommitGrammar failed with error 0x%08x", res
    );
    it->mLoaded = true;
}

bool SpeechMgr::HasGrammar(Symbol name) {
    return std::find(mGrammars.begin(), mGrammars.end(), name) != mGrammars.end();
}

void SpeechMgr::SetGrammarState(Symbol name, bool enabled) {
    auto it = std::find(mGrammars.begin(), mGrammars.end(), name);
    if (it != mGrammars.end()) {
        NuiSpeechSetGrammarState(
            &it->mGrammar,
            enabled ? NUI_SPEECH_GRAMMARSTATE_ENABLED : NUI_SPEECH_GRAMMARSTATE_DISABLED
        );
    } else {
        MILO_NOTIFY(
            "Could not find grammar %s to set state to %s",
            name.Str(),
            enabled ? "true" : "false"
        );
    }
}

void SpeechMgr::UnloadGrammar(Symbol name) {
    MILO_ASSERT(mEnabled, 0x271);
    auto it = std::find(mGrammars.begin(), mGrammars.end(), name);
    if (it != mGrammars.end()) {
        it->Unload();
        mGrammars.erase(it);
    } else {
        MILO_NOTIFY("Could not find grammar %s to unload", name);
    }
}

void SpeechMgr::SetRule(Symbol s1, Symbol s2, bool b3) {
    AutoGlitchReport report(35, "SpeechMgr::SetRule");
    if (!mEnabled) {
        return;
    }
    bool old = mRecognizing;
    if (old) {
        SetRecognizing(false);
    }
    auto it = std::find(mGrammars.begin(), mGrammars.end(), s1);
    if (it == mGrammars.end()) {
        MILO_NOTIFY("Grammar %s not found", s1);
        return;
    } else if (!it->mLoaded) {
        MILO_NOTIFY("Grammar %s not loaded", s1);
        return;
    } else {
        std::wstring wstr = ANSItoWstr(s2.Str());
        NUI_SPEECH_RULESTATE eState =
            b3 ? NUI_SPEECH_RULESTATE_ACTIVE : NUI_SPEECH_RULESTATE_INACTIVE;
        if (b3) {
            NuiSpeechSetGrammarState(&it->mGrammar, NUI_SPEECH_GRAMMARSTATE_ENABLED);
        }
        HRESULT res = NuiSpeechSetRuleState(&it->mGrammar, wstr.c_str(), eState);
        if (res != ERROR_SUCCESS) {
            MILO_NOTIFY("NuiSpeechSetRuleState failed with error 0x%08x", res);
            return;
        } else {
            if (mOverlay->Showing()) {
                mOverlay->Print(
                    MakeString("SetRule %s %s %s\n", s1, s2, b3 ? "enabled" : "disabled")
                );
            }
            if (old) {
                SetRecognizing(old);
            }
        }
    }
}

void SpeechMgr::AddDynamicRule(Symbol name, const char *c2, void **initialState) {
    MILO_ASSERT(initialState, 0x3B2);
    auto it = std::find(mGrammars.begin(), mGrammars.end(), name);
    MILO_ASSERT(it != mGrammars.end(), 0x3B5);
    Grammar grammar(*it);
    HRESULT res = NuiSpeechCreateRule(
        &grammar.mGrammar, ANSItoWstr(c2).c_str(), 0x21, true, initialState
    );
    MILO_ASSERT_FMT(SUCCEEDED(res), "NuiSpeechCreateRule failed with error 0x%08x", res);
}

void SpeechMgr::AddDynamicRuleWord(
    Symbol name, const char *c2, const char *c3, void **fromState, void **toState
) {
    MILO_ASSERT(fromState, 0x3C6);
    auto it = std::find(mGrammars.begin(), mGrammars.end(), name);
    MILO_ASSERT(it != mGrammars.end(), 0x3C9);
    Grammar grammar(*it);
    bool createSuccess = true;
    if (toState) {
        HRESULT res = NuiSpeechCreateState(&grammar.mGrammar, *fromState, toState);
        createSuccess = SUCCEEDED(res);
        if (!createSuccess) {
            MILO_NOTIFY(
                "NuiSpeechCreateState failed with error 0x%08x on %s %s", res, c2, c3
            );
        }
    }
    if (createSuccess) {
        wchar_t buffer[1024];
        size_t ret;
        mbstowcs_s(&ret, buffer, 1024, c3, strlen(c3));
        UTF8toWChar_t(buffer, c2);
        NUI_SPEECH_SEMANTIC s;
        HRESULT res = NuiSpeechAddWordTransition(
            &grammar.mGrammar,
            *fromState,
            toState ? *toState : nullptr,
            buffer,
            nullptr,
            NUI_SPEECH_WORDTYPE_LEXICAL,
            1,
            &s
        );
        bool transitionSuccess = SUCCEEDED(res);
        if (!transitionSuccess) {
            MILO_NOTIFY(
                "NuiSpeechAddWordTransition failed with error 0x%08x on %s %s", res, c2, c3
            );
        }
    }
}

void SpeechMgr::Poll() {
    if (!mEnabled) {
        mOverlay->CurrentLine() = "speech is not enabled (no Kinect?)";
    } else {
        ULONG fetched = 0;
        NUI_SPEECH_EVENT event[5];
        memset(&event[0], 0, sizeof(event));
        HRESULT res = NuiSpeechGetEvents(5, event, &fetched);
        if (SUCCEEDED(res) && fetched != 0) {
            for (ULONG i = 0; i < fetched; i++) {
                NUI_SPEECH_EVENT &cur = event[i];
                if (cur.eventId > 0x20 && (cur.eventId == 0x40 || cur.eventId == 0x80)
                    && cur.pResult) {
                    ProcessRecoResult(cur.pResult);
                }
                NuiSpeechDestroyEvent(&cur);
            }
        }
    }
}

void SpeechMgr::ProcessRecoResult(NUI_SPEECH_RECORESULT *result) {
    MILO_ASSERT(result, 0x318);
    NUI_SPEECH_RULERESULT &rule = result->Phrase.Rule;
    NUI_SPEECH_SEMANTICRESULT *sr = result->Phrase.pSemanticProperties;
    Symbol s94(rule.pcwszName ? WstrToANSI(rule.pcwszName) : gNullStr);
    if (mOverlay->Showing()) {
        float phraseConfidence;
        if (sr) {
            if (sr->fSREngineConfidence < 0.01)
                goto next;
            else {
                phraseConfidence = sr->fSREngineConfidence;
            }
        } else {
            phraseConfidence = 0;
        }
        mOverlay->Print(MakeString(
            "Recognized \"%s\" phrase with %.2f phrase confidence, %.2f rule confidence:\n",
            rule.pcwszName ? WstrToANSI(rule.pcwszName) : gNullStr,
            phraseConfidence,
            rule.fSREngineConfidence
        ));
        mOverlay->Print("  heard: ");
        for (int i = 0; i < result->Phrase.ulCountOfElements; i++) {
            mOverlay->Print(MakeString(
                "%s, ", WstrToANSI(result->Phrase.pElements[i].pcwszDisplayText)
            ));
        }
        mOverlay->Print("\n");
        if (!sr)
            return;
        mOverlay->Print("  semantic tree:\n");
        PrintSemanticTree(sr, 2);
    }
next:
    if (sr) {
        float initialEngineConfidence = sr->fSREngineConfidence;
        DataArrayPtr ptr(new DataArray(0));
        do {
            ptr->Insert(ptr->Size(), Symbol(WstrToANSI(sr->pcwszValue)));
            sr = sr->pNextSibling;
        } while (sr);
        static SpeechRecoMessage msg(DataArrayPtr(nullptr), 0.0f, Symbol(gNullStr));
        msg[0] = ptr;
        msg[1] = initialEngineConfidence;
        msg[2] = s94;
        Export(msg, false);
    }
}

#pragma endregion
