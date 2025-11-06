#pragma once
#include "os/File.h"
#include "utl/BinStream.h"
#include "utl/Str.h"

class FilePath : public String {
private:
    static FilePath sRoot;
    static FilePath sNull;

public:
    FilePath() : String() {}
    FilePath(const char *str);
    FilePath(const char *cc, const char *cc2);

    void Set(const char *, const char *);

    // const char *FilePathRelativeToRoot() {
    //     return FileRelativePath(sRoot.c_str(), this->c_str());
    // }
    void SetRoot(const char *str) { Set(sRoot.c_str(), str); }
    static FilePath &Root() { return sRoot; }
    static FilePath &Null() { return sNull; }
};

inline TextStream &operator<<(TextStream &ts, FilePath &fp) {
    return ts << FileRelativePath(FilePath::Root().c_str(), fp.c_str());
}

// inline void ResetRoot(const char *path) { FilePath::sRoot.Set(FileRoot(), path); }

inline BinStream &operator<<(BinStream &bs, const FilePath &fp) {
    return bs << FileRelativePath(FilePath::Root().c_str(), fp.c_str());
}

BinStream &operator>>(BinStream &, FilePath &);

// inline BinStream &operator>>(BinStream &bs, FilePath &fp) {
//     char buf[0x100];
//     bs.ReadString(buf, 0x100);
//     fp.SetRoot(buf);
//     return bs;
// }

class FilePathTracker {
public:
    FilePathTracker(const char *root) {
        mOldRoot = FilePath::Root();
        FilePath::Root().Set(FileRoot(), root);
    }

    ~FilePathTracker() { FilePath::Root() = mOldRoot; }

    FilePath mOldRoot;
};
