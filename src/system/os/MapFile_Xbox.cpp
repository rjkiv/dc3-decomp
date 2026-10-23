#include "os/MapFile_Xbox.h"
#include "os/File.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include <cstdio>

namespace {
    const char *kUnknownFunction = "????";
    const char *kSpecialOperators[] = { "constuctor",
                                        "destructor",
                                        "operator new",
                                        "operator delete",
                                        "operator =",
                                        "operator >>",
                                        "operator <<",
                                        "operator !",
                                        "operator ==",
                                        "operator !=",
                                        "operator []",
                                        "operator type",
                                        "operator ->",
                                        "operator *",
                                        "operator ++",
                                        "operator --",
                                        "operator -",
                                        "operator +",
                                        "operator &",
                                        "operator ->*",
                                        "operator /",
                                        "operator %",
                                        "operator <",
                                        "operator <=",
                                        "operator >",
                                        "operator >=",
                                        "operator ,",
                                        "operator ()",
                                        "operator ~",
                                        "operator ^",
                                        "operator |",
                                        "operator &&",
                                        "operator ||",
                                        "operator *=",
                                        "operator +=",
                                        "operator -=",
                                        "operator /=",
                                        "operator %=",
                                        "operator >>=",
                                        "operator <<=",
                                        "operator &=",
                                        "operator |=",
                                        "operator ^=",
                                        "$vftable",
                                        "$vbtable",
                                        "vcall",
                                        "typeof",
                                        "local static guard",
                                        "string literal",
                                        "__vbaseDtor",
                                        "__vecDelDtor",
                                        "__dflt_ctor_closure",
                                        "__delDtor",
                                        "__vec_ctor",
                                        "__vec_dtor",
                                        "__vec_ctor_vb",
                                        "$vdispmap",
                                        "__ehvec_ctor",
                                        "__ehvec_dtor",
                                        "__ehvec_ctor_vb",
                                        "__copy_ctor_closure",
                                        "udt returning",
                                        "EH",
                                        "RTTI",
                                        "$locvftable",
                                        "__local_vftable_ctor_closure",
                                        "operator new []",
                                        "operator delete []",
                                        "omni callsig",
                                        "__placement_delete_closure",
                                        "__placement_arrayDelete_closure",
                                        "operator _Z",
                                        "operator __0",
                                        "operator __1",
                                        "operator __2",
                                        "operator __3",
                                        "operator __4",
                                        "operator __5",
                                        "operator __6",
                                        "operator __7",
                                        "operator __8",
                                        "operator __9",
                                        "__man_vec_ctor",
                                        "__man_vec_dtor",
                                        "__ehvec_copy_ctor",
                                        "__ehvec_copy_ctor_vb" };

    void specialIdxStr(char *buffer, unsigned int size, int idx) {
        if (idx < DIM(kSpecialOperators)) {
            _snprintf(buffer, size, "%s", kSpecialOperators[idx]);
        } else {
            int theChar = idx % 36;
            if (theChar < 10) {
                theChar += '0';
            } else {
                theChar += '7';
            }
            _snprintf(buffer, size, "operator %.*s%c", idx / 36, "_____", (char)theChar);
        }
        buffer[size - 1] = '\0';
    }
}

void TryDemangleScope(char *demangled, const char *mangled, int i3) {
    char *c2 = strchr(mangled, 64);
    if (c2 && c2 < mangled + i3) {
        int count = c2 - mangled;
        TryDemangleScope(demangled, c2 + 1, (i3 - count) - 1);
        strcat(demangled, "::");
        strncat(demangled, mangled, count);
    } else {
        strncat(demangled, mangled, i3);
    }
}

void TryDemangleParams(char *demangled, const char *mangled) {
    bool b2 = true;
    const char *p8 = mangled;
    if (!mangled) {
        strcat(demangled, "()");
    } else {
        while (p8) {
            const char *strAtAt = strstr(p8, "@@");
            if (b2) {
                if (strAtAt) {
                    strcat(demangled, "(");
                } else {
                    strcat(demangled, "()");
                }
                b2 = false;
            } else {
                if (strAtAt) {
                    strcat(demangled, ",");
                } else {
                    strcat(demangled, ")");
                }
            }
            if (!strAtAt)
                return;
            const char *strV = strstr(p8, "V");
            const char *strW = strstr(p8, "W4");
            if (!strV || (strW && strV >= strW)) {
                if (strW) {
                    strV = strW + 2;
                } else {
                    strV = p8;
                }
            } else {
                strV++;
            }
            TryDemangleScope(demangled, strV, strAtAt - strV);
            p8 = strAtAt + 2;
        };
    }
}

