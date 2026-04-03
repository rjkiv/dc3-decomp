#include "Memory.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/MemMgr.h"
#include "utl/MemTrack.h"
#include "utl/MemTracker.h"
#include "utl/Symbol.h"
#include "utl/TextStream.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/xbox.h"
#include <cstdio>

XALLOC_ATTRIBUTES Attr(DWORD dw) { return *reinterpret_cast<XALLOC_ATTRIBUTES *>(&dw); }

namespace {
    int gPhysicalUsage;
    const char *gPhysicalType = gNullStr;

    const char *AllocType(DWORD dwAllocAttributes) {
        DWORD type = Attr(dwAllocAttributes).dwObjectType;
        bool notPhys = Attr(dwAllocAttributes).dwMemoryType;
        switch (type) {
        case 0:
            if (notPhys) {
                if (gPhysicalType != gNullStr) {
                    return gPhysicalType;
                } else {
                    return "XTL(phys):D3D";
                }
            } else {
                return "XTL:D3D";
            }
        case 1:
            return !notPhys ? "XTL:D3DX" : "XTL(phys):D3DX";
        case 2:
            return !notPhys ? "XTL:XAUDIO" : "XTL(phys):XAUDIO";
        case 3:
            return !notPhys ? "XTL:XAPI" : "XTL(phys):XAPI";
        case 4:
            return !notPhys ? "XTL:XACT" : "XTL(phys):XACT";
        case 5:
            return !notPhys ? "XTL:XBOXKERNEL" : "XTL(phys):XBOXKERNEL";
        case 6:
            return !notPhys ? "XTL:XBDM" : "XTL(phys):XBDM";
        case 7:
            return !notPhys ? "XTL:XGRAPHICS" : "XTL(phys):XGRAPHICS";
        case 8:
            return !notPhys ? "XTL:XONLINE" : "XTL(phys):XONLINE";
        case 9:
            return !notPhys ? "XTL:XVOICE" : "XTL(phys):XVOICE";
        case 10:
            return !notPhys ? "XTL:XHV" : "XTL(phys):XHV";
        case 0xb:
            return !notPhys ? "XTL:USB" : "XTL(phys):USB";
        case 0xc:
            return !notPhys ? "XTL:XMV" : "XTL(phys):XMV";
        case 0xd:
            return !notPhys ? "XTL:SHADERCOMPILER" : "XTL(phys):SHADERCOMPILER";
        case 0xe:
            return !notPhys ? "XTL:XUI" : "XTL(phys):XUI";
        case 0xf:
            return !notPhys ? "XTL:XASYNC" : "XTL(phys):XASYNC";
        case 0x10:
            return !notPhys ? "XTL:XCAM" : "XTL(phys):XCAM";
        case 0x11:
            return !notPhys ? "XTL:XVIS" : "XTL(phys):XVIS";
        case 0x12:
            return !notPhys ? "XTL:XIME" : "XTL(phys):XIME";
        case 0x13:
            return !notPhys ? "XTL:XFILECACHE" : "XTL(phys):XFILECACHE";
        case 0x14:
            return !notPhys ? "XTL:XRN" : "XTL(phys):XRN";
        case 0x15:
            return !notPhys ? "XTL:XMCORE" : "XTL(phys):XMCORE";
        case 0x16:
            return !notPhys ? "XTL:XMASSIVE" : "XTL(phys):XMASSIVE";
        case 0x17:
            return !notPhys ? "XTL:XAUDIO2" : "XTL(phys):XAUDIO2";
        case 0x18:
            return !notPhys ? "XTL:XAVATAR" : "XTL(phys):XAVATAR";
        case 0x19:
            return !notPhys ? "XTL:XLSP" : "XTL(phys):XLSP";
        case 0x1a:
            return !notPhys ? "XTL:D3DAlloc" : "XTL(phys):D3DAlloc";
        case 0x1b:
            return !notPhys ? "XTL:NUISPEECH" : "XTL(phys):NUISPEECH";
        case 0x1c:
            return !notPhys ? "XTL:NuiApi" : "XTL(phys):NuiApi";
        case 0x1d:
            return !notPhys ? "XTL:NuiIdentity" : "XTL(phys):NuiIdentity";
        case 0x3e:
            return !notPhys ? "XTL:NuiApi_LargePageReadWrite"
                            : "XTL(phys):NuiApi_LargePageReadWrite";
        default:
            if (type <= 0x7F) {
                return !notPhys ? "XTL:Game" : "XTL(phys):Game";
            } else if (type >= 0xC0) {
                return !notPhys ? "XTL:Middleware" : "XTL(phys):Middleware";
            } else {
                return "XTL:Unknown";
            }
        }
    }

    int AllocAlign(DWORD);

