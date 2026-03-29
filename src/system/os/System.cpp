#include "os/System.h"
#include "HolmesClient.h"
#include "math/FileChecksum.h"
#include "math/Geo.h"
#include "math/Rand.h"
#include "math/Trig.h"
#include "net/WebSvcMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Task.h"
#include "os/AppChild.h"
#include "os/Archive.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/FileCache.h"
#include "os/Joypad.h"
#include "os/JoypadClient.h"
#include "os/Keyboard.h"
#include "os/MapFile_Xbox.h"
#include "os/Memcard_Xbox.h"
#include "os/Platform.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"
#include "os/Timer.h"
#include "os/VirtualKeyboard.h"
#include "utl/CacheMgr.h"
#include "utl/Cheats.h"
#include "utl/DataPointMgr.h"
#include "utl/GlitchFinder.h"
#include "utl/Licenses.h"
#include "utl/Loader.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "utl/NetCacheMgr.h"
#include "utl/Option.h"
#include "utl/Spew.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"
#include "xdk/XAPILIB.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

const char *gNullStr = "";

static GfxMode gGfxMode;

namespace {
    bool gPreconfigOverride = false;
}

bool gHostConfig = false;
bool gHostLogging = false;
bool gHostCached = false;

static DataArray *gSystemConfig;
static DataArray *gSystemTitles;
static int gUsingCD;
static int gSystemMs;
static float gSystemFrac;
const char *gHostFile;
static Symbol gSystemLanguage;

std::vector<char *> TheSystemArgs;

static Symbol gSystemLocale;
static std::vector<char *> gPristineSystemArgs;
static Timer gSystemTimer;

bool gNetUseTimedSleep;
bool(__cdecl *ParseStack)(char const *, struct StackData *, int, class FixedString &) =
    XboxMapFile::ParseStack;

namespace {
    bool gHasPreconfig = true;

    void CheckForArchive() {
        gUsingCD = true;
        FileStat buffer;
        Symbol plat = PlatformSymbol(TheLoadMgr.GetPlatform());
        int ret = FileGetStat(MakeString("gen/main_%s.hdr", plat), &buffer);
        gUsingCD &= ret;
    }
}

Licenses sLicense("system/src/stlport", Licenses::kRequirementNotification);

int Hx_snprintf(char *c, unsigned int ui, char const *cc, ...) {
    std::va_list args;
    // va_start(args, cc);
    int ret = vsnprintf(c, ui, cc, args);
    if (ret < 0) {
        c[ui - 1] = '\0';
        return -1;
    }
    return ret;
}

GfxMode GetGfxMode() { return gGfxMode; }

Symbol PlatformSymbol(Platform pform) {
    static Symbol sym[] = { gNullStr, gNullStr, "xbox", "pc", "ps3", "wii", "3ds" };
    if (pform >= 0 && pform < 7) {
        return sym[pform];
    } else
        return gNullStr;
}

bool UsingCD() { return gUsingCD; }
void SetUsingCD(bool b) { gUsingCD = b; }

DataArray *SystemConfig() { return gSystemConfig; }

DataArray *SystemConfig(Symbol s) { return gSystemConfig->FindArray(s); }

DataArray *SystemConfig(Symbol s1, Symbol s2) {
    return gSystemConfig->FindArray(s1)->FindArray(s2);
}
DataArray *SystemConfig(Symbol s1, Symbol s2, Symbol s3) {
    return gSystemConfig->FindArray(s1)->FindArray(s2)->FindArray(s3);
}

DataArray *SystemConfig(Symbol s1, Symbol s2, Symbol s3, Symbol s4) {
    return gSystemConfig->FindArray(s1)->FindArray(s2)->FindArray(s3)->FindArray(s4);
}

DataArray *SystemConfig(Symbol s1, Symbol s2, Symbol s3, Symbol s4, Symbol s5) {
    return gSystemConfig->FindArray(s1)
        ->FindArray(s2)
        ->FindArray(s3)
        ->FindArray(s4)
        ->FindArray(s5);
}

