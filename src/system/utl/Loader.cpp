#include "utl/Loader.h"
#include "Loader.h"
#include "MemTrack.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"
#include "utl/Option.h"
#include "utl/Std.h"

int gLoadCount;
LoadMgr TheLoadMgr;

struct LoaderGlitchInfo {
    String loaderFile; // 0x0
    const char *loaderState; // 0x8
    const char *frontLoaderState; // 0xc
    LoaderPos loaderPos; // 0x10
};

void FrontLoaderGlitchCB(float f1, void *v) {
    LoaderGlitchInfo *info = static_cast<LoaderGlitchInfo *>(v);
    MILO_LOG(
        "Loader %s %s took %f (%s to %s)\n",
        LoadMgr::LoaderPosString(info->loaderPos, true),
        info->loaderFile,
        f1,
        info->loaderState,
        info->frontLoaderState
    );
}

__declspec(noinline) const char *WhiteSpace(int count) {
    const int len = 0x80;
    MILO_ASSERT(count < len, 0x179);
    MILO_ASSERT(count >= 0, 0x17A);
    return &"                                                                                                                                "
        [len - count];
}

#pragma region Loader

Loader::Loader(const FilePath &fp, LoaderPos pos)
    : unk4(0), mPos(pos), mFile(fp), mLoadTimeStartMs(-1), mHeap(GetCurrentHeapNum()) {
    MILO_ASSERT(MemNumHeaps() == 0 || (mHeap != kNoHeap && mHeap != kSystemHeap), 0x1F0);
    TheLoadMgr.Loaders().push_front(this);
    if (mPos == kLoadFront) {
        TheLoadMgr.Loading().push_front(this);
    } else if (mPos == kLoadStayBack) {
        TheLoadMgr.Loading().push_back(this);
    } else {
        auto it = TheLoadMgr.Loading().end();
        while (it != TheLoadMgr.Loading().begin()) {
            it--;
            if ((*it)->GetPos() <= kLoadBack) {
                it++;
                break;
            }
        }
        TheLoadMgr.Loading().insert(it, this);
    }
}

Loader::~Loader() {
    TheLoadMgr.Loading().remove(this);
    TheLoadMgr.Loaders().remove(this);
    if (mLoadTimeStartMs != -1) {
        gLoadCount--;
    }
}

#pragma endregion
#pragma region FileLoader

FileLoader::FileLoader(
    const FilePath &fp,
    const char *cc,
    LoaderPos pos,
    int i4,
    bool b5,
    bool b6,
    BinStream *bs,
    const char *cc8
)
    : Loader(fp, pos), mFile(nullptr), mStream(bs), mBuffer(nullptr), mBufLen(0),
      mAccessed(false), mTemp(b5), mWarn(b6), mFlags(i4), mFilename(cc), unk3c(0),
      unk40(-1), mState(nullptr) {
    unk44 = cc8 ? cc8 : "main";
    if (mStream) {
        mState = &FileLoader::LoadStream;
    } else {
        mState = &FileLoader::OpenFile;
    }
}

FileLoader::~FileLoader() {
    if (!mAccessed) {
        MemFree((void *)mBuffer);
        delete mFile;
    }
}

const char *FileLoader::DebugText() {
    return MakeString("FileLoader: %s", LoaderFile().c_str());
}
bool FileLoader::IsLoaded() const { return mState == &FileLoader::DoneLoading; }
void FileLoader::PollLoading() { (this->*mState)(); }
int FileLoader::GetSize() { return mBufLen; }
void FileLoader::DoneLoading() {}

void FileLoader::AllocBuffer() {
    const char *filename = mFilename.c_str();
    MemHeapTracker tmp(MemFindHeap(unk44.c_str()));
    BeginMemTrackFileName(filename);
    if (mTemp) {
        mBuffer =
            (const char *)_MemAllocTemp(mBufLen, __FILE__, 0x241, "Temp Resource", 0);
    } else {
        mBuffer = (const char *)MemAlloc(
            mBufLen, __FILE__, 0x243, Symbol(FileGetExt(filename)).Str()
        );
    }
    EndMemTrackFileName();
}

void FileLoader::LoadFile() {
    int asdf;
    if (mFile->ReadDone(asdf)) {
        if (mFile->Fail()) {
            mBufLen = 0;
            MemFree((void *)mBuffer);
            mBuffer = nullptr;
        }
        RELEASE(mFile);
        mState = &FileLoader::DoneLoading;
    }
}

void FileLoader::LoadStream() {
    while (mStream->Eof() != NotEof) {
        if (TheLoadMgr.CheckSplit())
            return;
    }
    if (!mBuffer) {
        int size;
        *mStream >> size;
        if (size == -1) {
            *mStream >> unk40;
            *mStream >> mBufLen;
        } else {
            unk40 = 0;
            mBufLen = size;
        }
        AllocBuffer();
    }
    int i2 = unk40 > 0 ? 0x10000 : mBufLen;
    while (true) {
        int i3 = Min(mBufLen - unk3c, i2);
        while (mStream->Eof() != NotEof) {
            if (TheLoadMgr.CheckSplit())
                return;
        }
        if (i3 == 0)
            break;
        mStream->Read((void *)(mBuffer + unk3c), i3);
        unk3c += i3;
    }
    mState = &FileLoader::DoneLoading;
}