void TryDemangleFunc(char *demangled, const char *mangled) {
    *demangled = '\0';
    if (*mangled && *mangled == '?') {
        const char *p = mangled + 1;
        if (*p == '?') {
            p++;
            int idx = 0;
            while (p[idx] == '_') {
                idx++;
            }
            int i7 = 0x56;
            char curChar = p[idx];
            if (curChar >= '0' && curChar <= '9') {
                i7 = (idx * 0x24 + curChar) - 0x30;
            } else if (curChar >= 'A' && curChar <= 'Z') {
                i7 = (idx * 0x24 + curChar) - 0x37;
            }
            char buf[128];
            specialIdxStr(buf, 0x80, i7);
            p += idx + 1;
            const char *params_start = strstr(p, "@@");
            if (!params_start) {
                params_start = strstr(p, "@");
            }
            MILO_ASSERT(params_start, 0x100);
            TryDemangleScope(demangled, p, params_start - p);
            strcat(demangled, "::");
            strcat(demangled, kSpecialOperators[i7]);
            TryDemangleParams(demangled, params_start + 2);
        } else {
            const char *params_start = strstr(p, "@@");
            TryDemangleScope(demangled, p, params_start - p);
            TryDemangleParams(demangled, params_start + 2);
        }
    } else {
        strncpy(demangled, mangled, 0x1E);
        strcpy(&demangled[0x1E], "...");
    }
    for (char *p = demangled; *p != '\0'; p++) {
        if (*p == '@') {
            *p = ' ';
        }
    }
}

XboxMapFile::XboxMapFile(const char *filename) {
    static int _x = MemFindHeap("main");
    MemHeapTracker tmp(_x);
    mFile = NewFile(filename, FILE_OPEN_NOARK | FILE_OPEN_READ);
    char buf[1024];
    if (!mFile) {
        return;
    } else {
        do {
            ReadLine(buf, 1024);
        } while (!strstr(buf, "Publics by Value"));
        ReadLine(buf, 1024);
        mStart = mFile->Tell();
    }
}

XboxMapFile::~XboxMapFile() { delete mFile; }

void XboxMapFile::ReadLine(char *dest, int destSize) {
    int idx = 0;
    while (!mFile->Eof()) {
        char curChar;
        mFile->Read(&curChar, 1);
        if (curChar == '\n')
            break;
        if (idx < destSize - 1) {
            dest[idx] = curChar;
            idx++;
        }
    }
    dest[idx] = '\0';
}

const char *XboxMapFile::GetFunction(unsigned int addr, bool restart) {
    static char funcName[1024];
    if (restart) {
        mFile->Seek(mStart, 0);
    }
    char buf1[2048];
    char funcAddr[1024];
    char buf2[2048];
    sprintf(funcAddr, "%8x", addr);
    char *dest = buf1;
    const char *src = "";
    int oldTell = mFile->Tell();
    while (!mFile->Eof()) {
        dest = dest == buf1 ? buf2 : buf1;
        int curTell = mFile->Tell();
        ReadLine(dest, 0x800);
        char *pCur = &dest[0x15];
        char *orig = pCur;
        for (char notSpace = *pCur; notSpace != ' '; notSpace = *++pCur)
            ;
        *pCur = 0;
        for (char space = *++pCur; space == ' '; space = *++pCur)
            ;
        pCur[8] = 0;
        if (strcmp(funcAddr, pCur) < 0) {
            TryDemangleFunc(funcName, src);
            mFile->Seek(oldTell, 0);
            return funcName;
        }
        src = orig;
        oldTell = curTell;
    }
    return "(unknown)";
}

template <typename T>
void InsertSort(int *keys, T *vals, int size) {
    int offset = (int)keys - (int)vals;
    for (int i = 1; i < size; i++) {
        int k = keys[i];
        T v = vals[i];
        int j = i;
        for (; j > 0 && v < vals[j - 1]; j--) {
            vals[j] = vals[j - 1];
            keys[j] = keys[j - 1];
        }
        vals[j] = v;
        keys[j] = k;
    }
}

bool XboxMapFile::ParseStack(
    const char *mapname, struct StackData *results, int stack_size, FixedString &msg
) {
    XboxMapFile mapfile(mapname);
    if (!mapfile.mFile) {
        return false;
    } else {
        int keys[50];
        int numKeys;
        for (numKeys = 0; numKeys < stack_size; numKeys++) {
            keys[numKeys] = numKeys;
        }
        InsertSort(keys, results, numKeys);
        char funcNames[50][128];
        for (int i = 0; i < stack_size; i++) {
            unsigned int addr = results[i].address;
            int diff = keys[i] * 128;
            const char *source = mapfile.GetFunction(addr, false);
            char *which = funcNames[0] + diff;
            strncpy(which, source, 127);
            funcNames[0][diff + 127] = '\0';
        }
        for (int i = 0; i < stack_size; i++) {
            msg += "\n   ";
            msg += funcNames[i];
        }
    }

    return true;
}