Symbol SystemLanguage() { return gSystemLanguage; }
Symbol SystemLocale() { return gSystemLocale; }

DataArray *SystemTitles() { return gSystemTitles; }

Symbol GetSongTitlePronunciationLanguage() {
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    static Symbol fre("fre");
    static Symbol frc("frc");
    static Symbol can("can");
    if (lang == fre && gSystemLocale == can) {
        lang = frc;
    }
    return lang;
}

int SystemExec(const char *args) {
    if (gUsingCD)
        return -1;
    else
        return HolmesClientSysExec(args);
}

bool PlatformLittleEndian(Platform p) {
    MILO_ASSERT(p != kPlatformNone, 0x175);
    return p == kPlatformPC || p == kPlatform3DS || p == kPlatformNone;
}

Platform ConsolePlatform() { return kPlatformXBox; }

DataArray *ReadSystemConfig(const char *config) {
    Timer timer;
    timer.Start();
    DataArray *cfgArr = DataReadFile(config, true);
    timer.Stop();
    MILO_LOG("reading system config file \"%s\" took %f ms\n", config, timer.Ms());
    return cfgArr;
}

void StripEditorData() {
    Symbol editor("editor");
    Symbol types("types");
    DataArray *objectsCfg = SystemConfig("objects");
    for (int i = 1; i < objectsCfg->Size(); i++) {
        DataArray *objectsArr = objectsCfg->Array(i);
        DataArray *objEditorArr = objectsArr->FindArray(editor, false);
        if (objEditorArr != 0)
            objEditorArr->Resize(1);
        DataArray *typesArr = objectsArr->FindArray(types, false);
        if (typesArr != 0) {
            for (int j = 1; j < typesArr->Size(); j++) {
                DataArray *typesEditorArr = typesArr->Array(j)->FindArray(editor, false);
                if (typesEditorArr != 0)
                    typesEditorArr->Resize(1);
            }
        }
    }
}

int SystemMs() {
    gSystemTimer.Restart();
    float lastMs = gSystemTimer.GetLastMs();
    int ms = gSystemFrac + lastMs;
    gSystemFrac = (gSystemFrac + lastMs) - ms;
    gSystemMs += ms;
    return gSystemMs;
}

void SystemPoll(bool b1) {
    static Timer *_t = AutoTimer ::GetTimer("system_poll");
    AutoTimer _at(_t, 50.0f, nullptr, nullptr);
    Timer::ClearSlowFrame();
    SystemMs();
    TheDebug.Poll();
    TheMC.Poll();
    if (gUsingCD == 0) {
        HolmesClientPoll();
    }
    JoypadPoll();
    JoypadClientPoll();
    KeyboardPoll();
    ThreadCallPoll();
    FileCache::PollAll();
    TheLoadMgr.Poll();
    TheCacheMgr->Poll();
    TheNetCacheMgr->Poll();
    TheWebSvcMgr.Poll();
    if (TheAppChild) {
        TheAppChild->Poll();
    }
    if (b1) {
        TheTaskMgr.Poll();
    }
    ThePlatformMgr.Poll();
    TheVirtualKeyboard.Poll();
    TheContentMgr.PollRefresh();
}

DataArray *SupportedLanguages(bool cheats) {
    static Symbol system("system");
    static Symbol language("language");
    static Symbol supported("supported");
    static Symbol cheat_supported("cheat_supported");
    return SystemConfig(system, language, cheats ? cheat_supported : supported)->Array(1);
}

bool IsSupportedLanguage(Symbol language, bool cheats) {
    DataArray *languages = SupportedLanguages(cheats);
    for (int i = 0; i < languages->Size(); i++) {
        if (languages->Sym(i) == language)
            return true;
    }
    return false;
}

