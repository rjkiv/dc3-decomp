#pragma once

#include "obj/Data.h"
#include "types.h"
#include "utl/Str.h"

class KnownIssues {
    String unk_0x0;
    String unk_0x8;
    float unk_0x10;
    u8 unk_0x14;

public:
    KnownIssues(void);
    void Draw();
    void Display(String, float);
    void Init();

    static DataNode OnDisplayKnownIssues(DataArray *);
    static DataNode OnToggleLastKnownIssues(DataArray *);
    static DataNode OnToggleAllowKnownIssues(DataArray *);
};

extern KnownIssues TheKnownIssues;
