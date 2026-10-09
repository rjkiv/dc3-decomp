#pragma once
#include "obj/Data.h"
#include "utl/Str.h"

/** Metadata for known issues, meant to be displayed on-screen. */
class KnownIssues {
public:
    KnownIssues();
    void Draw();
    void Display(String, float);
    void Init();

    static DataNode OnDisplayKnownIssues(DataArray *);
    static DataNode OnToggleLastKnownIssues(DataArray *);
    static DataNode OnToggleAllowKnownIssues(DataArray *);

private:
    /** The known issue's name. */
    String mFeature; // 0x0
    /** The known issue's description. */
    String mIssue; // 0x8
    float mDisplayTime; // 0x10
    /** Whether or not to display the known issue. */
    bool mAllowed; // 0x14
};

extern KnownIssues TheKnownIssues;
