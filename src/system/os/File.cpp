#include "os/File.h"
#include "HolmesClient.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ArkFile_p.h"
#include "os/AsyncFile.h"
#include "os/Block.h"
#include "os/Debug.h"
#include "os/FileCache.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "types.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Option.h"
#include "utl/Str.h"
#include <cctype>
#include <cstdio>
#include <cstring>

static char gSystemRoot[256] = { 0 }; // 0x0
static char gExecRoot[256] = { 0 }; // 0x100
static char gRoot[256] = { 0 }; // 0x200
static File *gOpenCaptureFile = nullptr; // 0x300
static int gCaptureFileMode = 0;

bool gFakeFileErrors = false;
bool gNullFiles = false;
void *kNoHandle = nullptr;
DataArray *gFrameRateArray = nullptr;

std::vector<File *> gFiles(0x80); // 0x10...?
std::vector<String> gDirList;
const int File::MaxFileNameLen = 0x100;

const char *FileRoot() { return gRoot; }
const char *FileExecRoot() { return gExecRoot; }
const char *FileSystemRoot() { return gSystemRoot; }

void FileTerminate() {
    RELEASE(gOpenCaptureFile);
    *gRoot = 0;
    *gExecRoot = 0;
    *gSystemRoot = 0;
    TheDebug.StopLog();
    HolmesClientTerminate();
}

void FileQualifiedFilename(String &out, const char *in) {
    char buf[256];
    FileQualifiedFilename(buf, 0x100, in);
    out = buf;
}

void FileNormalizePath(const char *cc) {
    for (char *ptr = (char *)cc; *ptr != '\0'; ptr++) {
        if (*ptr == '\\')
            *ptr = '/';
        else
            *ptr = tolower(*ptr);
    }
}

const char *FileGetExt(const char *root) {
    const char *end = root + strlen(root);
    for (const char *search = end - 1; search >= root; search--) {
        if (*search == '.') {
            return search + 1;
        } else if (*search == '/' || *search == '\\') {
            return end;
        }
    }
    return end;
}

const char *FileGetName(const char *file) {
    const char *dir = strrchr(file, '/');
    if (!dir) {
        dir = strrchr(file, '\\');
    }
    if (dir) {
        file = dir + 1;
    }
    return file;
}

static bool FileMatchInternal(const char *arg0, const char *arg1, bool arg2) {
    for (; *arg0 != 0; arg0++) {
        if (FileMatch(arg0, arg1))
            return true;
        if (!arg2 && (*arg0 == '/' || *arg0 == '\\'))
            return false;
    }
    return (*arg1 == *arg0);
}

bool FileMatch(const char *param1, const char *param2) {
    if (param2 == 0)
        return false;
    while (*param2 != '\0') {
        if (*param2 == '*')
            return FileMatchInternal(param1, param2 + 1, 0);
        if (*param2 == '&')
            return FileMatchInternal(param1, param2 + 1, 1);
        if (*param1 == '\0')
            break;
        if (*param2 == '?') {
            if ((*param1 == '\\') || (*param1 == '/'))
                return 0;
        } else if ((*param2 == '/') || (*param2 == '\\')) {
            if ((*param1 != '/') && (*param1 != '\\'))
                return 0;
        } else if (*param2 != *param1)
            return 0;
        param2++;
        param1++;
    }
    return (*param2 - *param1) == 0;
}

bool FileDiscSpinUp() { return TheBlockMgr.SpinUp(); }

const char *FrameRateSuffix() {
    return MakeString("_keep_%s.dta", PlatformSymbol(TheLoadMgr.GetPlatform()));
}

const char *FileGetPathBuf(const char *iBuf, char *oBuf) {
    MILO_ASSERT(oBuf, 0x3F6);
    if (iBuf) {
        strcpy(oBuf, iBuf);
        char *p = oBuf + strlen(oBuf) - 1;
        for (; p >= oBuf && *p != '/' && *p != '\\'; p--)
            ;
        if (p >= oBuf) {
            if (p != oBuf && p[-1] != ':') {
                p[0] = '\0';
                return oBuf;
            }
            p[1] = '\0';
            return oBuf;
        }
    }
    oBuf[0] = '.';
    oBuf[1] = '\0';
    return oBuf;
}

