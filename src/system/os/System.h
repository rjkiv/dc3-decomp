#pragma once
#include "obj/Data.h"
#include "os/Platform.h"
#include "utl/Symbol.h"

#define kCommandLineSz 0x200

int Hx_snprintf(char *, unsigned int, char const *, ...);

extern const char *gNullStr;
extern std::vector<char *> TheSystemArgs;
extern const char *gHostFile;
extern bool gHostCached;
extern bool gHostConfig;
extern bool gHostLogging;

enum GfxMode {
    kOldGfx = 0,
    kNewGfx = 1,
};

struct StackData {
    /** Addresses in memory corresponding to called functions. */
    unsigned int mFailThreadStack[50]; // 0x0
};

class GenericMapFile {
public:
    static bool ParseStack(
        const char *mapName, struct StackData *data, int stackIdx, FixedString &outputStr
    );
};

void SetGfxMode(GfxMode mode);
GfxMode GetGfxMode();

inline bool IsVertexCompressionSupported(Platform p) {
    return p == kPlatformXBox || p == kPlatformPS3;
}

bool UsingCD();
void SetUsingCD(bool usingCD);

DataArray *SystemConfig();
DataArray *SystemConfig(Symbol tag);
DataArray *SystemConfig(Symbol tag1, Symbol tag2);
DataArray *SystemConfig(Symbol tag1, Symbol tag2, Symbol tag3);
DataArray *SystemConfig(Symbol tag1, Symbol tag2, Symbol tag3, Symbol tag4);
DataArray *SystemConfig(Symbol tag1, Symbol tag2, Symbol tag3, Symbol tag4, Symbol tag5);

Symbol PlatformSymbol(Platform platform);
bool PlatformLittleEndian(Platform platform);
Platform ConsolePlatform();

Symbol SystemLanguage();
Symbol SystemLocale();
DataArray *SystemTitles();
Symbol GetSongTitlePronunciationLanguage();

int SystemExec(const char *args);
int SystemMs();
void SystemPoll(bool);

bool HongKongExceptionMet();
Symbol GetSystemLanguage(Symbol langSym);
Symbol GetSystemLocale(Symbol locSym);

DataArray *ReadSystemConfig(const char *cfg);
void StripEditorData();

DataArray *SupportedLanguages(bool cheats);
bool IsSupportedLanguage(Symbol language, bool cheats);
void SetSystemLanguage(Symbol language, bool cheats);

void AppendStackTrace(FixedString &, void *);
void AppendThreadStackTrace(FixedString &, struct StackData *);

void LanguageInit();
void InitSystem(const char *cfg);
void NormalizeSystemArgs();
void PreInitSystem(const char *cfg);
void SetSystemArgs(const char *commandLine);
void SystemInit(const char *cfg);
void SystemTerminate();
void SystemPreInit(const char *cfg);
void SystemPreInit(const char *commandLine, const char *cfg);
void SystemPreInit(int, char **const, const char *cfg);

unsigned long ULSystemLocale();
unsigned long ULSystemLanguage();

typedef void DiscErrorCallbackFunc(void);

DiscErrorCallbackFunc *SetDiskErrorCallback(DiscErrorCallbackFunc *func);
DiscErrorCallbackFunc *GetDiskErrorCallback();

bool PlatformDebugBreak();

void GetMapFileName(String &filename);
void ShowDirtyDiscError();
void CaptureStackTrace(int, struct StackData *, void *);