void FileLoader::OpenFile() {
    Archive *old = TheArchive;
    const char *fname = mFilename.c_str();
    bool oldusingcd = UsingCD();
    bool b1 = gHostFile && FileMatch(fname, gHostFile);
    if (b1) {
        SetUsingCD(false);
        TheArchive = nullptr;
    }
    mFile = NewFile(fname, mFlags | 2);
    if (b1) {
        SetUsingCD(oldusingcd);
        TheArchive = old;
    }

    if (!mFile && *fname != '\0' && mWarn) {
        MILO_NOTIFY(
            "Could not load: %s (actually %s)",
            FileLocalize(Loader::mFile.c_str(), 0),
            fname
        );
    }
    if (mFile && !mFile->Fail()) {
        mBufLen = mFile->Size();
        AllocBuffer();
        mFile->ReadAsync((void *)mBuffer, mBufLen);
        mState = &FileLoader::LoadFile;
    } else {
        mState = &FileLoader::DoneLoading;
    }
}

char *FileLoader::GetBuffer(int *size) {
    MILO_ASSERT(IsLoaded(), 0x2B7);
    if (size)
        *size = mBufLen;
    mAccessed = true;
    return (char *)mBuffer;
}

void FileLoader::SaveData(BinStream &bs, void *v, int size) {
    MILO_ASSERT(size >= 0, 0x314);
    bs << -1;
    bs << 1;
    bs << size;
    int seek = 0;
    while (true) {
        int curBytes = size - seek;
        if (curBytes > 0x10000) {
            curBytes = 0x10000;
        } else if (curBytes == 0)
            return;
        const char *c = (char *)v;
        bs.Write(c + seek, curBytes);
        seek += curBytes;
        MarkChunk(bs);
    };
}

#pragma endregion
#pragma region LoadMgr

LoadMgr::LoadMgr()
    : mPlatform(kPlatformXBox), mEditMode(false), mCacheMode(false), mPeriod(10.0f),
      mAsyncUnload(0), mLoaderPos(kLoadFront) {}

LoadMgr::~LoadMgr() {}

void LoadMgr::StartAsyncUnload() { mAsyncUnload++; }
void LoadMgr::FinishAsyncUnload() { mAsyncUnload--; }
int LoadMgr::AsyncUnload() const { return mAsyncUnload; }

const char *LoadMgr::LoaderPosString(LoaderPos pos, bool abbrev) {
    static const char *names[4] = {
        "kLoadFront", "kLoadBack", "kLoadFrontStayBack", "kLoadStayBack"
    };
    static const char *abbrevs[4] = { "F", "B", "FSB", "SB" };
    MILO_ASSERT(pos >= 0 && pos <= kLoadStayBack, 0x121);
    if (abbrev)
        return abbrevs[pos];
    else
        return names[pos];
}

void LoadMgr::Print() {
    FOREACH (it, mLoading) {
        TheDebug << (*it)->LoaderFile().c_str() << " "
                 << LoaderPosString((*it)->GetPos(), false) << "\n";
    }
}

void LoadMgr::SetEditMode(bool flag) {
    mEditMode = flag;
    static DataNode &edit_mode = DataVariable("edit_mode");
    edit_mode = mEditMode;
}

Loader *LoadMgr::ForceGetLoader(const FilePath &fp) {
    if (fp.empty())
        return nullptr;
    else {
        Loader *gotten = GetLoader(fp);
        if (!gotten) {
            gotten = TheLoadMgr.AddLoader(fp, kLoadFront);
            if (!gotten) {
                MILO_NOTIFY("Don't recognize file %s", fp);
            }
        }
        if (gotten) {
            TheLoadMgr.PollUntilLoaded(gotten, 0);
        }
        return gotten;
    }
}

void LoadMgr::Poll() {
    if (mPeriod > 0) {
        mTimer.Restart();
        unk1c = mPeriod;
        while (!mLoading.empty()) {
            PollFrontLoader();
            if (!mLoading.empty()) {
                if (mLoading.front()->IsLoaded()) {
                    mLoading.pop_front();
                }
            }
            if (CheckSplit())
                return;
        }
    }
}

void LoadMgr::RegisterFactory(const char *cc, LoaderFactoryFunc *func) {
    FOREACH (it, mFactories) {
        if (it->first == cc) {
            MILO_NOTIFY("More than one LoadMgr factory for extension \"%s\"!", cc);
        }
    }
    mFactories.push_back(std::pair<String, LoaderFactoryFunc *>(cc, func));
}

