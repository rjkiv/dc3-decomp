#include "os/CDReader.h"
#include "os/Archive.h"
#include "os/File.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "xdk/xapilibi/errhandlingapi.h"
#include "xdk/xapilibi/fileapi.h"
#include <vector>
#include "xdk/XAPILIB.h"

namespace {
    OVERLAPPED gOverlapped;
    int gPendingFile;
    int gErrorCode;
    std::vector<void *> gArkFiles;
    std::vector<void *> gExternalArkFiles;

    void DiskErrorLoop() {
        MILO_NOTIFY("Disc error: %d", gErrorCode);
        ThePlatformMgr.SetDiskError(kDiskError);
    }

    int ArkFilesInit() {
        if (gArkFiles.empty()) {
            gArkFiles.resize(TheArchive->NumArkFiles());
            gExternalArkFiles.resize(TheArchive->NumArkFiles());
            gOverlapped.Internal = 0;
            gOverlapped.InternalHigh = 0;
            gOverlapped.Offset = 0;
            gOverlapped.OffsetHigh = 0;
            gOverlapped.hEvent = 0;
            for (int i = 0; i < gArkFiles.size(); i++) {
                const char *arkFileName = TheArchive->GetArkfileName(i);
                String fullPath;
                FileQualifiedFilename(fullPath, arkFileName);
                gArkFiles[i] = CreateFileA(
                    fullPath.c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                    nullptr
                );
                if (!gArkFiles[i]) {
                    gErrorCode = GetLastError();
                    DiskErrorLoop();
                    return 1;
                }
                gExternalArkFiles[i] = CreateFileA(
                    fullPath.c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL,
                    nullptr
                );
                if (!gExternalArkFiles[i]) {
                    gErrorCode = GetLastError();
                    DiskErrorLoop();
                    return 1;
                }
            }
        }
        return 0;
    }
}

bool CDReadDone() {
    if (gFakeFileErrors || !UsingCD()) {
        gErrorCode = 0x45D;
    } else {
        DWORD bytes;
        if (GetOverlappedResult(gArkFiles[gPendingFile], &gOverlapped, &bytes, false)) {
            return true;
        }
        gErrorCode = GetLastError();
        if (gErrorCode == ERROR_IO_PENDING || gErrorCode == ERROR_IO_INCOMPLETE) {
            return false;
        }
    }
    DiskErrorLoop();
    return true;
}

int CDRead(int arkFile, int i2, int i3, void *v) {
    if (gFakeFileErrors || !UsingCD()) {
        gErrorCode = 0x45D;
        DiskErrorLoop();
        return 1;
    } else {
        int ret = ArkFilesInit();
        if (ret) {
            return ret;
        }
        gOverlapped.OffsetHigh = (i2 << 0xB) >> 0x20;
        gOverlapped.Offset = i2 << 0xB;
        if (!ReadFile(gArkFiles[arkFile], v, i3 << 0xB, nullptr, &gOverlapped)) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING || err == ERROR_IO_INCOMPLETE) {
                MILO_NOTIFY("Disc error: ERROR_IO_INCOMPLETE, ignoring");
                gPendingFile = arkFile;
                return 0;
            } else {
                gErrorCode = err;
                DiskErrorLoop();
                return 1;
            }
        }
    }
}

bool CDReadExternal(void *&v, int i, u64 u) {
    if (ArkFilesInit() != 0) {
        return false;
    } else {
        v = gExternalArkFiles[i];
        LONG l = u;
        SetFilePointer(v, u, &l, 0);
        return true;
    }
}
