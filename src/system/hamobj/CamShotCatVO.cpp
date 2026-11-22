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
    Symbol camera_category,
    Symbol &cam_type,
    Symbol &character,
    Symbol &crew,
    Symbol &win_level
) {
    cam_type = character = crew = win_level = gNullStr;
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
    bool hasIntros = DataGetMacro(INTRO_CAM_CATS)->Contains(camera_category);
    bool hasOutros = DataGetMacro(OUTRO_CAM_CATS)->Contains(camera_category);
    if (hasIntros || hasOutros) {
        String str(camera_category);
        std::vector<String> subStrings;
        str.split("_", subStrings);
        if (hasIntros) {
            static Symbol INTRO_QUICK("INTRO_QUICK");
            static Symbol INTRO_SKILLS("INTRO_SKILLS");
            static Symbol INTRO_SKILLS_LONG("INTRO_SKILLS_LONG");
            static Symbol INTRO_PLAYLIST("INTRO_PLAYLIST");
            if (camera_category == INTRO_QUICK) {
                cam_type = intro_quick;
                character = all;
            } else if (
                camera_category == INTRO_SKILLS || camera_category == INTRO_SKILLS_LONG
            ) {
                cam_type = intro_skills;
                character = all;
            } else if (camera_category == INTRO_PLAYLIST) {
                cam_type = intro_playlist;
                character = all;
            } else if (subStrings.size() > 0) {
                if (subStrings[0] == "BATTLE") {
                    cam_type = battle_intro_crew;
                    character = all;
                    crew = StrToCrewSym(subStrings[2]);
                } else if (subStrings[0] == "CAMP") {
                    cam_type = camp_intro_crew;
                    character = all;
                }
            }
            if (cam_type.Null()) {
                MILO_NOTIFY("Unknown intro category %s", camera_category);
            }
        } else if (hasOutros) {
            if (subStrings.size() > 0 && subStrings[0] == "BATTLE") {
                cam_type = battle_outro_crew;
                character = all;
                crew = StrToCrewSym(subStrings[2]);
            } else if (subStrings.size() > 1 && subStrings[1] == "CAMP") {
                if (subStrings[0] == "WIN") {
                    cam_type = win_camp_crew;
                    character = all;
                } else if (subStrings[0] == "LOSE") {
                    cam_type = lose_camp_char;
                    if (subStrings.size() > 2) {
                        character = StrToCharacterSym(subStrings[2]);
                        String s2Str(character);
                        if (s2Str.contains("robot")) {
                            character = all;
                        }
                    } else {
                        MILO_NOTIFY("Could not find character in %s", camera_category);
                    }
                } else {
                    MILO_NOTIFY("Could not determine cam_type for %s", camera_category);
                }
            } else if (subStrings.size() > 1 && subStrings[1] == "HYPE") {
                static Symbol WIN_HYPE_SOLO("WIN_HYPE_SOLO");
                static Symbol WIN_HYPE_DIFF_CREW("WIN_HYPE_DIFF_CREW");
                if (camera_category == WIN_HYPE_SOLO) {
                    cam_type = win_hype_solo;
                    character = active;
                } else if (camera_category == WIN_HYPE_DIFF_CREW) {
                    cam_type = win_hype_diff_crew;
                    character = all;
                } else {
                    cam_type = win_hype_crew;
                    character = all;
                }
            } else {
                if (subStrings.size() > 2) {
                    const String &str2 = subStrings[2];
                    if (str2 == "DLG") {
                        cam_type = win_dlg_char;
                    } else if (str2 == "MOV") {
                        cam_type = win_mov_char;
                    } else {
                        MILO_NOTIFY("Could not find cam_type for %s", camera_category);
                    }
                }
                if (subStrings.size() > 1) {
                    String strd0(subStrings[1]);
                    strd0.ToLower();
                    static Symbol low("low");
                    static Symbol med("med");
                    static Symbol high("high");
                    if (strd0 == low || strd0 == med || strd0 == high) {
                        win_level = strd0.c_str();
                    }
                }
                if (win_level.Null()) {
                    MILO_NOTIFY("Couldn't find win level for %s", camera_category);
                }
                if (subStrings.size() > 3) {
                    character = StrToCharacterSym(subStrings[3]);
                } else {
                    MILO_NOTIFY("Couldn't find character for %s", camera_category);
                }
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