void SetSystemLanguage(Symbol lang, bool cheats) {
    if (!IsSupportedLanguage(lang, cheats)) {
        static Symbol system("system");
        static Symbol language("language");
        static Symbol defaultSym("default");
        DataArray *cfg = SystemConfig(system, language);
        DataArray *arr = cfg->FindArray(defaultSym, false);
        if (arr) {
            Symbol arrLang = arr->Sym(1);
            if (IsSupportedLanguage(arrLang, cheats)) {
                if (!gSystemLanguage.Null() && arrLang != gSystemLanguage) {
                    TheLocale.Terminate();
                    gSystemLanguage = arrLang;
                    TheLocale.Init();
                }
            } else {
                MILO_NOTIFY(
                    "Both %s and the default language (%s) are not supported!\n",
                    lang,
                    arrLang
                );
            }
        } else {
            MILO_NOTIFY(
                "Language %s is not supported, and there is no default language found!\n",
                lang
            );
        }
    } else if (!gSystemLanguage.Null() && lang != gSystemLanguage) {
        TheLocale.Terminate();
        gSystemLanguage = lang;
        TheLocale.Init();
    }
}

void SetGfxMode(GfxMode mode) {
    gGfxMode = mode;
    HolmesClientReInit();
    DataVariable("gfx_mode") = mode;
}

DataNode OnSystemLanguage(DataArray *) { return gSystemLanguage; }
DataNode OnSystemLocale(DataArray *) { return gSystemLocale; }
DataNode OnSystemExec(DataArray *a) { return SystemExec(a->Str(1)); }
DataNode OnUsingCD(DataArray *) { return UsingCD(); }
DataNode OnSupportedLanguages(DataArray *) { return SupportedLanguages(false); }
DataNode OnSystemMs(DataArray *) { return SystemMs(); }

DataNode OnSwitchSystemLanguage(DataArray *a) {
    DataArray *langs = SupportedLanguages(true);
    int i;
    for (i = 0; i < langs->Size(); i++) {
        if (gSystemLanguage == langs->Sym(i)) {
            break;
        }
    }
    SetSystemLanguage(langs->Sym((i + 1) % langs->Size()), true);
    return 1;
}

void LanguageInit() {
    if (ThePlatformMgr.GetRegion() == kRegionNone) {
        MILO_NOTIFY("LanguageInit called, but region has not been initialized");
    }
    DataArray *cfg = SystemConfig("system", "language");
    Symbol lang = GetSystemLanguage("eng");
    DataArray *remapArr = cfg->FindArray("remap", false);
    if (remapArr) {
        remapArr->FindData(lang, lang, false);
    }
    Symbol forceSym;
    if (cfg->FindData("force", forceSym, false)) {
        if (forceSym != "") {
            lang = forceSym;
        }
    }
    const char *str = OptionStr("lang", nullptr);
    if (str) {
        lang = str;
    }
    SetSystemLanguage(lang, false);
}

void AppendStackTrace(FixedString &str, void *v) {
    StackData data;
    memset(&data, 0, sizeof(StackData));
    CaptureStackTrace(50, &data, v);
    int stackIdx;
    for (stackIdx = 0; stackIdx < 50; stackIdx++) {
        if (data.mFailThreadStack[stackIdx] == 0)
            break;
    }
    String mapName;
    GetMapFileName(mapName);
    str += "Stack Trace: \r\n";
    bool parse;
    if (!UsingCD() && !FileIsLocal(mapName.c_str())) {
        String strf8;
        HolmesClientStackTrace(mapName.c_str(), &data, stackIdx, strf8);
        str += strf8.c_str();
        parse = !strf8.empty();
    } else if (TheArchive && TheArchive->Patched()) {
        parse = false;
    } else {
        parse = (*ParseStack)(mapName.c_str(), &data, stackIdx, str);
    }
    if (!parse) {
        GenericMapFile::ParseStack(mapName.c_str(), &data, stackIdx, str);
    }
    str += "\r\n";
}

void AppendThreadStackTrace(FixedString &str, StackData *stack) {
    str += "\n\n-- Thread failure, no stack yet --";
    int idx;
    for (idx = 0; idx < 50; idx++) {
        if (stack->mFailThreadStack[idx] == 0)
            break;
    }
    GenericMapFile::ParseStack(nullptr, stack, idx, str);
}

