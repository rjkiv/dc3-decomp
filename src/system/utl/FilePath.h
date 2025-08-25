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
    // virtual ~FilePath() {} // i guess hmx didn't have this for FilePath?

    void Set(const char *, const char *);

    // const char *FilePathRelativeToRoot() {
    //     return FileRelativePath(sRoot.c_str(), this->c_str());
    // }
    // void SetRoot(const char *str) { Set(sRoot.c_str(), str); }
    static FilePath &Root() { return sRoot; }
};

inline TextStream &operator<<(TextStream &ts, FilePath &fp) {
    return ts << FileRelativePath(FilePath::Root().c_str(), fp.c_str());
}

// inline void ResetRoot(const char *path) { FilePath::sRoot.Set(FileRoot(), path); }

inline BinStream &operator<<(BinStream &bs, const FilePath &fp) {
    bs << FileRelativePath(FilePath::Root().c_str(), fp.c_str());
    return bs;
    //       pcVar1 = FileRelativePath(FilePath::sRoot.c_str(),*(this + 0x68));
    //   this_00 = BinStream::operator<<(param_1,pcVar1);
}

// inline BinStream &operator>>(BinStream &bs, FilePath &fp) {
//     char buf[0x100];
//     bs.ReadString(buf, 0x100);
//     fp.SetRoot(buf);
//     return bs;
// }

class FilePathTracker {
public:
    FilePathTracker(const char *root);

    ~FilePathTracker();

    FilePath mOldRoot;
};
