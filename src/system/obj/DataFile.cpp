#include "obj/DataFile.h"
#include "math/Utl.h"
#include "obj/DataFlex.h"
#include "obj/DataUtl.h"
#include "utl/BinStream.h"
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

static DataArray *gArray = nullptr;
static int gNode = 0;
static BinStream *gStream = nullptr;
static int gOpenArray = 0;
static bool gCachingFile = false;
static bool gReadingFile = false;
int gDataLine = 0;
static CriticalSection gDataReadCrit;
Symbol gFile;
static std::list<bool> gConditional;
static std::map<String, DataNode> gReadFiles;
// bool gCompressCached;

#pragma region DataLoader

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
        TheLoadMgr.SetUnk1c(0);
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

#pragma endregion
#pragma region DataLoaderThreadObj

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

#pragma endregion

void DataWriteFile(const char *file, const DataArray *array, int startIndex) {
    TextStream *stream;
    if (file != 0) {
        stream = new TextFileStream(file, false);
    } else {
        stream = new Debug();
    }
    for (int i = startIndex; i < array->Size(); i++) {
        array->Node(i).Print(*stream, false, 0);
        *stream << "\n";
    }
    delete stream;
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

DataArray *DataReadString(const char *str) {
    BufStream stream = BufStream((void *)str, strlen(str), true);
    return DataReadStream(&stream);
}

DataArray *ReadCacheStream(BinStream &stream, const char *file) {
    CritSecTracker cst(&gDataReadCrit);
    stream.EnableReadEncryption();
    DataArray::SetFile(file);
    DataArray *arr;
    stream >> arr;
    stream.DisableEncryption();
    return arr;
}

DataArray *DataReadStream(BinStream *bs) {
    CritSecTracker tracker(&gDataReadCrit);
    gStream = bs;
    gFile = bs->Name();
    gDataLine = 1;
    gOpenArray = 0;
    unsigned int before = gConditional.size();
    DataArray *parse = ParseArray();
    unsigned int after = gConditional.size();
    MILO_ASSERT_FMT(
        after == before, "DataReadFile: conditional block not closed (file %s)", gFile
    );
    return parse;
}

DataArray *LoadDtz(const char *c, int charIdx) {
    int decompSize = 0;
    char *decompChars = (char *)&decompSize;
    decompChars[0] = c[charIdx - 1];
    decompChars[1] = c[charIdx - 2];
    decompChars[2] = c[charIdx - 3];
    decompChars[3] = c[charIdx - 4];
    MILO_ASSERT(decompSize > 0, 0x456);
    void *pDecompBuf = MemAlloc(decompSize, __FILE__, 0x459, "LoadDtz", 0);
    MILO_ASSERT(pDecompBuf, 0x45b);
    DecompressMem(c, charIdx - 4, pDecompBuf, decompSize, 0);
    BufStream buf_stream(pDecompBuf, decompSize, true);
    DataArray *da = nullptr;
    buf_stream >> da;
    if (pDecompBuf) {
        MemFree(pDecompBuf, __FILE__, 0x46a);
    }
    return da;
}

const char *CachedDataFile(const char *file, bool &b) {
    bool isLocal = FileIsLocal(file);
    const char *filename = strstr(file, ".dtb");
    if (filename) {
        b = true;
    } else {
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
    CritSecTracker tracker(&gDataReadCrit);
    const char *filepath = FileGetPath(gFile.Str());
    const char *madePath = FileMakePath(filepath, file);
    Symbol localfile = gFile;
    int dataline = gDataLine;
    int node = gNode;
    BinStream *bs = gStream;
    int openArr = gOpenArray;
    DataArray *arr = gArray;
    yyrestart(nullptr);
    DataArray *da = DataReadFile(madePath, b);
    if (b && !da) {
        MILO_FAIL(
            "Couldn't open embedded file: %s (file %s, line %d)",
            madePath,
            da->File(),
            da->Line()
        );
    }
    gNode = node;
    gStream = bs;
    gDataLine = dataline;
    gFile = localfile;
    gArray = arr;
    gOpenArray = openArr;
    yyrestart(nullptr);
    return da;
}

void PushBack(const DataNode &n) {
    if (gNode == gArray->Size()) {
        int max = 0x7FFF;
        if (gNode >= 0x7FFF) {
            MILO_FAIL(
                "%s(%d): array size > max %d lines", gArray->File(), gArray->Line(), max
            );
        }
        {
            MemTemp tmp;
            gArray->Resize(Min(gNode * 2, 0x7FFF));
        }
    }
    gArray->Node(gNode++) = n;
}

int DataInput(void *v, int x) {
    if (gStream->Fail()) {
        return 0;
    } else if (gStream->Eof()) {
        return 0;
    } else {
        gStream->Read(v, x);
        MILO_ASSERT(!gStream->Fail(), 0x260);
        return x;
    }
}

DataArray *DataReadFile(const char *file, bool warn) {
    char buffer[256];
    strcpy(buffer, file);

    bool b;
    const char *cached = CachedDataFile(buffer, b);
    DataNode *node;
    if (gReadingFile) {
        node = &gReadFiles[cached];
        if (node->Type() == kDataArray) {
            DataArray *arr = node->LiteralArray();
            arr->AddRef();
            return arr;
        }
    } else {
        node = nullptr;
        BeginDataRead();
    }

    FileStream fs(cached, FileStream::kRead, true);
    if (fs.Fail()) {
        if (warn) {
            MILO_NOTIFY("DataReadFile: Can't open %s", buffer);
        }
        if (!node) {
            FinishDataRead();
        }
        return nullptr;
    } else {
        DataArray *ret;
        if (b) {
            if (HasFileChecksumData()) {
                fs.StartChecksum();
            }
            ret = ReadCacheStream(fs, buffer);
            if (HasFileChecksumData()) {
                fs.ValidateChecksum();
            }
        } else {
            ret = DataReadStream(&fs);
        }
        if (node) {
            *node = ret;
        } else {
            FinishDataRead();
        }
        return ret;
    }
}

bool Defined() {
    FOREACH (it, gConditional) {
        if (!*it)
            return false;
    }
    return true;
}

bool ParseNode() {
    int token = yylex();
    if (!Defined() && token != kDataTokenIfdef && token != kDataTokenIfndef
        && token != kDataTokenElse && token != kDataTokenEndif) {
        return true;
    }

    char bom[3] = { 0xEF, 0xBB, 0xBF };
    if (gNode == 0 && strncmp(yytext, bom, DIM(bom)) == 0) {
        if (yyleng > 3) {
            MILO_FAIL(
                "%s starts with a ByteOrderMark, put a line return at the top of its file",
                gFile
            );
        } else {
            return true;
        }
    }

    if (token == kDataTokenFinished) {
        if (gOpenArray == kDataTokenFinished) {
            // don't fail, just return
        } else if (gOpenArray == kDataTokenArrayOpen) {
            MILO_FAIL("Array closed incorrectly (file %s, line %d)", gFile, gDataLine);
        } else if (gOpenArray == kDataTokenCommandOpen) {
            MILO_FAIL("Command closed incorrectly (file %s, line %d)", gFile, gDataLine);
        } else if (gOpenArray == kDataTokenPropertyOpen) {
            MILO_FAIL("Property closed incorrectly (file %s, line %d)", gFile, gDataLine);
        }
        return false;
    } else if (token == kDataTokenArrayClose) {
        if (gOpenArray == kDataTokenArrayOpen) {
            // don't fail, just return
        } else if (gOpenArray == kDataTokenFinished) {
            MILO_FAIL("File %s ends with open array", gFile);
        } else if (gOpenArray == kDataTokenCommandOpen) {
            MILO_FAIL("Command closed incorrectly (file %s, line %d)", gFile, gDataLine);
        } else if (gOpenArray == kDataTokenPropertyOpen) {
            MILO_FAIL("Property closed incorrectly (file %s, line %d)", gFile, gDataLine);
        }
        return false;
    } else if (token == kDataTokenPropertyClose) {
        if (gOpenArray == kDataTokenPropertyOpen) {
            // don't fail, just return
        } else if (gOpenArray == kDataTokenFinished) {
            MILO_FAIL("File %s ends with open array", gFile);
        } else if (gOpenArray == kDataTokenArrayOpen) {
            MILO_FAIL("Array closed incorrectly (file %s, line %d)", gFile, gDataLine);
        } else if (gOpenArray == kDataTokenCommandOpen) {
            MILO_FAIL("Command closed incorrectly (file %s, line %d)", gFile, gDataLine);
        }
        return false;
    } else if (token == kDataTokenCommandClose) {
        if (gOpenArray == kDataTokenCommandOpen) {
            // don't fail, just return
        } else if (gOpenArray == kDataTokenFinished) {
            MILO_FAIL("File %s ends with open array", gFile);
        } else if (gOpenArray == kDataTokenArrayOpen) {
            MILO_FAIL("Array closed incorrectly (file %s, line %d)", gFile, gDataLine);
        } else if (gOpenArray == kDataTokenPropertyOpen) {
            MILO_FAIL("Property closed incorrectly (file %s, line %d)", gFile, gDataLine);
        }
        return false;
    }

    if (token == kDataTokenMerge) {
        if (yylex() != kDataTokenSymbol) {
            MILO_FAIL(
                "DataReadFile: merging a non-symbol (file %s, line %d)", gFile, gDataLine
            );
        }
        if (gCachingFile) {
            PushBack(DataNode(kDataMerge, Symbol(yytext).Str()));
        } else {
            bool usingEmbedded = false;
            DataArray *fileArr = DataGetMacro(yytext);
            if (!fileArr) {
                fileArr = ReadEmbeddedFile(yytext, true);
                usingEmbedded = true;
            }
            if (fileArr && fileArr->Size() == 0) {
                MILO_FAIL("Empty merge file (possibly a re-included file): %s", yytext);
            }
            gArray->Resize(gNode);
            DataMergeTags(gArray, fileArr);
            gNode = gArray->Size();
            if (usingEmbedded) {
                fileArr->Release();
            }
        }
        return true;
    }

    switch (token) {
    case kDataTokenInclude:
    case kDataTokenIncludeOptional: {
        bool required = token == kDataTokenInclude;
        if (yylex() != kDataTokenSymbol) {
            MILO_FAIL(
                "DataReadFile: including a non-symbol (file %s, line %d)",
                gFile,
                gDataLine
            );
        }
        if (gCachingFile) {
            PushBack(DataNode(kDataInclude, Symbol(yytext).Str()));
        } else {
            DataArray *fileArr = ReadEmbeddedFile(yytext, required);
            if (fileArr) {
                for (int i = 0; i < fileArr->Size(); i++) {
                    PushBack(fileArr->Node(i));
                }
                fileArr->Release();
            }
        }
        return true;
    }
    case kDataTokenIfdef:
    case kDataTokenIfndef: {
        bool positive = token == kDataTokenIfdef;

        int symToken = yylex();
        bool isSymbol =
            symToken == kDataTokenSymbol || symToken == kDataTokenQuotedSymbol;
        if (!isSymbol) {
            MILO_FAIL(
                "DataReadFile: not macro symbol (file %s, line %d)", gFile, gDataLine
            );
        }

        char *text;
        if (symToken == kDataTokenQuotedSymbol) {
            // Strip quotes from quoted symbol
            yytext[yyleng - 1] = '\0';
            text = yytext + 1;
        } else {
            text = yytext;
        }

        Symbol macro(text);
        if (positive) {
            if (gCachingFile) {
                PushBack(DataNode(kDataIfdef, macro.Str()));
            } else {
                bool defined = DataGetMacro(macro) != 0;
                gConditional.push_back(defined);
            }
        } else {
            if (gCachingFile) {
                PushBack(DataNode(kDataIfndef, macro.Str()));
            } else {
                bool ndefined = DataGetMacro(macro) == 0;
                gConditional.push_back(ndefined);
            }
        }
        return true;
    }
    case kDataTokenElse: {
        if (gCachingFile) {
            PushBack(DataNode(kDataElse, 0));
        } else {
            if (gConditional.empty()) {
                MILO_FAIL(
                    "DataReadFile: #else not in conditional (file %s, line %d)",
                    gFile,
                    gDataLine
                );
            }
            gConditional.back() = !gConditional.back();
        }
        return true;
    }
    default:
        break;
    }
    // if/else if chain here
    if (token == kDataTokenEndif) {
        if (gCachingFile) {
            PushBack(DataNode(kDataEndif, 0));
        } else {
            if (gConditional.empty()) {
                MILO_FAIL(
                    "DataReadFile: #endif not in conditional (file %s, line %d)",
                    gFile,
                    gDataLine
                );
            }
            gConditional.pop_back();
        }
        return true;
    } else if (token == kDataTokenAutorun) {
        int cmdToken = yylex();
        if (cmdToken != kDataTokenCommandOpen) {
            MILO_FAIL("DataReadFile: not command (file %s, line %d)", gFile, gDataLine);
        }

        int openArray = gOpenArray;
        gOpenArray = cmdToken;
        DataArray *array = ParseArray();
        gOpenArray = openArray;

        DataNode node(array, kDataCommand);
        if (gCachingFile) {
            PushBack(DataNode(kDataAutorun, 0));
            PushBack(node);
        } else {
            node.Command(array)->Execute();
        }

        array->Release();
        return true;
    } else if (token == kDataTokenDefine) {
        if (yylex() != kDataTokenSymbol) {
            MILO_FAIL("DataReadFile: not symbol (file %s, line %d)", gFile, gDataLine);
        }

        Symbol macro(yytext);

        int cmdToken = yylex();
        if (cmdToken != kDataTokenArrayOpen) {
            MILO_FAIL("DataReadFile: not array (file %s, line %d)", gFile, gDataLine);
        }

        int openArray = gOpenArray;
        gOpenArray = cmdToken;
        DataArray *array = ParseArray();
        gOpenArray = openArray;

        if (gCachingFile) {
            PushBack(DataNode(kDataDefine, macro.Str()));
            PushBack(array);
        } else {
            DataSetMacro(macro, array);
        }

        array->Release();
        return true;
    } else if (token == kDataTokenUndef) {
        if (yylex() != kDataTokenSymbol) {
            MILO_FAIL("DataReadFile: not synbol (file %s, line %d)", gFile, gDataLine);
        }

        Symbol macro(yytext);
        if (gCachingFile) {
            PushBack(DataNode(kDataUndef, macro.Str()));
        } else {
            DataSetMacro(macro, nullptr);
        }

        return true;
    };

    switch (token) {
    case kDataTokenArrayOpen:
    case kDataTokenPropertyOpen:
    case kDataTokenCommandOpen: {
        int openArray = gOpenArray;
        gOpenArray = token;
        DataArray *array = ParseArray();
        gOpenArray = openArray;

        DataType type;
        if (token == kDataTokenArrayOpen) {
            type = kDataArray;
        } else if (token == kDataTokenCommandOpen) {
            type = kDataCommand;
        } else {
            type = kDataProperty;
        }

        PushBack(DataNode(array, type));
        array->Release();

        return true;
    }
    default:
        break;
    }
    if (token == kDataTokenVar) {
        PushBack(&DataVariable(yytext + 1));
        return true;
    } else if (token == kDataTokenUnhandled) {
        PushBack(DataNode(kDataUnhandled, 0));
        return true;
    } else if (token == kDataTokenInt) {
        PushBack(atoi(yytext));
        return true;
    } else if (token == kDataTokenHex) {
        int i = 0;

        // Parse in reverse, up until the `x` of `0x`
        int base = 1;
        // TODO: yytext needs to be loaded twice here, but is being optimized to one
        // load
        for (char *c = yytext + strlen(yytext) - 1; *c != 'x'; --c, base <<= 4) {
            if (*c >= 'a') {
                i += (*c - 'a' + 10) * base;
#ifdef NON_MATCHING
            } else if (*c >= 'A') {
#else
            } else if (*c > 'A') { //! BUG: should be >=, else `A` won't parse
                                   //! correctly
#endif
                i += (*c - 'A' + 10) * base;
            } else {
                i += (*c - '0') * base;
            }
        }

        PushBack(i);
        return true;
    } else if (token == kDataTokenFloat) {
        PushBack((float)atof(yytext));
        return true;
    }

    switch (token) {
    case kDataTokenSymbol:
    case kDataTokenQuotedSymbol: {
        char *text;
        if (token == kDataTokenQuotedSymbol) {
            // Strip quotes from quoted symbol
            yytext[yyleng - 1] = '\0';
            text = yytext + 1;
        } else {
            text = yytext;
        }

        Symbol sym(text);
        DataArray *macro = DataGetMacro(sym);
        bool b = macro && !gCachingFile;
        if (b) {
            for (int i = 0; i < macro->Size(); i++) {
                PushBack(macro->Node(i));
            }
        } else {
            PushBack(sym);
        }

        return true;
    }
    case kDataTokenString: {
        yytext[yyleng - 1] = '\0';
        char *text = yytext + 1;

        for (char *c = text; *c != '\0'; c++) {
            bool escaped = false;
            if (*c == '\\') {
                if (c[1] == 'n') {
                    *c = '\n';
                    escaped = true;
                } else if (c[1] == 'q') {
                    *c = '\"';
                    escaped = true;
                }
            } else if (*c == '\n') {
                // Newlines in strings must be accounted for manually,
                // as the lexer won't run the action for it due to being part of the
                // string literal
                gDataLine++;
            }

            if (escaped) {
                for (char *d = c + 1; *d != '\0'; d++) {
                    *d = *(d + 1);
                }
            }
        }

        PushBack(text);
        return true;
    }

    default:
        MILO_FAIL(
            "DataReadFile: Unrecognized token %d (file %s, line %d)",
            token,
            gFile,
            gDataLine
        );
        return false;
    }
}

DataArray *ParseArray() {
    DataArray *arr = gArray;
    int node = gNode;
    gArray = new DataArray(16);
    gArray->SetFileLine(gFile, gDataLine);
    gNode = 0;
    while (ParseNode())
        ;
    gArray->Resize(gNode);
    DataArray *ret = gArray;
    gNode = node;
    gArray = arr;
    return ret;
}