bool GenericMapFile::ParseStack(
    const char *cc, struct StackData *stack, int stackIdx, FixedString &str
) {
    str += " (map file unavailable)";
    for (int i = 0; i < stackIdx; i++) {
        str += MakeString("\n   %08x", stack->mFailThreadStack[i]);
    }
    return true;
}

void InitSystem(const char *config) {
    if (!gPreconfigOverride && config) {
        bool oldCD = UsingCD();
        Archive *oldArchive = TheArchive;
        if (gHostConfig) {
            gUsingCD = false;
            TheArchive = nullptr;
        }
        DataArray *systemConfig = ReadSystemConfig(config);
        MILO_ASSERT(systemConfig, 0x267);
        DataMergeTags(systemConfig, gSystemConfig);
        DataReplaceTags(systemConfig, gSystemConfig);
        gSystemConfig->Release();
        gSystemConfig = systemConfig;
        DataVariable("syscfg") = gSystemConfig;
        gUsingCD = oldCD;
        TheArchive = oldArchive;
        StripEditorData();
    }
    FinishDataRead();
}

void NormalizeSystemArgs() {
    for (int i = 0; i < TheSystemArgs.size(); i++) {
        char *cur = TheSystemArgs[i];
        for (char *p = cur; *p != '\0'; p++) {
            if (*p == -0x6A) {
                *p = '-';
            }
            if (*p == -0x6D || *p == -0x6C) {
                *p = '\"';
            }
        }
    }
}

void PreInitSystem(const char *config) {
    Archive *oldArchive = TheArchive;
    bool oldCD = UsingCD();
    if (gHostConfig) {
        gUsingCD = false;
        TheArchive = nullptr;
    }
    DataArrayPtr ptr(1);
    DataSetMacro("HX_XBOX", ptr);
    DataSetMacro("HX_WIN", ptr);
    DataSetMacro("HX_NG", ptr);
    while (true) {
        const char *str = OptionStr("define", nullptr);
        if (!str)
            break;
        DataSetMacro(str, ptr);
    }
    const char *cfgStr = OptionStr("config", nullptr);
    if (cfgStr && !gHasPreconfig) {
        config = cfgStr;
    }
    BeginDataRead();
    gSystemConfig = ReadSystemConfig(config);
    MILO_ASSERT(gSystemConfig, 0x1FF);
    DataVariable("syscfg") = gSystemConfig;
    gUsingCD = oldCD;
    TheArchive = oldArchive;
    DataRegisterFunc("system_language", OnSystemLanguage);
    DataRegisterFunc("system_locale", OnSystemLocale);
    DataRegisterFunc("system_exec", OnSystemExec);
    DataRegisterFunc("using_cd", OnUsingCD);
    DataRegisterFunc("supported_languages", OnSupportedLanguages);
    DataRegisterFunc("switch_system_language", OnSwitchSystemLanguage);
    DataRegisterFunc("system_ms", OnSystemMs);
    SetGfxMode(kNewGfx);
    if (cfgStr && gHasPreconfig) {
        InitSystem(cfgStr);
        gPreconfigOverride = true;
    }
}

void SystemInit(const char *config) {
    if (OptionBool("force_cd", false)) {
        MILO_FAIL("force_cd is deprecated in favor of no_cd");
    }
    gSystemTimer.Start();
    Symbol::Init();
    InitSystem(config);
    gSystemTitles = SystemConfig("system", "titles");
    ObjectDir::Init();
    TrigTableInit();
    ThreadCallInit();
    GeoInit();
    TrigInit();
    SpewInit();
    TheLocale.Terminate();
    TheLocale.Init();
    CheatsInit();
    TheMC.Init();
    FileCache::Init();
    CacheMgrInit();
    NetCacheMgrInit();
    TheDataPointMgr.Init();
    TheWebSvcMgr.Init();
    ThePlatformMgr.Init();
    TheVirtualKeyboard.Init();
    TheContentMgr.Init();
    GlitchFinder::Init();
    TheDebug.AddExitCallback(SystemTerminate);
    if (OptionBool("licenses", false)) {
        Licenses::PrintAll();
        TheDebug.Exit(0, true);
    }
}

