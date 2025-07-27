#pragma once

#include "types.h"

class App {
private:
    int _pad;

protected:
    void CDECL DrawRegular(void);
    void CDECL CaptureHiRes(void);

public:
    CDECL App(int, char **);
    CDECL ~App(void);

    void CDECL RunWithoutDebugging(void);
    void CDECL Run(void);
};