Loader *LoadMgr::GetLoader(const FilePath &fp) const {
    if (fp.empty())
        return nullptr;
    else {
        Loader *theLoader = nullptr;
        FOREACH (it, mLoaders) {
            if ((*it)->LoaderFile() == fp) {
                theLoader = *it;
                break;
            }
        }
        return theLoader;
    }
}

Loader *LoadMgr::AddLoader(const FilePath &file, LoaderPos pos) {
    if (file.empty())
        return nullptr;
    if (sFileOpenCallback) {
        sFileOpenCallback(file.c_str());
    }
    const char *ext = FileGetExt(file.c_str());
    FOREACH (it, mFactories) {
        if (it->first == ext) {
            return (it->second)(file, pos);
        }
    }
    return new FileLoader(file, file.c_str(), pos, 0, false, true, nullptr, nullptr);
}

void LoadMgr::PollUntilLoaded(Loader *l1, Loader *l2) {
    AutoGlitchReport r(50, __FUNCTION__);
    static int sLoadCount = 1;
    int count = ++sLoadCount;
    l1->SetUnk4(count);
    float old = unk1c;
    while (!l1->IsLoaded()) {
        unk1c = kHugeFloat;
        if (l2 && l2 == mLoading.front()) {
            MILO_FAIL(
                "PollUntilLoaded circular dependency %s on %s",
                l2->DebugText(),
                l1->DebugText()
            );
        }
        PollFrontLoader();
        if (!ListFind(mLoading, l1) || l1->GetUnk4() != count)
            break;
        if (mLoading.front()->IsLoaded()) {
            mLoading.pop_front();
        }
    }
    unk1c = old;
}

void LoadMgr::PollFrontLoader() {
    Loader *l = mLoading.front();
    LoaderPos old = mLoaderPos;
    mLoaderPos = l->GetPos();
    LoaderGlitchInfo info;
    info.loaderFile = l->LoaderFile().c_str();
    info.loaderPos = l->GetPos();
    info.loaderState = l->StateName();
    if (TheArchive) {
        if (Archive::DebugArkOrder() && l->GetLoadTimeStartMs() == -1) {
            l->SetLoadTimeStartMs(SystemMs());
            if (gLoadCount == 0) {
                MILO_LOG("Loading%s Start '%s'\n", WhiteSpace(0), info.loaderFile);
            }
            gLoadCount++;
        }
    }
    bool c7 = false;
    bool b5 = false;
    int loadStart = l->GetLoadTimeStartMs();
    {
        MemHeapTracker t(l->Heap());
        if (UsingCD()) {
            AutoGlitchReport r(mPeriod * 3, FrontLoaderGlitchCB, &info);
            l->PollLoading();
            if (!ListFind(mLoading, l)) {
                c7 = true;
                b5 = true;
                info.frontLoaderState = "deleted";
            } else {
                info.frontLoaderState = l->StateName();
                c7 = l->IsLoaded();
            }
        } else {
            l->PollLoading();
        }
    }
    if (TheArchive) {
        if (Archive::DebugArkOrder() && c7) {
            int loadEnd = SystemMs();
            if (!b5) {
                gLoadCount--;
                l->SetLoadTimeStartMs(-1);
            }
            if (loadEnd - loadStart > 20 || gLoadCount == 0) {
                int diff = loadEnd - loadStart;
                MILO_LOG(
                    "Loading%s End   %4d [%5d,%5d]  '%s'\n",
                    WhiteSpace(gLoadCount),
                    diff,
                    loadStart,
                    loadEnd,
                    info.loaderFile
                );
            }
        }
    }
    mLoaderPos = old;
}

#pragma endregion
#pragma region Handlers

DataNode OnSetLoadMgrDebug(DataArray *a) {
    TheLoadMgr.SetCacheMode(a->Int(1));
    return 0;
}

DataNode OnSetEditMode(DataArray *a) {
    TheLoadMgr.SetEditMode(a->Int(1));
    return 0;
}

DataNode OnSetLoaderPeriod(DataArray *a) {
    return TheLoadMgr.SetLoaderPeriod(a->Float(1));
}

DataNode OnSysPlatformSym(DataArray *a) {
    return PlatformSymbol(TheLoadMgr.GetPlatform());
}

DataNode OnLoadMgrPrint(DataArray *a) {
    TheLoadMgr.Print();
    return 0;
}

void LoadMgr::Init() {
    SetEditMode(false);
    if (OptionBool("null_platform", false))
        mPlatform = kPlatformNone;
    DataRegisterFunc("loadmgr_debug", OnSetLoadMgrDebug);
    DataRegisterFunc("loadmgr_print", OnLoadMgrPrint);
    DataRegisterFunc("set_edit_mode", OnSetEditMode);
    DataRegisterFunc("set_loader_period", OnSetLoaderPeriod);
    DataRegisterFunc("sysplatform_sym", OnSysPlatformSym);
    DataVariable("sysplatform") = (int)mPlatform;
}