void SetSystemArgs(const char *commandLine) {
    MILO_ASSERT(commandLine && strlen(commandLine) < kCommandLineSz, 0x39A);
    static char buffer[512];
    strncpy(buffer, commandLine, sizeof(buffer) - 1);
    char *arg = buffer;
    buffer[sizeof(buffer) - 1] = '\0';
    bool b2 = true;
    bool b4 = false;
    for (char *p = buffer; *p != '\0'; p++) {
        if (!b4 && *p == ' ') {
            *p = '\0';
            b4 = true;
            arg = p + 1;
        } else {
            if (*p != '\"') {
                if (b2) {
                    TheSystemArgs.push_back(arg);
                    b2 = false;
                }
                arg = p + 1;
            } else {
                *p = '\0';
                arg = p + 1;
                b4 = !b4;
                if (!b4) {
                    b2 = true;
                } else {
                    TheSystemArgs.push_back(arg);
                    b2 = false;
                }
            }
        }
    }
    NormalizeSystemArgs();
    gPristineSystemArgs = TheSystemArgs;
}

void SystemPreInit(const char *config) {
    InitMakeString();
    Symbol::PreInit(640000, 80000);
    ThePlatformMgr.RegionInit();
    ThePlatformMgr.PreInit();
    if (!OptionBool("no_cd", false)) {
        CheckForArchive();
    }
    OptionInit();
    if (OptionBool("no_checksum", false)) {
        ClearFileChecksumData();
    }
    TimeConversionInit();
    Timer::Init();
    gHostConfig = OptionBool("host_config", false);
    gHostLogging = OptionBool("host_logging", false);
    gHostFile = OptionStr("host_file", nullptr);
    if (gHostFile) {
        gHostConfig = true;
    }
    gHostCached = OptionBool("host_cached", false);
    FileInit();
    AppChild::Init();
    DateTimeInit();
    SYSTEMTIME time;
    GetLocalTime(&time);
    SeedRand(time.wMilliseconds);
    TheContentMgr.PreInit();
    ArchiveInit();
    TheDebug.Init();
    String str;
    for (int i = 0; i < gPristineSystemArgs.size(); i++) {
        str += ' ';
        str += gPristineSystemArgs[i];
    }
    gPristineSystemArgs.reserve(0);
    MILO_LOG("SystemInit Params:%s\n", str);
    DataInit();
    PreInitSystem(config);
    LanguageInit();
    gSystemLocale = GetSystemLocale("usa");
    MemInit();
    TheLoadMgr.Init();
    JoypadInit();
    KeyboardInit();
    AutoTimer::Init();
    ThreadCallPreInit();
    TheTaskMgr.Init();
}

void SystemPreInit(const char *cmdLine, const char *cfg) {
    SetSystemArgs(cmdLine);
    SystemPreInit(cfg);
}

void SystemTerminate() {
    TheDebug.RemoveExitCallback(SystemTerminate);
    SpewTerminate(); // some other unknown stub Terminate func goes here
    TheVirtualKeyboard.Terminate();
    CacheMgrTerminate();
    NetCacheMgrTerminate();
    FileCache::Terminate();
    TheLocale.Terminate();
    TheMC.Terminate();
    CheatsTerminate();
    KeyboardTerminate();
    JoypadTerminate();
    SpewTerminate();
    ThreadCallTerminate();
    TheTaskMgr.Terminate();
    ObjectDir::Terminate();
    TheContentMgr.Terminate();
    TrigTableTerminate();
    gSystemConfig->Release();
    DataTerminate();
    Symbol::Terminate();
    AppChild::Terminate();
    TheSystemArgs.clear();
    TerminateMakeString();
}
