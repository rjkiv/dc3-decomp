#include "utl/SuperFormatString.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/LocaleOrdinal.h"

#define BUF_SIZE 2048

// fills in placeholders in a localized string (e.g. {int:param:name})
SuperFormatString::SuperFormatString(
    const char *fmt, const DataArray *dict, bool tokens_only, Locale &locale, Symbol lang
)
    : mTokensOnly(tokens_only), mNeedsDoublePercentCollapse(false) {
		
    // there is nothing to actually fill, so it's just becomes a regular FormatString
    if (!dict && !tokens_only) {
        InitializeWithFmt(fmt, true);
        return;
    }
	
    char tempFmt[BUF_SIZE];
    char *tempFmtPos = tempFmt;
    char phInfo[64];
    char *phInfoPos = phInfo;
    char param[8];
    char *paramPos = param;
	
    // used to tell whether the string has %% but no actual printf args
    bool hasPrintfArg = false;
    bool atDoublePercent = false;
	
    enum {
        kString,
        kInt,
        kSepInt,
        kFloat,
        kToken,
        kOrdinal
    } phType = kString;
    enum {
        kInit,
        kType,
        kParam,
        kName
    } state = kInit;
	
    for (const char *i = fmt; *i != '\0'; i++) {
        switch (state) {
        case kInit:
            // copy plain text directly until we hit a placeholder
            if (*i == '{') {
                if (i[1] == '{') {
                    *tempFmtPos++ = '{';
                    i++;
                } else {
                    state = kType;
                }
            } else {
                if (*i == '%' && !atDoublePercent) {
                    if (i[1] == '%' && !hasPrintfArg) {
                        atDoublePercent = true;
                        mNeedsDoublePercentCollapse = true;
                    } else {
                        hasPrintfArg = true;
                        mNeedsDoublePercentCollapse = false;
                    }
                } else {
                    atDoublePercent = false;
                }
                *tempFmtPos++ = *i;
            }
            break;
        case kType:
            // reading the type up to the first :
            if (*i == ':') {
                MILO_ASSERT(phInfoPos - phInfo < 64, 0x5a);
                *phInfoPos = '\0';
                phInfoPos = phInfo;
                state = kName;
                if (strcmp(phInfo, "string") == 0) {
                    phType = kString;
                } else if (strcmp(phInfo, "int") == 0) {
                    phType = kInt;
                    state = kParam;
                    *paramPos++ = '%';
                } else if (strcmp(phInfo, "sep_int") == 0) {
                    phType = kSepInt;
                } else if (strcmp(phInfo, "float") == 0) {
                    phType = kFloat;
                    state = kParam;
                    *paramPos++ = '%';
                } else if (strcmp(phInfo, "token") == 0) {
                    phType = kToken;
                } else if (strcmp(phInfo, "ordinal") == 0) {
                    // no % here, the param is the gender/number letters
                    phType = kOrdinal;
                    state = kParam;
                } else {
                    MILO_FAIL("bad SuperFormatString placeholder type '%s'", phInfo);
                }
            } else {
                *phInfoPos++ = *i;
            }
            break;
        case kParam:
            // reading the param (seesm to be int, float, and ordinal only) up to the next :
            if (*i == ':') {
                if (phType == kFloat) {
                    *paramPos++ = 'f';
                    *paramPos = '\0';
                } else if (phType == kInt) {
                    *paramPos++ = 'i';
                    *paramPos = '\0';
                }
                MILO_ASSERT(paramPos - param < 8, 0x8f);
                if (phType == kOrdinal) {
                    MILO_ASSERT(param + 2 == paramPos, 0x95);
                }
                paramPos = param;
                state = kName;
            } else {
                *paramPos++ = *i;
            }
            break;
        case kName:
            // reading the name up to the closing }, and then we can fill in the placeholder
            if (*i == '}') {
                MILO_ASSERT(phInfoPos - phInfo < 64, 0xa3);
                *phInfoPos = '\0';
                phInfoPos = phInfo;
                state = kInit;
                bool isToken = phType == kToken;
                DataArray *arr = nullptr;
                if (!tokens_only && !isToken) {
                    arr = dict->FindArray(phInfo, false);
                }
                if (arr || isToken) {
                    DataNode val = isToken ? DataNode() : arr->Node(1).Evaluate();
                    bool wrongType = false;
                    switch (phType) {
                    case kString:
                        wrongType =
                            val.Type() != kDataString && val.Type() != kDataSymbol;
                        break;
                    case kInt:
                        wrongType = val.Type() != kDataInt;
                        break;
                    case kSepInt:
                        wrongType = val.Type() != kDataInt;
                        break;
                    case kFloat:
                        wrongType = val.Type() != kDataFloat && val.Type() != kDataInt;
                        break;
                    case kToken:
                        wrongType = false;
                        break;
                    case kOrdinal:
                        wrongType = val.Type() != kDataInt;
                        break;
                    }
                    if (!wrongType) {
                        int n = 0;
                        switch (phType) {
                        case kString:
                            if (val.Type() == kDataString) {
                                n = Hx_snprintf(
                                    tempFmtPos,
                                    tempFmt + BUF_SIZE - tempFmtPos,
                                    "%s",
                                    val.Str()
                                );
                            } else {
                                n = Hx_snprintf(
                                    tempFmtPos,
                                    tempFmt + BUF_SIZE - tempFmtPos,
                                    "%s",
                                    Localize(val.Sym(), nullptr, locale)
                                );
                            }
                            break;
                        case kInt:
                            n = Hx_snprintf(
                                tempFmtPos,
                                tempFmt + BUF_SIZE - tempFmtPos,
                                param,
                                val.Int()
                            );
                            break;
                        case kSepInt:
                            n = Hx_snprintf(
                                tempFmtPos,
                                tempFmt + BUF_SIZE - tempFmtPos,
                                "%s",
                                LocalizeSeparatedInt(val.Int(), locale)
                            );
                            break;
                        case kFloat:
                            n = Hx_snprintf(
                                tempFmtPos,
                                tempFmt + BUF_SIZE - tempFmtPos,
                                param,
                                val.Float()
                            );
                            break;
                        case kToken:
                            n = Hx_snprintf(
                                tempFmtPos,
                                tempFmt + BUF_SIZE - tempFmtPos,
                                "%s",
                                Localize(phInfo, nullptr, locale)
                            );
                            break;
                        // not m means feminine, not s means plural?
                        case kOrdinal: {
                            LocaleGender gender = param[0] == 'm' ? LocaleGenderMasculine
                                                                  : LocaleGenderFeminine;
                            LocaleNumber number =
                                param[1] == 's' ? LocaleSingular : LocalePlural;
                            n = Hx_snprintf(
                                tempFmtPos,
                                tempFmt + BUF_SIZE - tempFmtPos,
                                "%s",
                                LocalizeOrdinal(
                                    val.Int(), gender, number, false, lang, locale
                                )
                            );
                            break;
                        }
                        }
                        tempFmtPos += n;
                        break;
                    }
                    MILO_NOTIFY(
                        "parameter for placeholder '%s' was the wrong type", phInfo
                    );
                } else {
                    MILO_NOTIFY("couldn't find parameter for placeholder '%s'", phInfo);
                }
                // couldn't fill it in, so leave a marker in the string instead
                int n = Hx_snprintf(
                    tempFmtPos, tempFmt + BUF_SIZE - tempFmtPos, "{missing:%s}", phInfo
                );
                tempFmtPos += n;
            } else {
                *phInfoPos++ = *i;
            }
            break;
        }
    }
    // the string ended in the middle of a placeholder so emit a NOTIFY
    if (state != kInit) {
        *phInfoPos = '\0';
        MILO_NOTIFY("bad formatting for placeholder '%s'", phInfo);
        int n = Hx_snprintf(
            tempFmtPos, tempFmt + BUF_SIZE - tempFmtPos, "{badfmt:%s", phInfo
        );
        tempFmtPos += n;
    }
    *tempFmtPos = '\0';
    MILO_ASSERT(tempFmtPos - tempFmt < BUF_SIZE, 0x10b);
	
    // tokens_only skips the printf parsing, since the result won't have args fed into it
    InitializeWithFmt(tempFmt, !tokens_only);
}

const char *SuperFormatString::FinalStr() {
    if (mTokensOnly) {
        return mFmt;
    } else {
        const char *ret = Str();
        if (mNeedsDoublePercentCollapse) {
            String str(ret);
            str += "%s";
            return MakeString(str.c_str(), "");
        } else {
            return ret;
        }
    }
}