const char *FileGetDriveBuf(const char *iFilepath, char *oBuf) {
    MILO_ASSERT(iFilepath, 0x437);
    MILO_ASSERT(oBuf, 0x438);
    char *chr = strchr(iFilepath, ':');
    if (chr) {
        strncpy(oBuf, iFilepath, chr - iFilepath);
        oBuf[chr - iFilepath] = '\0';
    } else {
        oBuf[0] = '\0';
    }
    return oBuf;
}

const char *FileGetBaseBuf(const char *iFilepath, char *oBuf) {
    MILO_ASSERT(iFilepath, 0x458);
    MILO_ASSERT(oBuf, 0x459);
    char *chr = strrchr(iFilepath, '/');
    if (!chr) {
        chr = strrchr(iFilepath, '\\');
    }
    if (!chr) {
        strcpy(oBuf, iFilepath);
    } else {
        strcpy(oBuf, chr + 1);
    }
    char *dot = strrchr(oBuf, '.');
    if (dot) {
        *dot = '\0';
    }
    return oBuf;
}

const char *FileMakePathBuf(const char *iRoot, const char *iFilepath, char *oBuf) {
    MILO_ASSERT(iRoot, 0x300);
    MILO_ASSERT(iFilepath, 0x301);
    MILO_ASSERT(oBuf, 0x302);
    char buffer[256];
    if (iFilepath < oBuf || iFilepath >= oBuf + sizeof(buffer)) {
        if (iRoot >= oBuf && iRoot < oBuf + sizeof(buffer)) {
            strcpy(buffer, iRoot);
            iRoot = buffer;
        }
    } else {
        strcpy(buffer, iFilepath);
        iFilepath = buffer;
    }
    char driveBuf[256];
    const char *drive = FileGetDriveBuf(iFilepath, driveBuf);
    if (drive[0] != '\0') {
        iFilepath += strlen(drive) + 1;
    }
    char *c;
    if (iFilepath[0] != '/' && iFilepath[0] != '\\' && iFilepath[0] != '\0') {
        sprintf(oBuf, "%s/%s", iRoot, iFilepath);
        drive = FileGetDriveBuf(iRoot, driveBuf);
        if (drive[0] != '\0') {
            c = oBuf + strlen(drive) + 1;
        } else {
            c = oBuf;
        }
    } else if (drive[0] != '\0') {
        sprintf(oBuf, "%s:%s", drive, iFilepath);
        c = oBuf + strlen(drive) + 1;
    } else {
        drive = FileGetDriveBuf(iRoot, driveBuf);
        if (drive[0] != '\0') {
            sprintf(oBuf, "%s:%s", drive, iFilepath);
            c = oBuf + strlen(drive) + 1;
        } else {
            strcpy(oBuf, iFilepath);
            c = oBuf;
        }
    }
    FileNormalizePath(oBuf);
    bool slashFirst = oBuf[0] == '/';

    char *dirs[32];
    char **endDir = dirs;
    char *tok = strtok(c, "/");
    while (tok) {
        if (tok[0] == '.') {
            if (tok[1] == '.' && tok[2] == '\0') {
                if (endDir == dirs || *(endDir[-1]) == '.') {
                    *endDir++ = tok;
                } else {
                    endDir--;
                }
            }
        } else {
            *endDir++ = tok;
        }
        tok = strtok(nullptr, "/");
    }
    MILO_ASSERT(endDir - dirs <= 32, 0x35C);

    if (endDir == dirs) {
        if (slashFirst) {
            *c++ = '/';
        } else {
            *c++ = '.';
        }
    } else {
        for (char **it = dirs; it != endDir; ++it) {
            if (it != dirs || slashFirst) {
                *c++ = '/';
            }
            for (char *ptr = *it; *ptr != '\0'; ptr++) {
                *c++ = *ptr;
            }
        }
    }
    MILO_ASSERT(c - oBuf < File::MaxFileNameLen, 0x372);
    *c = '\0';
    return oBuf;
}

