#include "HolmesClient.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "xdk/XAPILIB.h"
#include <cstdio>

bool FileIsLocal(const char *file) {
    const char *drive = FileGetDrive(file);
    MILO_ASSERT(!strieq(drive, "game"), 0x24);
    return strlen(drive) > 1;
}

int FileGetStat(const char *iFilename, FileStat *iBuffer) {
    String fullName;
    FileQualifiedFilename(fullName, iFilename);
    if (!UsingCD() && !FileIsLocal(fullName.c_str())) {
        return HolmesClientGetStat(fullName.c_str(), *iBuffer);
    } else {
        FileStat curStat;
        BOOL res =
            GetFileAttributesExA(fullName.c_str(), GetFileExInfoStandard, &curStat);
        iBuffer->st_ctime = curStat.st_ctime;
        iBuffer->st_atime = curStat.st_atime;
        iBuffer->st_mode = curStat.st_mode;
        iBuffer->st_mtime = curStat.st_mtime;
        iBuffer->st_size = curStat.st_size;
        return res ? 0 : -1;
    }
}

int FileDelete(const char *iFilename) {
    String str;
    FileQualifiedFilename(str, iFilename);
    if (FileIsLocal(str.c_str())) {
        return !DeleteFileA(str.c_str());
    } else {
        return HolmesClientDelete(str.c_str());
    }
}

int FileMkDir(const char *iDirname) {
    String str;
    FileQualifiedFilename(str, iDirname);
    if (FileIsLocal(str.c_str())) {
        return CreateDirectoryA(str.c_str(), nullptr);
    } else {
        return HolmesClientMkDir(str.c_str());
    }
}

void FileQualifiedFilename(char *out, int, const char *in) {
    MILO_ASSERT(in && out, 0x121);
    String str(in);
    const char *inStr = str.c_str();
    const char *path = UsingCD() ? "d:" : HolmesFileShare();
    char buf[256];
    path = FileMakePathBuf(path, inStr, buf);
    strcpy(out, path);
    for (char *p = out; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\\';
        }
    }
}

void FileEnumerate(
    const char *dir,
    void (*cb)(const char *, const char *),
    bool recurse,
    const char *pattern,
    bool b2
) {
    bool local = FileIsLocal(dir);
    if (UsingCD() && !local)
        TheArchive->Enumerate(dir, cb, recurse, pattern);
    else if (!UsingCD() && !local)
        HolmesClientEnumerate(dir, cb, recurse, pattern, b2);
    else {
        char qualified[256];
        FileQualifiedFilename(qualified, 0x100, dir);
        WIN32_FIND_DATAA lpFindFileData;
        HANDLE hFindFile =
            FindFirstFileA(MakeString("%s\\*", qualified), &lpFindFileData);
        if (hFindFile == (HANDLE)-1) {
            DWORD err = GetLastError();
            switch (err) {
            case ERROR_FILE_NOT_FOUND:
                MILO_LOG("FileEnumerate: path empty %s\n", qualified);
                break;
            case ERROR_PATH_NOT_FOUND:
                MILO_LOG("FileEnumerate: path not found %s\n", qualified);
                break;
            case ERROR_INVALID_NAME:
                MILO_LOG("FileEnumerate: path invalid name %s\n", qualified);
                break;
            default:
                MILO_LOG(
                    "FileEnumerate: last error: %d (see winerror.h) for %s\n",
                    err,
                    qualified
                );
                break;
            }
        } else {
            char buf150[256];
            while (true) {
                if (strcmp(qualified, ".") == 0) {
                    sprintf(buf150, "%s", lpFindFileData.cFileName);
                } else {
                    sprintf(buf150, "%s/%s", qualified, lpFindFileData.cFileName);
                }
                if (lpFindFileData.dwFileAttributes & 0x10) {
                    if (strcmp(lpFindFileData.cFileName, "..")) {
                        if (b2 && FileMatch(buf150, pattern)) {
                            cb(qualified, lpFindFileData.cFileName);
                        }
                        if (recurse) {
                            FileEnumerate(buf150, cb, recurse, pattern, b2);
                        }
                    }
                } else {
                    if (!b2 && (!pattern || FileMatch(buf150, pattern))) {
                        cb(qualified, lpFindFileData.cFileName);
                    }
                }
                if (!FindNextFileA(hFindFile, &lpFindFileData)) {
                    CloseHandle(hFindFile);
                    return;
                }
            }
        }
    }
}
