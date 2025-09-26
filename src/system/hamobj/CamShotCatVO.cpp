#include "hamobj/CamShotCatVO.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "os/Debug.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

Symbol StrToCharacterSym(String str) {
    str.ToLower();
    Symbol charSym(str.c_str());
    static Symbol CHARACTERS("CHARACTERS");
    DataArray *charArr = DataGetMacro(CHARACTERS)->FindArray(charSym, false);
    if (charArr) {
        return charSym;
    } else {
        MILO_NOTIFY("%s is not a valid character symbol", charSym);
        return gNullStr;
    }
}

Symbol StrToCrewSym(String str) {
    str.ToLower();
    Symbol crewSym(str.c_str());
    static Symbol CREWS("CREWS");
    DataArray *charArr = DataGetMacro(CREWS)->FindArray(crewSym, false);
    if (charArr) {
        return crewSym;
    } else {
        MILO_NOTIFY("%s is not a valid crew symbol", crewSym);
        return gNullStr;
    }
}

void CamShotVOData(
    Symbol s, Symbol &s1, Symbol &charSym, Symbol &crewSym, Symbol &winLevelSym
) {
    s1 = charSym = crewSym = winLevelSym = gNullStr;
    static Symbol INTRO_CAM_CATS("INTRO_CAM_CATS");
    static Symbol OUTRO_CAM_CATS("OUTRO_CAM_CATS");
    static Symbol intro_quick("intro_quick");
    static Symbol intro_skills("intro_skills");
    static Symbol intro_playlist("intro_playlist");
    static Symbol battle_intro_crew("battle_intro_crew");
    static Symbol camp_intro_crew("camp_intro_crew");
    static Symbol win_dlg_char("win_dlg_char");
    static Symbol win_mov_char("win_mov_char");
    static Symbol win_hype_solo("win_hype_solo");
    static Symbol win_hype_crew("win_hype_crew");
    static Symbol win_hype_diff_crew("win_hype_diff_crew");
    static Symbol win_camp_crew("win_camp_crew");
    static Symbol lose_camp_char("lose_camp_char");
    static Symbol battle_outro_crew("battle_outro_crew");
    static Symbol all("all");
    static Symbol active("active");
    bool hasIntros = DataGetMacro(INTRO_CAM_CATS)->Contains(s);
    bool hasOutros = DataGetMacro(OUTRO_CAM_CATS)->Contains(s);
    if (hasIntros || hasOutros) {
        String str(s);
        std::vector<String> subStrings;
        str.split("_", subStrings);
        if (hasIntros) {
            static Symbol INTRO_QUICK("INTRO_QUICK");
            static Symbol INTRO_SKILLS("INTRO_SKILLS");
            static Symbol INTRO_SKILLS_LONG("INTRO_SKILLS_LONG");
            static Symbol INTRO_PLAYLIST("INTRO_PLAYLIST");
            if (s == INTRO_QUICK) {
                s1 = intro_quick;
                charSym = all;
            } else if (s == INTRO_SKILLS || s == INTRO_SKILLS_LONG) {
                s1 = intro_skills;
                charSym = all;
            } else if (s == INTRO_PLAYLIST) {
                s1 = intro_playlist;
                charSym = all;
            } else if (subStrings.size() > 0) {
                if (subStrings[0] == "BATTLE") {
                    s1 = battle_intro_crew;
                    charSym = all;
                    crewSym = StrToCrewSym(subStrings[2]);
                } else if (subStrings[0] == "CAMP") {
                    s1 = camp_intro_crew;
                    charSym = all;
                }
            }
            if (s1.Null()) {
                MILO_NOTIFY("Unknown intro category %s", s);
            }
        } else if (hasOutros) {
            if (subStrings.size() > 0 && subStrings[0] == "BATTLE") {
                s1 = battle_outro_crew;
                crewSym = StrToCrewSym(subStrings[2]);
            } else if (subStrings.size() > 1 && subStrings[1] == "CAMP") {
                if (subStrings[0] == "WIN") {
                    s1 = win_camp_crew;
                    charSym = all;
                } else if (subStrings[0] == "LOSE") {
                    s1 = lose_camp_char;
                    if (subStrings.size() > 2) {
                        charSym = StrToCharacterSym(subStrings[2]);
                        String s2Str(charSym);
                        if (s2Str.contains("robot")) {
                            charSym = all;
                        }
                    } else {
                        MILO_NOTIFY("Could not find character in %s", s);
                    }
                } else {
                    MILO_NOTIFY("Could not determine cam_type for %s", s);
                }
            } else if (subStrings.size() > 1 && subStrings[1] == "HYPE") {
                static Symbol WIN_HYPE_SOLO("WIN_HYPE_SOLO");
                static Symbol WIN_HYPE_DIFF_CREW("WIN_HYPE_DIFF_CREW");
                if (s == WIN_HYPE_SOLO) {
                    s1 = win_hype_solo;
                    charSym = active;
                } else {
                    s1 = s == WIN_HYPE_DIFF_CREW ? win_hype_diff_crew : win_hype_crew;
                    charSym = all;
                }
            } else if (subStrings.size() > 2) {
                if (subStrings[2] == "DLG") {
                    s1 = win_dlg_char;
                } else if (subStrings[2] == "MOV") {
                    s1 = win_mov_char;
                } else {
                    MILO_NOTIFY("Could not find cam_type for %s", s);
                }
            }

            if (subStrings.size() > 1) {
                String strd0(subStrings[1]);
                strd0.ToLower();
                static Symbol low("low");
                static Symbol med("med");
                static Symbol high("high");
                if (strd0 == low || strd0 == med || strd0 == high) {
                    winLevelSym = strd0.c_str();
                }
            }
            if (winLevelSym.Null()) {
                MILO_NOTIFY("Couldn't find win level for %s", s);
            }
            if (subStrings.size() > 3) {
                charSym = StrToCharacterSym(subStrings[3]);
            } else {
                MILO_NOTIFY("Couldn't find character for %s", s);
            }
        }
    }
}

DataNode OnCamShotVOData(DataArray *a) {
    Symbol s = a->Sym(1);
    Symbol s1, s2, s3, s4;
    CamShotVOData(s, s1, s2, s3, s4);
    *a->Var(2) = s1;
    *a->Var(3) = s2;
    *a->Var(4) = s3;
    *a->Var(5) = s4;
    return 0;
}

void CamShotCatVOInit() { DataRegisterFunc("camshot_vo_data", OnCamShotVOData); }