const char *FileRelativePathBuf(const char *iRoot, const char *iFilepath, char *oBuf) {
    MILO_ASSERT(iRoot, 0x38D);
    MILO_ASSERT(iFilepath, 0x38E);
    MILO_ASSERT(oBuf, 0x38F);
    if (*iFilepath != '\0') {
        char rootBuffer[256];
        char filePathBuffer[256];
        strcpy(rootBuffer, iRoot);
        strcpy(filePathBuffer, iFilepath);
        std::list<char *> tok218;
        std::list<char *> tok220;
        char *tok = strtok(rootBuffer, "/");
        while (tok) {
            tok218.push_back(tok);
            tok = strtok(nullptr, "/");
        }
        tok = strtok(filePathBuffer, "/");
        while (tok) {
            tok220.push_back(tok);
            tok = strtok(nullptr, "/");
        }
        if (!tok220.empty() && !tok218.empty()) {
            if (streq(tok218.front(), tok220.front())) {
                while (tok218.size() && tok220.size()
                       && streq(tok218.front(), tok220.front())) {
                    tok218.pop_front();
                    tok220.pop_front();
                }
                char *p = oBuf;
                while (tok218.size()) {
                    if (p != oBuf) {
                        *p++ = '/';
                    }
                    *p++ = '.';
                    *p++ = '.';
                    tok218.pop_front();
                }
                while (tok220.size()) {
                    if (p != oBuf) {
                        *p++ = '/';
                    }
                    for (char *ptr = tok220.front(); *ptr != '\0'; ptr++) {
                        *p++ = *ptr;
                    }
                    tok220.pop_front();
                }
                MILO_ASSERT(p - oBuf < File::MaxFileNameLen, 0x3D9);
                if (p == oBuf) {
                    *p++ = '.';
                }
                *p++ = '\0';
                return oBuf;
            }
        }
    }
    return iFilepath;
}

const char *FileGetPath(const char *file) {
    static char sBuf[0x100];
    MainThread();
    return FileGetPathBuf(file, sBuf);
}

const char *FileGetDrive(const char *file) {
    static char sBuf[0x100];
    MainThread();
    return FileGetDriveBuf(file, sBuf);
}

const char *FileGetBase(const char *file) {
    static char sBuf[0x100];
    MainThread();
    return FileGetBaseBuf(file, sBuf);
}

const char *FileMakePath(const char *root, const char *file) {
    static char sBuf[0x100];
    MainThread();
    return FileMakePathBuf(root, file, sBuf);
}

const char *FileRelativePath(const char *root, const char *filepath) {
    MainThread();
    static char relative[256];
    return FileRelativePathBuf(root, filepath, relative);
}

const char *FileLocalize(const char *iFilename, char *buffer) {
    static char sLocalBuf[256];
    bool newGfx = GetGfxMode() == kNewGfx;
    if (!SystemLanguage().Null() || newGfx) {
        if (!SystemLanguage().Null()) {
            const char *ptr = iFilename;
        here:
            if (ptr[0] != '\0') {
                if (ptr[0] != '/' || ptr[1] != 'e' || ptr[2] != 'n' || ptr[3] != 'g'
                    || ptr[4] != '/') {
                    ptr++;
                    goto here;
                }
                if (!buffer) {
                    buffer = sLocalBuf;
                }
                strcpy(buffer, iFilename);
                if (HongKongExceptionMet()
                    && (strstr(iFilename, "sfx/loc/")
                        || strstr(iFilename, "barks.milo"))) {
                    memcpy(&buffer[ptr + 1 - iFilename], "eng", 3);
                } else {
                    memcpy(&buffer[ptr + 1 - iFilename], SystemLanguage().Str(), 3);
                }
                iFilename = buffer;
            }
        }
        if (newGfx) {
            const char *ptr = iFilename;
            while (ptr[0] != '\0') {
                if (ptr[0] == '/' && ptr[1] == 'o' && ptr[2] == 'g' && ptr[3] == '/') {
                    if (buffer == iFilename) {
                        char *lmao = (char *)ptr;
                        lmao[1] = 'n';
                        return iFilename;
                    }
                    if (!buffer) {
                        buffer = sLocalBuf;
                    }
                    strcpy(buffer, iFilename);
                    buffer[ptr + 1 - iFilename] = 'n';
                    iFilename = buffer;
                    break;
                }
                ptr++;
            }
        }
    }
    return iFilename;
}

