#include "meta_ham/Utl.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <cstring>

Symbol GetStarsToken(int i) {
    static Symbol stars_0("stars_0");
    static Symbol stars_1("stars_1");
    static Symbol stars_2("stars_2");
    static Symbol stars_3("stars_3");
    static Symbol stars_4("stars_4");
    static Symbol stars_5("stars_5");
    static Symbol stars_6("stars_6");
    switch (i) {
    case 0:
        return stars_0;
    case 1:
        return stars_1;
    case 2:
        return stars_2;
    case 3:
        return stars_3;
    case 4:
        return stars_4;
    case 5:
        return stars_5;
    case 6:
        return stars_6;
    }
    return gNullStr;
}

String GetSeconds(int i) {
    static Symbol stats_format_unit_seconds("stats_format_unit_seconds");
    static Symbol stats_format_unit_second("stats_format_unit_second");
    if (i == 1) {
        return Localize(stats_format_unit_second, 0, TheLocale);
    } else {
        return MakeString(Localize(stats_format_unit_seconds, 0, TheLocale), i);
    }
}

String GetMinutes(int i) {
    static Symbol stats_format_unit_minutes("stats_format_unit_minutes");
    static Symbol stats_format_unit_minute("stats_format_unit_minute");
    if (i == 1) {
        return Localize(stats_format_unit_minute, 0, TheLocale);
    } else {
        return MakeString(Localize(stats_format_unit_minutes, 0, TheLocale), i);
    }
}

String GetHours(int i) {
    static Symbol stats_format_unit_hours("stats_format_unit_hours");
    static Symbol stats_format_unit_hour("stats_format_unit_hour");
    if (i == 1) {
        return Localize(stats_format_unit_hour, 0, TheLocale);
    } else {
        return MakeString(Localize(stats_format_unit_hours, 0, TheLocale), i);
    }
}

String GetDays(int i) {
    static Symbol stats_format_unit_days("stats_format_unit_days");
    static Symbol stats_format_unit_day("stats_format_unit_day");
    if (i == 1) {
        return Localize(stats_format_unit_day, 0, TheLocale);
    } else {
        return MakeString(Localize(stats_format_unit_days, 0, TheLocale), i);
    }
}

void GetTimeString(int seconds, char *buf) {
    static Symbol stats_format_time_double("stats_format_time_double");
    String primary;
    String secondary;
    Symbol empty(gNullStr);
    if (seconds < 60) {
        primary = GetSeconds(seconds);
    } else if (seconds < 3600) {
        int mins = seconds / 60;
        int secs = seconds - mins * 60;
        if (secs > 0) {
            primary = GetMinutes(mins);
            secondary = GetSeconds(secs);
        } else {
            primary = GetMinutes(mins);
        }
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        int mins = (seconds - hours * 3600) / 60;
        if (mins > 0) {
            primary = GetHours(hours);
            secondary = GetMinutes(mins);
        } else {
            primary = GetHours(hours);
        }
    } else {
        int days = seconds / 86400;
        int hours = (seconds - days * 86400) / 3600;
        if (hours > 0) {
            primary = GetDays(days);
            secondary = GetHours(hours);
        } else {
            primary = GetDays(days);
        }
    }
    String result;
    if (!secondary.empty()) {
        result = MakeString(
            Localize(stats_format_time_double, 0, TheLocale), primary, secondary
        );
    } else {
        result = primary;
    }
    strcpy(buf, result.c_str());
}

char const *FormatTimeMS(int i) {
    int minutes = i / 60;
    int seconds = i % 60;
    return MakeString("%d:%02d", minutes, seconds);
}

char const *FormatTimeHMS(int i) {
    int seconds = i % 60;
    int minutes = (i % 3600) / 60;
    int hours = (i / 3600) >= 99 ? 99 : (i / 3600);
    return MakeString("%02d:%02d:%02d", hours, minutes, seconds);
}
