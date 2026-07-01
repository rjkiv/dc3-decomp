#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Locale.h"
#include "utl/LocaleOrdinal.h"
#include "xdk/XAPILIB.h"

void GetDateAndTime(DateTime &dt) {
    SYSTEMTIME time;
    GetLocalTime(&time);
    dt.mYear = time.wYear - 108;
    dt.mMonth = time.wMonth - 1;
    dt.mDay = time.wDay;
    dt.mHour = time.wHour;
    dt.mMin = time.wMinute;
    dt.mSec = time.wSecond;
}

void GetSystemDateAndTime(DateTime &dt) {
    SYSTEMTIME time;
    GetSystemTime(&time);
    dt.mYear = time.wYear - 108;
    dt.mMonth = time.wMonth - 1;
    dt.mDay = time.wDay;
    dt.mHour = time.wHour;
    dt.mMin = time.wMinute;
    dt.mSec = time.wSecond;
}

void GetTimeZoneBias(long &bias) {
    TIME_ZONE_INFORMATION info;
    bias = 0;
    if (GetTimeZoneInformation(&info) != -1) {
        bias = info.Bias;
    }
}

#define SECS_PER_MIN 60
#define SECS_PER_HOUR 3600
#define SECS_PER_DAY 86400
// approximate, secs in 32 days
#define SECS_PER_MONTH 2764800
// approximate, secs in 384 days
#define SECS_PER_YEAR 33177600

DateTime::DateTime(unsigned int code) {
    mYear = (code / SECS_PER_YEAR) + 100;
    mMonth = (code % SECS_PER_YEAR) / SECS_PER_MONTH;
    unsigned char u2 = (code % SECS_PER_YEAR) % SECS_PER_MONTH;
    mDay = u2 / SECS_PER_DAY;
    u2 = u2 % SECS_PER_DAY;
    mHour = u2 / SECS_PER_HOUR;
    u2 = u2 % SECS_PER_HOUR;
    mMin = u2 / SECS_PER_MIN;
    mSec = u2 - mMin * SECS_PER_MIN;
}

DateTime::DateTime(
    unsigned short year,
    unsigned char month,
    unsigned char day,
    unsigned char hr,
    unsigned char min,
    unsigned char sec
) {
    mYear = year - 1900;
    mMonth = month - 1;
    mDay = day;
    mHour = hr;
    mMin = min;
    mSec = sec;
}

unsigned int DateTime::ToCode() const {
    return (mDay * 0x15180) + (mMonth * 0x2A3000) + (mYear - 100) * 0x1FA4000
        + (mHour * 0xE10) + (mMin * 0x3C) + mSec;
}

int DateTime::Year() const { return mYear + 1900; }
int DateTime::Month() const { return mMonth + 1; }

bool DateTime::IsLeapYear() const {
    int year = Year();
    return (!(year & 3) && year % 100) || !(year % 400);
}

int DateTime::DayOfYear() const {
    static unsigned short sCumulativeDays[] = { 0,   31,  59,  90,  120, 151,
                                                181, 212, 243, 273, 304, 334 };
    int add = mMonth > 1 && IsLeapYear();
    return sCumulativeDays[mMonth] + mDay + add;
}

unsigned int DateTime::DiffSeconds(DateTime &dt) { return ToSeconds() - dt.ToSeconds(); }

void DateTime::FromFileTime(const FILETIME &ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    mYear = st.wYear - 108;
    mMonth = st.wMonth - 1;
    mDay = st.wDay;
    mHour = st.wHour;
    mMin = st.wMinute;
    mSec = st.wSecond;
}

namespace {
    Symbol MonthToken(int month) {
        MILO_ASSERT((0) <= (month) && (month) <= (11), 0x1A1);
        static Symbol month_symbols[12] = {
            "month_january",   "month_february", "month_march",    "month_april",
            "month_may",       "month_june",     "month_july",     "month_august",
            "month_september", "month_october",  "month_november", "month_december"
        };
        return month_symbols[month];
    }
}

void DateTime::Format(class String &str) const {
    char buf[256];

    if (SearchReplace(str.c_str(), "%d", MakeString("%02d", mDay), buf)) {
        str = buf;
    }
    static Symbol fre("fre");
    static Symbol ita("ita");
    static Symbol esl("esl");
    Symbol lang = SystemLanguage();
    if (lang == fre || lang == ita || lang == esl) {
        if (SearchReplace(str.c_str(), "%e", MakeString("%d", str), buf)) {
            str = buf;
        }
    } else {
        if (SearchReplace(
                str.c_str(),
                "%e",
                LocalizeOrdinal(
                    mDay, LocaleGenderMasculine, LocaleSingular, false, gNullStr, TheLocale
                ),
                buf
            )) {
            str = buf;
        }
    }

    if (SearchReplace(str.c_str(), "%m", MakeString("%02d", mMonth + 1), buf)) {
        str = buf;
    }
    if (strstr(str.c_str(), "%M")) {
        if (SearchReplace(
                str.c_str(), "%M", Localize(MonthToken(mMonth), nullptr, TheLocale), buf
            )) {
            str = buf;
        }
    }

    if (SearchReplace(str.c_str(), "%Y", MakeString("%04d", mYear + 1900), buf)) {
        str = buf;
    }
    if (SearchReplace(str.c_str(), "%H", MakeString("%02d", mHour), buf)) {
        str = buf;
    }
    if (SearchReplace(str.c_str(), "%i", MakeString("%02d", mMin), buf)) {
        str = buf;
    }
    if (SearchReplace(str.c_str(), "%s", MakeString("%02d", mSec), buf)) {
        str = buf;
    }
}

DateFormat DateTime::GetDateFormatting() {
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
    Symbol locale = SystemLocale();
    Symbol lang = SystemLanguage();
    if (locale == usa || locale == can)
        return kMDY;
    else if (locale == swe)
        return kYMD;
    else if (locale == chn || locale == hkg || locale == jpn || locale == kor
             || locale == sin || locale == tpe)
        return kISO;
    else
        return kDMY;
}

BinStream &operator<<(BinStream &bs, const DateTime &dt) {
    bs << dt.mSec << dt.mMin << dt.mHour << dt.mDay << dt.mMonth << dt.mYear;
    return bs;
}

BinStream &operator>>(BinStream &bs, DateTime &dt) {
    bs >> dt.mSec >> dt.mMin >> dt.mHour >> dt.mDay >> dt.mMonth >> dt.mYear;
    return bs;
}

void DateTime::ToDateString(String &str) const {
    switch (GetDateFormatting()) {
    case kMDY:
        str += MakeString("%02d/%02d/%02d", Month(), mDay, Year() % 100);
        break;
    case kDMY:
        str += MakeString("%02d/%02d/%02d", mDay, Month(), Year() % 100);
        break;
    case kYMD:
        str += MakeString("%02d/%02d/%02d", Year() % 100, Month(), mDay);
        break;
    case kISO:
        str += MakeString("%04d/%02d/%02d", Year(), Month(), mDay);
        break;
    default:
        break;
    }
}

void DateTime::ToString(String &str) const {
    ToDateString(str);
    str += MakeString(" %02d:%02d:%02d", mHour, mMin, mSec);
}

void DateTimeInit() {}