// the weird __rs in the debug symbols here, is for a FileStat&
// so BinStream >> FileStat
BinStream &operator>>(BinStream &bs, FileStat &fs) {
    bs >> fs.st_mode >> fs.st_size;
    u64 ctime;
    bs >> ctime;
    fs.st_ctime = ctime;
    u64 atime;
    bs >> atime;
    fs.st_atime = atime;
    u64 mtime;
    bs >> mtime;
    fs.st_mtime = mtime;
    return bs;
}

DataNode OnFileExecRoot(DataArray *da) { return gExecRoot; }
DataNode OnFileRoot(DataArray *da) { return gRoot; }
DataNode OnFileGetExt(DataArray *da) { return FileGetExt(da->Str(1)); }
DataNode OnFileMatch(DataArray *da) { return FileMatch(da->Str(1), da->Str(2)); }

DataNode OnWithFileRoot(DataArray *da) {
    FilePathTracker fpt(da->Str(1));
    int thresh = da->Size() - 1;
    int i;
    for (i = 2; i < thresh; i++) {
        da->Command(i)->Execute(true);
    }
    return da->Evaluate(i);
}

DataNode OnSynchProc(DataArray *) {
    MILO_FAIL("calling synchproc on non-pc platform");
    return "";
}

void OnFrameRateRecurseCB(const char *cc1, const char *cc2) {
    MILO_ASSERT(gFrameRateArray, 0x120);
    String str(cc2);
    str = str.substr(0, str.length() - strlen(FrameRateSuffix()));
    gFrameRateArray->Insert(gFrameRateArray->Size(), str);
}

bool FileExists(const char *iFilename, int iMode, String *str) {
    MILO_ASSERT((iMode & ~FILE_OPEN_NOARK) == 0, 0x2A8);
    File *theFile = NewFile(iFilename, iMode | 0x40002);
    if (theFile) {
        if (str) {
            *str = theFile->Filename();
        }
        delete theFile;
        return true;
    } else
        return false;
}

String UniqueFilename(const char *c1, const char *c2) {
    String ret;
    int i = 0;
    File *file = nullptr;
    do {
        i++;
        ret = MakeString("%s_%06d.%s", c1, i, c2);
        delete file;
        file = NewFile(ret.c_str(), 1);
    } while (file);
    return ret;
}

DataNode OnFileGetDrive(DataArray *a) { return FileGetDrive(a->Str(1)); }
DataNode OnFileGetPath(DataArray *a) { return FileGetPath(a->Str(1)); }
DataNode OnFileGetBase(DataArray *a) { return FileGetBase(a->Str(1)); }
DataNode OnFileAbsolutePath(DataArray *a) { return FileMakePath(a->Str(1), a->Str(2)); }

DataNode OnFileRelativePath(DataArray *a) {
    return FileRelativePath(a->Str(1), a->Str(2));
}

DataNode OnToggleFakeFileErrors(DataArray *a) {
    gFakeFileErrors = !gFakeFileErrors;
    Hmx::Object *cheatDisplay = ObjectDir::Main()->Find<Hmx::Object>("cheat_display");
    if (cheatDisplay) {
        static Message msg("show_bool", "Fake File errors", 0);
        msg[1] = gFakeFileErrors;
        cheatDisplay->Handle(msg, false);
    }
    return 0;
}

void DirListCB(const char *, const char *c) { gDirList.push_back(c); }

void RecursePatternInternal(
    const char *pttn, void (*cb)(char const *, char const *), bool b3, bool b4
) {
    MILO_ASSERT(pttn && pttn[0], 0x5B8);
    String patternStr = pttn;
    int ui1 = patternStr.find_first_of("&", 0);
    int ui2 = patternStr.find_first_of("?*", 0);
    int ui3 = ui1 == FixedString::npos ? patternStr.length() - 1 : ui1;
    if (ui2 != FixedString::npos) {
        ui3 = Min<unsigned int>(ui3, ui2);
    }
    if (b3 && ui1 == FixedString::npos) {
        int len = patternStr.length();
        for (; ui3 < len && patternStr[ui3] != '/' && patternStr[ui3] != '\\'; ui3++)
            ;
        if (ui3 == len) {
            b3 = false;
            for (; ui3 >= 0 && patternStr[ui3] != '/' && patternStr[ui3] != '\\'; ui3--)
                ;
        } else {
            String subStr1 = patternStr.substr(ui3, len - ui3);
            patternStr = patternStr.substr(0, ui3);
            RecursePatternInternal(patternStr.c_str(), DirListCB, false, true);
            std::vector<String> dirLists(gDirList);
            gDirList.clear();
            patternStr = FileGetPath(patternStr.c_str());
            for (int i = 0; i < dirLists.size(); i++) {
                RecursePatternInternal(
                    MakeString("%s/%s%s", patternStr, dirLists[i], subStr1), cb, b3, b4
                );
            }
            return;
        }
    }
    String str48;
    str48 = ui3 <= 0 ? String(".") : patternStr.substr(0, ui3);
    FileEnumerate(str48.c_str(), cb, b3, patternStr.c_str(), b4);
}

