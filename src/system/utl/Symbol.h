#pragma once

class Symbol {
public:
    Symbol(const char *);
    Symbol(const Symbol &rhs) : mStr(rhs.mStr) {}

    static void PreInit(int, int);
    static void Init(void);
    static void Terminate(void);

    const char *mStr;
};
