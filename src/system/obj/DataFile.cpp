#include "obj/DataFile.h"

#include "DataFlex.h"
#include "utl/Compress.h"
#include "utl/Std.h"
#include "math/FileChecksum.h"
#include "obj/Data.h"
#include "obj/DataFile_Flex.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "os/ThreadCall.h"
#include "utl/BufStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

CriticalSection gDataReadCrit; // yes these are the bss offsets. this tu sucks
//DataArray *gArray; // 0x28
int gNode; // 0x2c
Symbol gFile; // 0x30
BinStream *gBinStream; // 0x34
// int gOpenArray = kDataTokenFinished; // 0x38 ?
// std::list<ConditionalInfo> gConditional; // 0x48 - actually a list of ConditionalInfo
//                                          // structs
std::list<bool> gConditional;
int gDataLine; // 0x50
std::map<String, DataNode> gReadFiles; // 0x60

// bool gCompressCached;
// bool gCachingFile;
bool gReadingFile;

void DataWriteFile(const char *file, const DataArray *da, int i) {
    TextStream *stream;
    if (file != 0) {
        stream = new TextFileStream(file, false);
    }
    else {
        stream = new Debug();
    }
    for (; i < da->Size(); i++) {
        da->Node(i).Print(*stream, false, 0);
        stream->operator<<("\n");
    }
    if (stream) {
        stream->Space(1);
    }
}

void BeginDataRead() {
    MILO_ASSERT(gReadFiles.size() == 0, 0x29b);
    gReadingFile = 1;
}

void FinishDataRead() {
    gReadingFile = 0;
    std::map<String, DataNode> temp;
    gReadFiles.swap(temp);
}

DataArray *DataReadString(const char *c) {
    BufStream stream = BufStream(&c, strlen(c), true);
    return DataReadStream(&stream);
}

DataArray *ReadCacheStream(BinStream &bs, const char *cc) {
    CritSecTracker cst(&gDataReadCrit);
    bs.EnableReadEncryption();
    DataArray::SetFile(cc);
    DataArray *arr;
    bs >> arr;
    bs.DisableEncryption();
    return arr;
}

DataArray *DataReadStream(BinStream *bs) {
    gDataReadCrit.Enter();
    Symbol stream(bs->Name());
    gNode = 0;
    unsigned int conds1 = 0;
    gFile = stream;
    FOREACH(it, gConditional) {
        conds1++;
    }
    DataArray *parse = ParseArray();
    unsigned int conds2 = 0;
    FOREACH(it, gConditional) {
        conds2++;
    }
    if (conds2 != conds1) {
        MILO_FAIL("DataReadFile: conditional block not closed (file %s)", gFile);
    }
    gDataReadCrit.Exit();
    return parse;
}

DataArray *LoadDtz(const char *c, int i) {
    char d[4] = {c[i - 1], c[i - 2], c[i - 3], c[i - 4]};
    int decompSize = reinterpret_cast<int>(d);
    MILO_ASSERT(decompSize > 0, 0x456);
    auto pDecompBuf = MemAlloc(decompSize, __FILE__, 0x459, "LoadDtz", 0);
    MILO_ASSERT(pDecompBuf, 0x45b);
    DecompressMem(c, i - 4, pDecompBuf, decompSize, 0);
    BufStream buf_stream = BufStream(pDecompBuf, decompSize, true);
    DataArray *da;
    buf_stream >> da;
    if (pDecompBuf) {
        MemFree(pDecompBuf, __FILE__, 0x46a, "unknown");
    }
    return da;
}

const char *CachedDataFile(const char *file, bool &b) {
    bool isLocal = FileIsLocal(file);
    const char *filename = strstr(file, ".dtb");
    if (filename) {
        if (UsingCD() && !isLocal) {
            b = true;
            const char *filebase = FileGetBase(file);
            const char *filepath = FileGetPath(file);
            return MakeString("%s/gen/%s.dtb", filepath, filebase);
        }
        b = false;
    }
    return file;
}

void DataFail(const char *msg) {
    MILO_FAIL("%s (file %s, line %d)", msg, gFile, gDataLine);
}

