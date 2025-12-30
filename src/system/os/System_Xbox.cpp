#include "obj/Data.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"
#include "xdk/XBDM.h"
#include "Memory.h"

namespace {
    DiscErrorCallbackFunc *gCallback;
}

unsigned long ULSystemLocale() { return XGetLocale(); }
unsigned long ULSystemLanguage() { return XTLGetLanguage(); }

DiscErrorCallbackFunc *SetDiskErrorCallback(DiscErrorCallbackFunc *func) {
    DiscErrorCallbackFunc *old = gCallback;
    gCallback = func;
    return old;
}

DiscErrorCallbackFunc *GetDiskErrorCallback() { return gCallback; }

#define SUCCEEDED(hr) hr >= 0

namespace {
    void XDKCheck() {
        DM_SYSTEM_INFO info;
        info.SizeOfStruct = 0x20;
        int hr = DmGetSystemInfo(&info);
        MILO_ASSERT(SUCCEEDED(hr), 0x27);
        if (info.XDKVersion.Build < 21173) {
            MILO_NOTIFY(
                "Console firmware is out of date.  Console: %d  Binary: %d",
                info.XDKVersion.Build,
                21173
            );
        }
    }
}

Symbol GetSystemLanguage(Symbol s) {
    static Symbol eng("eng");
    static Symbol fre("fre");
    static Symbol ita("ita");
    static Symbol deu("deu");
    static Symbol esl("esl");
    static Symbol mex("mex");
    static Symbol swe("swe");
    static Symbol pol("pol");
    static Symbol nor("nor");
    static Symbol fin("fin");
    static Symbol dut("dut");
    static Symbol dan("dan");
    static Symbol ptb("ptb");
    static Symbol rus("rus");
    static Symbol cht("cht");
    static Symbol kor("kor");
    static Symbol jpn("jpn");

    unsigned long lang = ULSystemLanguage();
    unsigned long locale = ULSystemLocale();

    switch (locale) {
    case XC_LOCALE_SWEDEN:
        if (IsSupportedLanguage(swe, false))
            s = swe;
        break;
    case XC_LOCALE_NORWAY:
        if (IsSupportedLanguage(nor, false))
            s = nor;
        break;
    case XC_LOCALE_NETHERLANDS:
        if (IsSupportedLanguage(dut, false))
            s = dut;
        break;
    case XC_LOCALE_FINLAND:
        if (IsSupportedLanguage(fin, false))
            s = fin;
        break;
    case XC_LOCALE_DENMARK:
        if (IsSupportedLanguage(dan, false))
            s = dan;
    default:
        break;
    }

    switch (lang) {
    case XC_LANGUAGE_ENGLISH:
        if (locale == XC_LOCALE_BELGIUM && IsSupportedLanguage(dut, false)) {
            s = dut;
        }
        break;
    case XC_LANGUAGE_SCHINESE:
        s = eng;
        break;
    case XC_LANGUAGE_JAPANESE:
        s = jpn;
        break;
    case XC_LANGUAGE_GERMAN:
        s = deu;
        break;
    case XC_LANGUAGE_FRENCH:
        s = fre;
        break;
    case XC_LANGUAGE_SPANISH:
        if (locale == XC_LOCALE_CHILE || locale == XC_LOCALE_COLOMBIA
            || locale == XC_LOCALE_MEXICO) {
            if (IsSupportedLanguage(mex, false))
                s = mex;
            else
                s = esl;
        }
        break;
    case XC_LANGUAGE_ITALIAN:
        s = ita;
        break;
    case XC_LANGUAGE_KOREAN:
        s = kor;
        break;
    case XC_LANGUAGE_TCHINESE:
        s = cht;
        break;
    case XC_LANGUAGE_PORTUGUESE:
        s = ptb;
        break;
    case XC_LANGUAGE_POLISH:
        s = pol;
        break;
    case XC_LANGUAGE_RUSSIAN:
        s = rus;
    default:
        break;
    }

    return s;
}

