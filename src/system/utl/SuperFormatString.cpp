#include "utl/SuperFormatString.h"

const char *SuperFormatString::FinalStr() {
    if (unk1014) {
        return mFmt;
    } else {
        const char *ret = Str();
        if (unk1015) {
            String str(ret);
            str += "%s";
            return MakeString(str.c_str(), "");
        } else {
            return ret;
        }
    }
}