DataArray *ReadEmbeddedFile(const char *file, bool b) {
    gDataReadCrit.Enter();
    const char *filepath = FileGetPath(gFile.Str());
    const char *madePath = FileMakePath(filepath, file);
    Symbol localfile = gFile;
    int dataline = gDataLine;
    //dat_82f6d14 - are these dats gArray?
    auto bs = gBinStream;
    //dat_82f64d0c
    //dat_82f64d08
    yyrestart(nullptr);
    DataArray *da = DataReadFile(madePath, b);
    if (b && !da) {
        MILO_FAIL("Couldn\'t open embedded file: %s (file %s, line %d)", madePath, da->File(), da->Line());
    }
    //dat_82f64d08
    //dat_82f64d0c
    gBinStream = bs;
    //dat_82f64d14
    gDataLine = dataline;
    gFile = localfile;
    yyrestart(nullptr);
    gDataReadCrit.Exit();
    return da;
}

DataLoader::DataLoader(const FilePath &fp, LoaderPos pos, bool b3)
    : Loader(fp, pos), mFilename(""), mData(nullptr), mFile(nullptr), mBufLen(0),
      mBuffer(nullptr), mDtb(b3), mThreadObj(nullptr) {
    const char *new_str = fp.c_str();
    if (b3) {
        new_str = CachedDataFile(new_str, mDtb);
    }
    mFilename = new_str;
    mState = &DataLoader::OpenFile;
}

DataLoader::~DataLoader() {
    if (mState != &DataLoader::DoneLoading) {
        delete mFile;
        MemFree(mBuffer);
    } else if (mData) {
        mData->Release();
    }
}

bool DataLoader::IsLoaded() const { return mState == &DataLoader::DoneLoading; }
void DataLoader::PollLoading() { (this->*mState)(); }
void DataLoader::DoneLoading() {}

void DataLoader::OpenFile() {
    mFile = NewFile(mFilename.c_str(), 2);
    if (mFile && !mFile->Fail()) {
        mBufLen = mFile->Size();
        mBuffer = (char *)MemAlloc(mBufLen, __FILE__, 0x366, "Resource");
        mFile->ReadAsync(mBuffer, mBufLen);
        mState = &DataLoader::LoadFile;
    } else {
        if (!mFilename.empty()) {
            MILO_NOTIFY("Could not load: %s", FileLocalize(LoaderFile().c_str(), nullptr));
        }
        mState = &DataLoader::DoneLoading;
    }
}

DataArray *DataLoader::Data() {
    MILO_ASSERT(IsLoaded(), 0x3A5);
    return mData;
}

void DataLoader::ThreadDone(DataArray *arr) {
    MILO_ASSERT(MainThread(), 0x3B5);
    mData = arr;
    RELEASE(mThreadObj);
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x3BA);
        mBuffer = nullptr;
    }
    RELEASE(mFile);
    mState = &DataLoader::DoneLoading;
}

void DataLoader::LoadFile() {
    if (mThreadObj) {
        Timer::Sleep(0);
    } else {
        int x;
        if (mFile->ReadDone(x)) {
            if (mFile->Fail()) {
                ThreadDone(nullptr);
            } else {
                mThreadObj = new DataLoaderThreadObj(
                    this, mFile, (char *)mBuffer, mBufLen, mDtb, mFilename.c_str()
                );
                ThreadCall(mThreadObj);
            }
        }
    }
}

DataLoaderThreadObj::DataLoaderThreadObj(
    DataLoader *dl, File *file, char *buffer, int bufSize, bool dtb, const char *filename
)
    : mLoader(dl), unk8(nullptr), mFile(file), mBufLen(bufSize), mBuffer(buffer),
      mFilename(filename), mDtb(dtb), mLocal(FileIsLocal(filename)) {}

int DataLoaderThreadObj::ThreadStart() {
    BufStream bs(mBuffer, mBufLen, true);
    bs.SetName(FileLocalize(mLoader->LoaderFile().c_str(), nullptr));
    if (mDtb) {
        bool runChecksum = HasFileChecksumData() && !mLocal;
        if (runChecksum) {
            bs.StartChecksum(mFilename);
        }
        unk8 = ReadCacheStream(bs, mLoader->LoaderFile().c_str());
        if (runChecksum) {
            bs.ValidateChecksum();
        }
    } else {
        unk8 = DataReadStream(&bs);
    }
    return 0;
}

void DataLoaderThreadObj::ThreadDone(int) { mLoader->ThreadDone(unk8); }
