#pragma once
#include "os/File.h"
#include "utl/Str.h"

class XboxMapFile {
public:
    XboxMapFile(const char *filename);
    ~XboxMapFile();

    const char *GetFunction(unsigned int addr, bool restart);
    static bool ParseStack(
        const char *mapname, struct StackData *results, int stack_size, FixedString &msg
    );

private:
    void ReadLine(char *dest, int destSize);

    File *mFile; // 0x0
    int mStart; // 0x4
};
