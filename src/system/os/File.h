#pragma once
#include "utl/Str.h"
#include <stdlib.h>

#define FILE_OPEN_WRITE 1
#define FILE_OPEN_READ 2
#define FILE_OPEN_NOARK 0x10000

#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1

extern bool gNullFiles;
extern bool gFakeFileErrors;

class File {
public:
    File() {}
    virtual ~File() {}
    virtual class String Filename() const {
        class String str;
        return str;
    }
    virtual int Read(void *, int) = 0;
    virtual bool ReadAsync(void *, int) = 0;
    virtual int Write(const void *, int) = 0;
    virtual bool WriteAsync(const void *, int) { return false; }
    virtual int Seek(int, int) = 0;
    virtual int Tell() = 0;
    virtual void Flush() = 0;
    virtual bool Eof() = 0;
    virtual bool Fail() = 0;
    virtual int Size() = 0;
    virtual int UncompressedSize() = 0;
    virtual bool ReadDone(int &) = 0;
    virtual bool WriteDone(int &i) {
        i = 0;
        return true;
    }
    virtual bool GetFileHandle(void *&) = 0;
    virtual bool Truncate(int) { return 0; }

    bool ReadDone() {
        int x;
        return ReadDone(x);
    }
    bool WriteDone() {
        int x;
        return WriteDone(x);
    }

    static int sOpenCount;
    static const int MaxFileNameLen;
};

class NullFile : public File {
public:
    NullFile() {}
    virtual ~NullFile() {}
    virtual int Read(void *v, int i) {
        memset(v, 0, i);
        return i;
    }
    virtual bool ReadAsync(void *v, int i) {
        Read(v, i);
        return true;
    }
    virtual int Write(const void *v, int i) { return i; }
    virtual int Seek(int, int) { return 0; }
    virtual int Tell() { return 0; }
    virtual void Flush() {}
    virtual bool Eof() { return true; }
    virtual bool Fail() { return false; }
    virtual int Size() { return 0; }
    virtual int UncompressedSize() { return 0; }
    virtual bool ReadDone(int &i) {
        i = 0;
        return true;
    }
    virtual bool GetFileHandle(void *&) { return false; }
};

struct FileStat {
    unsigned int st_mode;
    unsigned int st_size;
    unsigned long st_ctime;
    unsigned long st_atime;
    unsigned long st_mtime;
};

extern "C" {

void FileInit();
void FileTerminate();

const char *FileMakePath(const char *root, const char *file);
const char *FileRelativePath(const char *root, const char *filepath);
const char *FileGetPath(const char *file);
/** Given a file, get the extension. */
const char *FileGetExt(const char *file);
/** Given a file, get the drive the file resides in. */
const char *FileGetDrive(const char *file);
/** Given a file, get the base. (i.e. the file name, minus the extension.) */
const char *FileGetBase(const char *file);
/** Given a file, get the name. (i.e. the file name, plus the extension.) */
const char *FileGetName(const char *file);

const char *FileMakePathBuf(const char *iRoot, const char *iFilepath, char *oBuf);
const char *FileRelativePathBuf(const char *, const char *, char *);

/** Given a file, get the corresponding file stats.
 * @param [in] iFilename The file.
 * @param [out] iBuffer The file's stats.
 * @returns 0 on success, -1 on failure.
 */
int FileGetStat(const char *iFilename, FileStat *iBuffer);

// int FileOpen(const char *iFilename, int iMode);
// int FileClose(int iFd);

/** Try to delete a file.
 * @param [in] iFilename The file to be deleted.
 * @returns Nonzero on success, zero on failure.
 */
int FileDelete(const char *iFilename);

// int FileWrite(int iFd, void *iBuff, unsigned int iLen);

/** Try to make a directory.
 * @param [in] iDirname The directory to be created.
 * @returns Nonzero on success, zero on failure.
 */
int FileMkDir(const char *iDirname);

void FileDiscSpinUp();

/** Normalize the path of a given file.
    (i.e. change '\'s to '/'s and make every letter lowercase.)
 */
void FileNormalizePath(const char *file);

bool FileMatch(const char *, const char *);
void FileEnumerate(
    const char *dir,
    void (*cb)(const char *, const char *),
    bool recurse,
    const char *pattern,
    bool
);
void FileRecursePattern(const char *, void (*)(char const *, char const *), bool);

class BinStream &operator>>(class BinStream &, FileStat &);
}

File *NewFile(const char *iFilename, int iMode);

const char *FileRoot();
const char *FileExecRoot();
const char *FileSystemRoot();

/** Given an input file name, get the full qualified file name: drive, folders, and all.
    e.g. C:\Users\Username\Documents\file.txt */
void FileQualifiedFilename(String &oNewName, const char *iOldName);
/** Given an input file name, get the full qualified file name: drive, folders, and all.
    e.g. C:\Users\Username\Documents\file.txt */
void FileQualifiedFilename(char *oNewName, int unusedLmao, const char *iOldName);

const char *FileLocalize(const char *iFilename, char *buffer);
bool FileReadOnly(const char *filepath);
bool FileExists(const char *iFilename, int iMode, String *);
/** Is this file inside the system's drive? */
bool FileIsLocal(const char *file);

String UniqueFilename(const char *c1, const char *c2);