    void MemAllocFailed(unsigned long requestedBytes, bool physical) {
        MEMORYSTATUS status;
        char buffer[2048];
        GlobalMemoryStatus(&status);
        MemDeltaFullReport();
        if (gMemTracker && !gMemTracker->GetHeapOnly()) {
            FILE *file = fopen("devkit:\\out_of_mem_alloc_info.csv", "w");
            if (file) {
                MemTracker::SpitAllocInfo(file);
                fclose(file);
            }
        }
        Hx_snprintf(
            buffer,
            2048,
            "Allocation failure, \"%s\", want %d bytes\n   Free (bytes) =  %8d\n   Usage (bytes) =%8d\n",
            physical ? "physical" : "XMV",
            requestedBytes,
            status.dwAvailPhys,
            gPhysicalUsage
        );
        MemPrintOverview(-3, &buffer[strlen(buffer)]);
        MILO_FAIL(buffer);
    }

}

PhysMemTypeTracker::PhysMemTypeTracker(Symbol type) {
    if (gPhysicalType == gNullStr) {
        gPhysicalType = type.Str();
        unk0 = true;
    } else {
        unk0 = false;
    }
}

PhysMemTypeTracker::~PhysMemTypeTracker() {
    if (unk0) {
        gPhysicalType = gNullStr;
    }
}

void *PhysicalAlloc(int sizeBytes) {
    void *alloc = XPhysicalAlloc(sizeBytes, -1, 0, 4);
    if (alloc) {
        gPhysicalUsage += XPhysicalSize(alloc);
    } else if (sizeBytes != 0) {
        MemAllocFailed(sizeBytes, true);
    }
    return alloc;
}

void PhysicalFree(void *address) {
    if (address != 0) {
        gPhysicalUsage -= XPhysicalSize(address);
    }
    XPhysicalFree(address);
}

void *PhysicalAllocTracked(
    unsigned long requestedSize,
    unsigned long u2,
    const char *file,
    int line,
    const char *type
) {
    int actualSize = 0;
    void *alloc = XPhysicalAlloc(requestedSize, -1, 0, u2);
    if (alloc) {
        actualSize = XPhysicalSize(alloc);
        gPhysicalUsage += actualSize;
    } else if (requestedSize != 0) {
        MemAllocFailed(requestedSize, true);
    }
    MemTrackAlloc(requestedSize, actualSize, type, alloc, false, 0, file, line);
    return alloc;
}

void PhysicalFreeTracked(void *address, const char *, int, const char *) {
    if (address != 0) {
        gPhysicalUsage -= XPhysicalSize(address);
    }
    XPhysicalFree(address);
    MemTrackFree(address);
}

int PhysicalUsage() { return gPhysicalUsage; }

int ForceLinkXMemFuncs() { return 0x2A; }

VOID *XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes) {
    VOID *ptr;
    if ((dwAllocAttributes & 0x80000000) == 0
        && (dwAllocAttributes & 0xFF0000) != 0x8C0000) {
        MILO_ASSERT(Attr(dwAllocAttributes).dwMemoryProtect == XALLOC_MEMPROTECT_READWRITE, 0xF9);
        ptr = _MemAllocTemp(
            dwSize,
            __FILE__,
            0x107,
            AllocType(dwAllocAttributes),
            AllocAlign(dwAllocAttributes)
        );
        if (dwAllocAttributes & 0x4000) {
            MILO_ASSERT(ptr, 0x10D);
        }
        if (dwAllocAttributes & 0x40000000 && ptr) {
            memset(ptr, 0, dwSize);
        }
    } else {
        ptr = XMemAllocDefault(dwSize, dwAllocAttributes);
        if (!ptr) {
            MemAllocFailed(dwSize, dwAllocAttributes & 0x80000000);
        }
        int sizeDefault = XMemSizeDefault(ptr, dwAllocAttributes);
        gPhysicalUsage += sizeDefault;
        MemTrackAlloc(
            dwSize, sizeDefault, AllocType(dwAllocAttributes), ptr, false, 0, __FILE__, 0xF0
        );
    }
    return ptr;
}

VOID XMemFree(LPVOID lpHandle, DWORD dwFreeAttributes) {
    if ((dwFreeAttributes & 0x80000000) == 0
        && (dwFreeAttributes & 0xFF0000) != 0x8C0000) {
        MemFree(lpHandle);
    } else {
        if (lpHandle) {
            gPhysicalUsage -= XMemSizeDefault(lpHandle, dwFreeAttributes);
        }
        MemTrackFree(lpHandle);
        XMemFreeDefault(lpHandle, dwFreeAttributes);
    }
}

INT XMemSize(LPVOID lpHandle, DWORD dwSizeAttributes) {
    if ((dwSizeAttributes & 0x80000000) == 0
        && (dwSizeAttributes & 0xFF0000) != 0x8C0000) {
        return MemAllocSize(lpHandle);
    } else {
        return XMemSizeDefault(lpHandle, dwSizeAttributes);
    }
}
