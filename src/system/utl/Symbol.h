#pragma once

class Symbol {
private:
    const char* mStr;
public:
    Symbol(const char *);
    Symbol(const Symbol &rhs) : mStr(rhs.mStr) {}

    const char* Str() const { return mStr; }

    static void PreInit(int, int);
    static void Init(void);
    static void Terminate(void);
};
