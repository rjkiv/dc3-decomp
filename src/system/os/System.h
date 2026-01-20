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
    static bool ParseStack(const char *, struct StackData *, int, FixedString &);
};

void SetGfxMode(GfxMode);
GfxMode GetGfxMode();

bool UsingCD();
void SetUsingCD(bool);

DataArray *SystemConfig();
DataArray *SystemConfig(Symbol);
DataArray *SystemConfig(Symbol, Symbol);
DataArray *SystemConfig(Symbol, Symbol, Symbol);
DataArray *SystemConfig(Symbol, Symbol, Symbol, Symbol);
DataArray *SystemConfig(Symbol, Symbol, Symbol, Symbol, Symbol);

Symbol PlatformSymbol(Platform);
bool PlatformLittleEndian(Platform);
Platform ConsolePlatform();

Symbol SystemLanguage();
Symbol SystemLocale();
DataArray *SystemTitles();
Symbol GetSongTitlePronunciationLanguage();

int SystemExec(const char *);
int SystemMs();
void SystemPoll();

bool HongKongExceptionMet();
Symbol GetSystemLanguage(Symbol);
Symbol GetSystemLocale(Symbol);

DataArray *ReadSystemConfig(const char *);
void StripEditorData();

DataArray *SupportedLanguages(bool);
bool IsSupportedLanguage(Symbol, bool);
void SetSystemLanguage(Symbol, bool);

void AppendStackTrace(FixedString &, void *);
void AppendThreadStackTrace(FixedString &, struct StackData *);

void LanguageInit();
void InitSystem(const char *);
void NormalizeSystemArgs();
void PreInitSystem(const char *);
void SetSystemArgs(const char *);
void SystemInit(const char *);
void SystemTerminate();
void SystemPreInit(const char *);
void SystemPreInit(const char *, const char *);
void SystemPreInit(int, char **const, const char *);

unsigned long ULSystemLocale();
unsigned long ULSystemLanguage();

typedef void DiscErrorCallbackFunc(void);

DiscErrorCallbackFunc *SetDiskErrorCallback(DiscErrorCallbackFunc *func);
DiscErrorCallbackFunc *GetDiskErrorCallback();

bool PlatformDebugBreak();

void GetMapFileName(String &);
void ShowDirtyDiscError();
void CaptureStackTrace(int, struct StackData *, void *);
