#pragma once
#include "utl/Str.h"
#include "utl/BinStream.h"
#include "xdk/XAPILIB.h"

enum DateFormat {
    kMDY = 0, // month day year (USA/Canada)
    kDMY = 1, // day month year (most of the world)
    kYMD = 2, // year month day (Sweden)
    kISO = 3, // full year month day (Asia)
};

struct DateTime {
    unsigned char mSec;
    unsigned char mMin;
    unsigned char mHour;
    unsigned char mDay;
    unsigned char mMonth;
    unsigned char mYear;

    DateTime() { memset(&mSec, 0, 6); }
    DateTime(unsigned int);
    DateTime(
        unsigned short year,
        unsigned char month,
        unsigned char day,
        unsigned char hr,
        unsigned char min,
        unsigned char sec
    );
    void GetDateAndTime(DateTime &);
    unsigned int ToCode() const;
    void ToString(class String &) const;
    void ToDateString(class String &) const;
    void Format(class String &) const;
    int Year() const;
    int Month() const;
    bool IsLeapYear() const;
    int DayOfYear() const;
    int DayOfWeek() const;
    int ToDayNumber();
    void FromDayNumber(int);
    unsigned int ToSeconds();
    unsigned int DiffSeconds(DateTime &);
    void FromFileTime(const FILETIME &);
    void ParseDate(const char *);
    void FromUtcToLocal();

    static DateFormat GetDateFormatting();
};

void DateTimeInit();
void GetDateAndTime(DateTime &);
void GetSystemDateAndTime(DateTime &dt);
int DateTimeCmp(const DateTime &, const DateTime &);
BinStream &operator<<(BinStream &, const DateTime &);
BinStream &operator>>(BinStream &, DateTime &);
