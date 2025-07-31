#pragma once

class Licenses {
public:
    enum Requirement {
    };
    Licenses(const char *, Requirement);

    static void PrintAll(void);

protected:
    static Licenses *sFront;
    static int sInited;

private:
    Requirement unk_0x0;
    const char *mName;
    Licenses *mNext; // 0x8
};