Symbol GetSystemLocale(Symbol s) {
    static Symbol aus("aus");
    static Symbol aut("aut");
    static Symbol bel("bel");
    static Symbol bra("bra");
    static Symbol can("can");
    static Symbol chi("chi");
    static Symbol chn("chn");
    static Symbol col("col");
    static Symbol cze("cze");
    static Symbol den("den");
    static Symbol esp("esp");
    static Symbol fin("fin");
    static Symbol fra("fra");
    static Symbol gbr("gbr");
    static Symbol ger("ger");
    static Symbol gre("gre");
    static Symbol hkg("hkg");
    static Symbol hun("hun");
    static Symbol ind("ind");
    static Symbol irl("irl");
    static Symbol ita("ita");
    static Symbol jpn("jpn");
    static Symbol kor("kor");
    static Symbol mex("mex");
    static Symbol ned("ned");
    static Symbol nor("nor");
    static Symbol nzl("nzl");
    static Symbol pol("pol");
    static Symbol por("por");
    static Symbol rsa("rsa");
    static Symbol rus("rus");
    static Symbol sin("sin");
    static Symbol svk("svk");
    static Symbol swe("swe");
    static Symbol sui("sui");
    static Symbol tpe("tpe");
    static Symbol usa("usa");
    switch (ULSystemLocale()) {
    case XC_LOCALE_AUSTRALIA:
        return aus;
    case XC_LOCALE_AUSTRIA:
        return aut;
    case XC_LOCALE_BELGIUM:
        return bel;
    case XC_LOCALE_BRAZIL:
        return bra;
    case XC_LOCALE_CANADA:
        return can;
    case XC_LOCALE_CHILE:
        return chi;
    case XC_LOCALE_CHINA:
        return chn;
    case XC_LOCALE_COLOMBIA:
        return col;
    case XC_LOCALE_CZECH_REPUBLIC:
        return cze;
    case XC_LOCALE_DENMARK:
        return den;
    case XC_LOCALE_FINLAND:
        return fin;
    case XC_LOCALE_FRANCE:
        return fra;
    case XC_LOCALE_GERMANY:
        return ger;
    case XC_LOCALE_GREECE:
        return gre;
    case XC_LOCALE_HONG_KONG:
        return hkg;
    case XC_LOCALE_HUNGARY:
        return hun;
    case XC_LOCALE_INDIA:
        return ind;
    case XC_LOCALE_IRELAND:
        return irl;
    case XC_LOCALE_ITALY:
        return ita;
    case XC_LOCALE_JAPAN:
        return jpn;
    case XC_LOCALE_KOREA:
        return kor;
    case XC_LOCALE_MEXICO:
        return mex;
    case XC_LOCALE_NETHERLANDS:
        return ned;
    case XC_LOCALE_NEW_ZEALAND:
        return nzl;
    case XC_LOCALE_NORWAY:
        return nor;
    case XC_LOCALE_POLAND:
        return pol;
    case XC_LOCALE_PORTUGAL:
        return por;
    case XC_LOCALE_SINGAPORE:
        return sin;
    case XC_LOCALE_SLOVAK_REPUBLIC:
        return svk;
    case XC_LOCALE_SOUTH_AFRICA:
        return rsa;
    case XC_LOCALE_SPAIN:
        return esp;
    case XC_LOCALE_SWEDEN:
        return swe;
    case XC_LOCALE_SWITZERLAND:
        return sui;
    case XC_LOCALE_TAIWAN:
        return tpe;
    case XC_LOCALE_GREAT_BRITAIN:
        return gbr;
    case XC_LOCALE_UNITED_STATES:
        return usa;
    case XC_LOCALE_RUSSIAN_FEDERATION:
        return rus;
    default:
        return s;
    }
}

bool HongKongExceptionMet() {
    if (ULSystemLanguage() == XC_LANGUAGE_TCHINESE
        && ULSystemLocale() == XC_LOCALE_HONG_KONG) {
        return true;
    } else
        return false;
}

void GetMapFileName(String &filename) {
    if (SystemConfig()) {
        const char *mapVersion = "r";
        DataArray *cfg = SystemConfig("system", "xbox_map_file");
        FileQualifiedFilename(
            filename, MakeString(cfg->Str(1), FileExecRoot(), mapVersion)
        );
    } else {
        char name[256];
        strcpy(name, FileGetName(TheSystemArgs.front()));
        char *rchar = strrchr(name, '.');
        if (rchar) {
            strcpy(rchar, ".map");
        }
        filename = name;
        FileQualifiedFilename(filename, name);
    }
}

void SystemPreInit(int, char **const, const char *c3) {
    SystemPreInit(GetCommandLineA(), c3);
    XDKCheck();
    ForceLinkXMemFuncs();
}

bool PlatformDebugBreak() {
    if (DmIsDebuggerPresent()) {
        DebugBreak();
        return true;
    }
    return false;
}

void ShowDirtyDiscError() {
    unsigned long ul;

    if (ThePlatformMgr.sXShowCallback(ul)) {
        XShowNuiDirtyDiscErrorUI(ul, 0);
    }

    XShowDirtyDiscErrorUI(0);
}

void CaptureStackTrace(int p1, struct StackData *stackData, void *p3) {
    stackData->mFailThreadStack[0] = 0;

    DmCaptureStackBackTrace(p1, stackData);

    memmove(stackData->mFailThreadStack, stackData->mFailThreadStack + 3, (p1 + -3) * 4);

    if (p3 != 0) {
        memmove(
            stackData->mFailThreadStack + 2,
            stackData->mFailThreadStack + 8,
            (p1 + -8) * 4
        );
    }
}
