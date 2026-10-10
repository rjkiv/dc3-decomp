#include "os/MapFile_Xbox.h"
#include "os/File.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include <cstdio>

namespace {
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
    mFile = NewFile(filename, 0x10002);
    char buf[1024];
    if (!mFile)
        return;
    else {
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
    static char sBuffer[0x400];
    if (restart) {
        mFile->Seek(mStart, 0);
    }
    char localC40[1024];
    char local1440[2048];
    char *cur = local1440;
    sprintf(localC40, "%8x", addr);
    int oldTell = mFile->Tell();
    char *b4 = "";
    while (!mFile->Eof()) {
        cur = cur != local1440 ? local1440 : localC40;
        int curTell = mFile->Tell();
        ReadLine(cur, 0x800);
        char c2 = cur[0x15];
        char *pCur = &cur[0x15];
        while (c2 != 0x20) {
            pCur++;
            c2 = *pCur;
        }
        *pCur = 0;
        while (*pCur++ == 0x20)
            ;
    }
    return "(unknown)";
}
