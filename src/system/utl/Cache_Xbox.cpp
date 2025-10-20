#include "utl/Cache_Xbox.h"
#include "os/Debug.h"
#include "utl/Cache.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include <cstring>

CacheID::~CacheID() {}

CacheIDXbox::CacheIDXbox() { memset(unkc, 0, sizeof(unkc)); }

CacheIDXbox::CacheIDXbox(CacheIDXbox const &c) : mStrCacheName(c.mStrCacheName) {
    memcpy(unkc, c.unkc, sizeof(unkc));
}

CacheIDXbox::~CacheIDXbox() {}

char const *CacheIDXbox::GetCachePath(char const *c) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCachePath - mStrCacheName is empty.\n");
    }

    if (c == nullptr) {
        return MakeString("%s:\\", mStrCacheName.c_str());
    } else {
        String s = c;
        s.ReplaceAll('/', '\\');

        return s.c_str();
    }
}

char const *CacheIDXbox::GetCacheSearchPath(char const *c) {
    if (mStrCacheName.empty()) {
        MILO_FAIL("CacheID::GetCacheSearchPath() - mStrCacheName is empty.\n");
    }

    if (c == nullptr) {
        return MakeString("%s:\\*", mStrCacheName.c_str());
    } else {
        return GetCachePath(c);
    }
}

CacheXbox::CacheXbox(CacheIDXbox const &c) : unk10(c) {}

CacheXbox::~CacheXbox() {}

bool CacheXbox::IsConnectedSync() { return false; }

bool CacheXbox::GetFileSizeAsync(char const *, unsigned int *, Hmx::Object *) {
    return false;
}

bool CacheXbox::ReadAsync(char const *, void *, unsigned int, Hmx::Object *) {
    return false;
}

bool CacheXbox::DeleteAsync(char const *, Hmx::Object *) { return false; }

bool CacheXbox::GetFreeSpaceSync(unsigned long long *) { return false; }

bool CacheXbox::GetDirectoryAsync(
    char const *, std::vector<CacheDirEntry> *, Hmx::Object *
) {
    return false;
}

bool CacheXbox::DeleteSync(char const *) { return false; }

bool CacheXbox::WriteAsync(char const *, void *, unsigned int, Hmx::Object *) {
    return false;
}

void CacheXbox::ThreadDone(int) {}

int CacheXbox::ThreadStart() { return 1; }

int CacheXbox::ThreadGetFileSize() { return 1; }

int CacheXbox::ThreadRead() { return 1; }

int CacheXbox::ThreadWrite() { return 1; }

bool CacheXbox::DeleteParentDirs(String) { return 1; }

int CacheXbox::ThreadDelete() { return 1; }

int CacheXbox::ThreadGetDir(String, String) { return 1; }
