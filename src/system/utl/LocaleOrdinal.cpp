#include "LocaleOrdinal.h"

#include "UTF8.h"
#include "os/System.h"

const char *LocalizeOrdinal(int num, LocaleGender gender, LocaleNumber number, bool superscriptMarkup, Symbol lang, Locale &locale) {
    char buf[255];
    buf[0] = *gNullStr;
    char code1, code2;
    if (!lang) {
        lang = SystemLanguage();
    }
    strncpy(buf, LocalizeSeparatedInt(num, locale), 255);
    int len = strlen(buf);
    if (len > 0) code1 = buf[len - 1];
    else code1 = '0';
    if (len > 1) code2 = buf[len - 2];
    else code2 = '0';

    static Symbol jpn("jpn");
    static Symbol eng("eng");
    static Symbol fre("fre");
    static Symbol deu("deu");
    static Symbol esl("esl");
    static Symbol ita("ita");

    if (lang != jpn) {
        if (lang == eng) {
            if (superscriptMarkup) strcat(buf, "<sup>");
            if (code1 == '1' && code2 != '1') strcat(buf, "st");
            else if (code2 == '2' && code2 != '1') strcat(buf, "nd");
            else if (code1 == '3' && code2 != '1') strcat(buf, "rd");
            else strcat(buf, "th");
            if (superscriptMarkup) strcat(buf, "</sup>");
        }
        else if (lang == fre) {
            if (superscriptMarkup) strcat(buf, "<sup>");
            if (code1 == code2) {
                if (gender == LocaleGenderMasculine) strcat(buf, "er");
                // looks like there may be something else happening here but i cant see it
                else strcat(buf, "re");
            }
            else strcat(buf, "e");
            if (superscriptMarkup) strcat(buf, "</sup>");
        }
        else if (lang == deu) strcat(buf, ".");
        else if (lang == esl || lang == ita) {
            String str;
            EncodeUTF8(str, 0xb0);
            if (gender == LocaleGenderMasculine) {
                if (number == 0) strcat(buf, str.c_str());
                else { strcat(buf, str.c_str()); strcat(buf, "s"); }
            }
            else if (number == 0) strcat(buf, str.c_str());
            else { strcat(buf, str.c_str()); strcat(buf, "s"); }
        }
        else MILO_NOTIFY("Localizing Ordinal for unsupported language %s", lang);
    }
    return MakeString(buf);
}