void FileRecursePattern(const char *c, void (*cb)(char const *, char const *), bool b3) {
    RecursePatternInternal(c, cb, b3, false);
}

DataNode OnEnumerateFrameRateResults(DataArray *a) {
    DataNode n(new DataArray(0));
    gFrameRateArray = n.Array();
    FileRecursePattern(
        MakeString("ui/framerate/venue_test/*%s", FrameRateSuffix()),
        OnFrameRateRecurseCB,
        false
    );
    gFrameRateArray = nullptr;
    return n;
}

void FileInit() {
    strcpy(gRoot, ".");
    strcpy(gExecRoot, ".");
    strcpy(gSystemRoot, FileMakePath(gExecRoot, "../../system/run"));
    FilePath::Root().Set(gRoot, gRoot);
    DataRegisterFunc("file_root", OnFileRoot);
    DataRegisterFunc("file_exec_root", OnFileExecRoot);
    DataRegisterFunc("file_get_drive", OnFileGetDrive);
    DataRegisterFunc("file_get_path", OnFileGetPath);
    DataRegisterFunc("file_get_base", OnFileGetBase);
    DataRegisterFunc("file_get_ext", OnFileGetExt);
    DataRegisterFunc("file_match", OnFileMatch);
    DataRegisterFunc("file_absolute_path", OnFileAbsolutePath);
    DataRegisterFunc("file_relative_path", OnFileRelativePath);
    DataRegisterFunc("with_file_root", OnWithFileRoot);
    DataRegisterFunc("synch_proc", OnSynchProc);
    DataRegisterFunc("toggle_fake_file_errors", OnToggleFakeFileErrors);
    DataRegisterFunc("enumerate_frame_rate_results", OnEnumerateFrameRateResults);
    HolmesClientInit();
    const char *str = OptionStr("file_order", nullptr);
    if (str && *str) {
        gOpenCaptureFile = NewFile(str, 0x301);
        MILO_ASSERT(gOpenCaptureFile, 0x18F);
    }
    TheDebug.AddExitCallback(FileTerminate);
}

bool FileReadOnly(const char *filepath) { return true; }

File *NewFile(const char *iFilename, int iMode) {
    if (gNullFiles) {
        return new NullFile();
    } else {
        if (!MainThread()) {
            MILO_NOTIFY("NewFile(%s) from !MainThread()", iFilename);
        }
        if (iFilename && *iFilename) {
            char pathBuf[256];
            char loc[256];
            if (iMode & 2) {
                iFilename = FileLocalize(iFilename, loc);
            }
            if (FileIsLocal(iFilename)) {
                iMode |= 0x10000;
            }
            if ((iMode & 2) && !(iMode & 0x20000)) {
                File *all = FileCache::GetFileAll(iFilename);
                if (all) {
                    return all;
                }
            }
            File *theFile = nullptr;
            if (UsingCD() && (iMode & 2) && !(iMode & 0x10000)) {
                theFile = new ArkFile(iFilename, iMode);
            } else {
                iMode &= ~0x30000;
                theFile = AsyncFile::New(iFilename, iMode);
            }
            if (theFile->Fail()) {
                delete theFile;
                return nullptr;
            } else {
                if (!gOpenCaptureFile || !(iMode & 2) || gCaptureFileMode >= 1U) {
                    return theFile;
                }
                sprintf(pathBuf, "'%s'\n", FileMakePath(".", iFilename));
                gOpenCaptureFile->Write(pathBuf, strlen(pathBuf));
                gOpenCaptureFile->Flush();
                return theFile;
            }
        }
        return nullptr;
    }
}
