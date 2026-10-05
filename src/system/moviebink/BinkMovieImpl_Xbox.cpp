#include "moviebink/BinkMovieImpl.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Str.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/fileapi.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/minwinbase.h"
#include "xdk/xbdm/xbdm.h"

// you'll never guess what this does
void MakeDir(const char *dir) {
    String path(dir);
    String parent(FileGetPath(path.c_str()));
    if (parent != path) {
        MakeDir(parent.c_str());
    }
    CreateDirectoryA(path.c_str(), nullptr);
}

// copies a bink movie to the devkit drive and points to it
// returns true if it could successfully copy to the devkit drive, false if not
bool BinkMovieImpl::PlatformCacheFile(const char *file) {
	// if we are running off disc or some other unknown condition, do nothing
    if (UsingCD() || unk18) {
        return true;
    }
	
	// make the devkit drive available
    DmMapDevkitDrive();
    FileStat stat;
    if (FileGetStat(mName.c_str(), &stat) < 0) {
        return false;
    }
    // conversion of unix time to FILETIME?
    LONGLONG writeTime = ((long)stat.st_mtime + 11644473600) * 10000000;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)writeTime;
    ft.dwHighDateTime = (DWORD)(writeTime >> 32);

    String cached(FileMakePath("DEVKIT:", file));
    FileQualifiedFilename(cached, cached.c_str());
    WIN32_FILE_ATTRIBUTE_DATA attr;
	
    if (!GetFileAttributesExA(cached.c_str(), GetFileExInfoStandard, &attr)|| CompareFileTime(&attr.ftLastWriteTime, &ft) < 0) {
        File *fin = NewFile(mName.c_str(), 2);
        MILO_ASSERT(fin, 0x46);
        MakeDir(FileGetPath(cached.c_str()));
        HANDLE h = CreateFileA(cached.c_str(), GENERIC_WRITE, 0,nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		
		// we couldn't get a handle
        if (h == INVALID_HANDLE_VALUE) {
            delete fin;
            return false;
        }
		
		
		// read and write the movie in 64kb chunks
        char buf[0x10000];
        while (!fin->Eof()) {
            int bytes = fin->Read(buf, sizeof(buf));
            DWORD written;
            WriteFile(h, buf, bytes, &written, nullptr);
        }
        CloseHandle(h);
        delete fin;
    }
	
	// make it now point to the newly copied file on devkit drive
    mName = cached;
    return true;
}
