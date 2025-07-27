#pragma once

class Symbol {
public:
    Symbol(const char *);

    static void PreInit(int, int);
    static void Init(void);
    static void Terminate(void);
};
