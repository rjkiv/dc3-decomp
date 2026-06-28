#include "LocaleOrdinal.h"
#include "UTF8.h"
#include "os/System.h"

const char *LocalizeOrdinal(
    int num,
    LocaleGender gender,
    LocaleNumber number,
    bool superscriptMarkup,
    Symbol lang,
    Locale &locale
) {
    char buf[255];
    if (lang.Null()) {
        lang = SystemLanguage();
    }
    strncpy(buf, LocalizeSeparatedInt(num, locale), sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    int len = strlen(buf);
    char code1 = len > 0 ? buf[len - 1] : '0';
    char code2 = len > 1 ? buf[len - 2] : '0';

    static Symbol jpn("jpn");
    static Symbol eng("eng");
    static Symbol fre("fre");
    static Symbol deu("deu");
    static Symbol esl("esl");
    static Symbol ita("ita");

    if (lang != jpn) {
        if (lang == eng) {
            if (superscriptMarkup) {
                strcat(buf, "<sup>");
            }
            if (code1 == '1' && code2 != '1') {
                strcat(buf, "st");
            } else if (code1 == '2' && code2 != '1') {
                strcat(buf, "nd");
            } else if (code1 == '3' && code2 != '1') {
                strcat(buf, "rd");
            } else {
                strcat(buf, "th");
            }
            if (superscriptMarkup) {
                strcat(buf, "</sup>");
            }
        } else if (lang == fre) {
            if (superscriptMarkup) {
                strcat(buf, "<sup>");
            }
            if (streq(buf, "1")) {
                if (gender == LocaleGenderMasculine) {
                    strcat(buf, "er");
                } else {
                    strcat(buf, "re");
                }
            } else {
                strcat(buf, "e");
            }
            if (superscriptMarkup) {
                strcat(buf, "</sup>");
            }
        } else if (lang == deu) {
            strcat(buf, ".");
        } else if (lang == esl || lang == ita) {
            String str;
            EncodeUTF8(str, 0xb0);
            if (gender == LocaleGenderMasculine) {
                if (number == 0) {
                    strcat(buf, str.c_str());
                } else {
                    strcat(buf, str.c_str());
                    strcat(buf, "s");
                }
            } else if (number == 0) {
                strcat(buf, str.c_str());
            } else {
                strcat(buf, str.c_str());
                strcat(buf, "s");
            }
        } else {
            MILO_NOTIFY("Localizing Ordinal for unsupported language %s", lang);
        }
    }
    return MakeString(buf);
}
