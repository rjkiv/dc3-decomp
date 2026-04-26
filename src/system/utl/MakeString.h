#pragma once
#include "utl/Symbol.h"
#include "utl/Str.h"

class FormatString {
private:
    enum Type {
        kInt = 0,
        kStr = 1,
        kFloat = 2,
        kNone = 3,
    };

    char *mFmt; // 0x0
    char *mBuf; // 0x4
    int mBufSize; // 0x8
    char mFmtBuf[0x1000]; // 0xc
    char *mFmtEnd; // 0x100c
    Type mType; // 0x1010

    void UpdateType();

protected:
    FormatString();
    void InitializeWithFmt(const char *, bool);

public:
    FormatString(const char *);
    FormatString &operator<<(void *);
    FormatString &operator<<(int);
    FormatString &operator<<(unsigned int);
    FormatString &operator<<(long);
    FormatString &operator<<(unsigned long);
    FormatString &operator<<(long long);
    FormatString &operator<<(unsigned long long);
    FormatString &operator<<(const class DataNode &);
    FormatString &operator<<(const char *);
    FormatString &operator<<(float);
    FormatString &operator<<(double);
    FormatString &operator<<(const class String &);
    FormatString &operator<<(const class FixedString &);
    FormatString &operator<<(class Symbol);

    const char *Str();
};

void InitMakeString();
bool MakeStringInitted();
void TerminateMakeString();

inline const char *MakeString(const char *c) {
    FormatString fs(c);
    return fs.Str();
}

template <class T>
const char *MakeString(const char *c, const T &t) {
    FormatString fs(c);
    fs << t;
    return fs.Str();
}

template <class T1, class T2>
const char *MakeString(const char *c, const T1 &t1, const T2 &t2) {
    FormatString fs(c);
    fs << t1 << t2;
    return fs.Str();
}

template <class T1, class T2, class T3>
const char *MakeString(const char *c, const T1 &t1, const T2 &t2, const T3 &t3) {
    FormatString fs(c);
    fs << t1 << t2 << t3;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4>
const char *
MakeString(const char *c, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4, class T5>
const char *MakeString(
    const char *c, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
const char *MakeString(
    const char *c,
    const T1 &t1,
    const T2 &t2,
    const T3 &t3,
    const T4 &t4,
    const T5 &t5,
    const T6 &t6
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5 << t6;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
const char *MakeString(
    const char *c,
    const T1 &t1,
    const T2 &t2,
    const T3 &t3,
    const T4 &t4,
    const T5 &t5,
    const T6 &t6,
    const T7 &t7
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5 << t6 << t7;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
const char *MakeString(
    const char *c,
    const T1 &t1,
    const T2 &t2,
    const T3 &t3,
    const T4 &t4,
    const T5 &t5,
    const T6 &t6,
    const T7 &t7,
    const T8 &t8
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
    return fs.Str();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
const char *MakeString(
    const char *c,
    const T1 &t1,
    const T2 &t2,
    const T3 &t3,
    const T4 &t4,
    const T5 &t5,
    const T6 &t6,
    const T7 &t7,
    const T8 &t8,
    const T9 &t9
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
    return fs.Str();
}

template <
    class T1,
    class T2,
    class T3,
    class T4,
    class T5,
    class T6,
    class T7,
    class T8,
    class T9,
    class T10>
const char *MakeString(
    const char *c,
    const T1 &t1,
    const T2 &t2,
    const T3 &t3,
    const T4 &t4,
    const T5 &t5,
    const T6 &t6,
    const T7 &t7,
    const T8 &t8,
    const T9 &t9,
    const T10 &t10
) {
    FormatString fs(c);
    fs << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10;
    return fs.Str();
}
