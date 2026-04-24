#include "math/Color.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

DataArray *gCallStack[HANDLE_STACK_SIZE];
DataArray **gCallStackPtr = gCallStack;
int gPreExecuteLevel;
DataFunc *DataArray::sDefaultHandler;
DataFunc *gPreExecuteFunc;
Symbol gFile;
std::list<bool> gConditional;

class DataCallStackFrame {
public:
    DataCallStackFrame(DataArray *arr) {
        MILO_ASSERT(gCallStackPtr - gCallStack < HANDLE_STACK_SIZE, 0x2F);
        *gCallStackPtr++ = arr;

        if (gPreExecuteFunc && (gCallStackPtr - gCallStack) <= gPreExecuteLevel) {
            gPreExecuteFunc(arr);
        }
    }

    ~DataCallStackFrame() {
        if (--gCallStackPtr == gCallStack) {
            gPreExecuteFunc = nullptr;
        }
    }
};

DataNode *NodesAlloc(int size) {
    return (DataNode *)MemOrPoolAlloc(size, __FILE__, 0xFE, "Nodes");
}

void NodesFree(int size, DataNode *mem) { MemOrPoolFree(size, mem); }

bool DataArray::PrintUnused(TextStream &ts, DataType ty, bool b) const {
    bool ret = false;
    for (int i = 0; i < mSize; i++) {
        ret |= mNodes[i].PrintUnused(ts, b);
    }
    return ret;
}

DataArray::~DataArray() {
    if (mSize < 0) {
        NodesFree(-mSize, mNodes);
    } else {
        for (int i = 0; i < mSize; i++) {
            mNodes[i].~DataNode();
        }
        NodesFree(mSize * sizeof(DataNode), mNodes);
    }
}

void DataArray::SetFileLine(Symbol file, int line) {
    mFile = file;
    mLine = line;
}

void DataArray::SetFile(Symbol file) { gFile = file; }

bool strncat_tofit(FixedString &str, const char *cc, int i) {
    if ((int)(strlen(str.c_str()) + strlen(cc)) < i) {
        str += cc;
        return true;
    } else
        return false;
}

DataNode &DataArray::Node(int i) const {
    MILO_ASSERT_FMT(
        i >= 0 && i < mSize,
        "Array doesn't have node %d, only has %d (file %s, line %d)",
        i,
        mSize,
        File(),
        Line()
    );
    return mNodes[i];
}

DataNode &DataArray::Node(int i) {
    MILO_ASSERT_FMT(
        i >= 0 && i < mSize,
        "Array doesn't have node %d, only has %d (file %s, line %d)",
        i,
        mSize,
        File(),
        Line()
    );
    return mNodes[i];
}

void DataArray::Print(TextStream &ts, DataType type, bool b, int i3) const {
    DataNode *end = &mNodes[mSize];
    MILO_ASSERT(type & kDataArray, 0xA3);
    char open = '\0';
    char close = '\0';
    if (type == kDataArray) {
        open = '(';
        close = ')';
    } else if (type == kDataCommand) {
        open = '{';
        close = '}';
    } else if (type == kDataProperty) {
        open = '[';
        close = ']';
    } else {
        MILO_FAIL("Unrecognized array type %d", type);
    }

    DataNode *n;
    for (n = mNodes; n < end && !(n->Type() & kDataArray); n++)
        ;

    if (n == end || b) {
        ts << open;
        for (n = mNodes; n < end; n++) {
            if (n != mNodes) {
                ts << " ";
            }
            n->Print(ts, b, i3);
        }
    } else {
        ts << open;
        n = mNodes;
        if (n->Type() == kDataSymbol) {
            n->Print(ts, b, i3);
            n++;
        }
        ts << "\n";
        i3 += 3;
        for (; n < end; n++) {
            ts.Space(i3);
            n->Print(ts, b, i3);
            ts << "\n";
        }
        i3 -= 3;
        ts.Space(i3);
    }
    ts << close;
}

void DataArray::Insert(int count, const DataNode &dn) {
    int i = 0;
    int newNodeCount = mSize + 1;
    DataNode *oldNodes = mNodes; // Save all nodes pointer
    // allocate new nodes
    mNodes = NodesAlloc(newNodeCount * sizeof(DataNode));

    for (i = 0; i < count; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i]);
    }
    for (; i < count + 1; i++) {
        new (&mNodes[i]) DataNode(dn);
    }
    for (; i < newNodeCount; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i - 1]);
    }
    for (i = 0; i < mSize; i++) {
        oldNodes[i].~DataNode();
    }

    // free old nodes
    NodesFree(mSize * sizeof(DataNode), oldNodes);
    mSize = newNodeCount;
}

void DataArray::InsertNodes(int count, const DataArray *da) {
    if ((da == 0) || (da->Size() == 0))
        return;
    int i = 0;
    int dacnt = da->Size();
    int newNodeCount = mSize + dacnt;
    DataNode *oldNodes = mNodes; // Save all nodes pointer
    // allocate new nodes
    mNodes = (DataNode *)NodesAlloc(newNodeCount * sizeof(DataNode));

    for (i = 0; i < count; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i]);
    }

    for (; i < count + dacnt; i++) {
        new (&mNodes[i]) DataNode(da->Node(i - count));
    }

    for (; i < newNodeCount; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i - dacnt]);
    }
    for (i = 0; i < mSize; i++) {
        oldNodes[i].~DataNode();
    }
    NodesFree(mSize * sizeof(DataNode), oldNodes);
    mSize = newNodeCount;
}

void DataArray::Resize(int i) {
    DataNode *oldNodes = mNodes;
    mNodes = (DataNode *)NodesAlloc(i * sizeof(DataNode));
    int min = Min<int>(mSize, i);
    int cnt = 0;
    for (cnt = 0; cnt < min; cnt++) {
        new (&mNodes[cnt]) DataNode(oldNodes[cnt]);
    }
    for (; cnt < i; cnt++) {
        new (&mNodes[cnt]) DataNode();
    }
    for (cnt = 0; cnt < mSize; cnt++) {
        oldNodes[cnt].~DataNode();
    }
    NodesFree(mSize * sizeof(DataNode), oldNodes);
    mSize = i;
    mDeprecated = 0;
}

void DataArray::Remove(int index) {
    MILO_ASSERT(index < mSize, 0x16E);
    DataNode *oldNodes = mNodes;
    int newCnt = mSize - 1;
    mNodes = NodesAlloc(newCnt * sizeof(DataNode));
    for (int i = 0; i < index; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i]);
    }
    for (int i = index; i < newCnt; i++) {
        new (&mNodes[i]) DataNode(oldNodes[i + 1]);
    }
    for (int i = 0; i < mSize; i++) {
        oldNodes[i].~DataNode();
    }
    NodesFree(mSize * sizeof(DataNode), oldNodes);
    mSize = newCnt;
}

void DataArray::Remove(const DataNode &dn) {
    int searchType = dn.UncheckedInt();
    for (int lol = mSize - 1; lol >= 0; lol--) {
        if (mNodes[lol].UncheckedInt() == searchType) {
            Remove(lol);
            return;
        }
    }
}

bool DataArray::Contains(const DataNode &dn) const {
    int searchType = dn.UncheckedInt();
    for (int lol = mSize - 1; lol >= 0; lol--) {
        if (mNodes[lol].UncheckedInt() == searchType) {
            return true;
        }
    }
    return false;
}

DataArray::DataArray(int size)
    : mFile(), mSize(size), mRefs(1), mLine(0), mDeprecated(0) {
    mNodes = NodesAlloc(size * sizeof(DataNode));
    for (int n = 0; n < size; n++) {
        new (&mNodes[n]) DataNode();
    }
}

DataArray::DataArray(const void *glob, int size)
    : mFile(), mSize(-size), mRefs(1), mLine(0), mDeprecated(0) {
    mNodes = NodesAlloc(size);
    memcpy(mNodes, glob, size);
}

int NodeCmp(const void *a, const void *b) {
    const DataNode *anode = (const DataNode *)a;
    const DataNode *bnode = (const DataNode *)b;
    switch (anode->Type()) {
    case kDataFloat:
    case kDataInt: {
        float a = anode->LiteralFloat();
        float b = bnode->LiteralFloat();
        if (a < b) {
            return -1;
        } else if (a == b) {
            return 0;
        } else {
            return 1;
        }
    }
    case kDataString:
    case kDataSymbol:
        return stricmp(anode->Str(), bnode->Str());
    case kDataArray:
        return NodeCmp(&anode->Array()->Node(0), &bnode->Array()->Node(0));
    case kDataObject: {
        const char *aName = anode->GetObj() ? anode->GetObj()->Name() : "";
        const char *bName = bnode->GetObj() ? bnode->GetObj()->Name() : "";
        return stricmp(aName, bName);
    }
    default:
        MILO_NOTIFY("could not sort array, bad type");
        return 0;
    }
}

void DataArray::SortNodes(int idx) {
    if (mSize <= 0)
        return;
    if (idx >= mSize)
        return;
    qsort(&mNodes[idx], mSize - idx, 8, NodeCmp);
}

void DataArrayGlitchCB(float f, void *v) {
    DataArray *arr = (DataArray *)v;
    arr->Node(0).Print(TheDebug, true, 0);
    MILO_LOG(" took %.2f ms (File: %s Line: %d)\n", f, arr->File(), arr->Line());
}

void DataArray::Save(BinStream &bs) const {
    bs << mSize << mLine << mDeprecated;
    for (int i = 0; i < mSize; i++) {
        mNodes[i].Save(bs);
    }
}

void DataArray::SaveGlob(BinStream &bs, bool b) const {
    if (b) {
        int i = -1 - mSize;
        bs << i;
        bs.Write(mNodes, i);
    } else {
        bs << mSize;
        bs.Write(mNodes, -mSize);
    }
}

void DataArray::LoadGlob(BinStream &bs, bool b) {
    MILO_ASSERT(mSize <= 0, 0x4A8);
    NodesFree(-mSize, mNodes);
    if (b) {
        int v;
        bs >> v;
        mSize = -(v + 1);
        mNodes = NodesAlloc(-mSize);
        bs.Read(mNodes, v);
        ((unsigned char *)mNodes)[v] = 0;
    } else {
        bs >> mSize;
        mNodes = NodesAlloc(-mSize);
        bs.Read(mNodes, -mSize);
    }
}

TextStream &operator<<(TextStream &ts, const DataArray *da) {
    if (da)
        da->Print(ts, kDataArray, false, 0);
    else
        ts << "<null>";
    return ts;
}

BinStream &operator<<(BinStream &bs, const DataArray *da) {
    if (da != 0) {
        bs << true;
        da->Save(bs);
    } else
        bs << false;
    return bs;
}

void DataAppendStackTrace(FixedString &msg) {
    if (gCallStackPtr <= gCallStack) {
        return;
    }

    msg += "\n\nData Stack Trace";

    char visualStudioFmt[14];
    bool msg_full = false;

    for (DataArray **ptr = gCallStackPtr - 1; ptr >= gCallStack; ptr--) {
        DataArray *a = *ptr;

        String s;
        if (a->Size() > 0) {
            a->Node(0).Print(s, true, 0);
        }

        memcpy(visualStudioFmt, "\n   %s(%d):%s", 0xe);
        const char *visualStudioMsg =
            MakeString(visualStudioFmt, a->File(), a->Line(), s.c_str());
        if (!msg_full) {
            if (!strncat_tofit(msg, visualStudioMsg, 0x400)) {
                MILO_LOG(msg.c_str());
                msg_full = true;
                msg += MakeString(
                    "\n   ... %d omitted stack frames", (ptr - gCallStack) + 1
                );
            }
        }
        if (msg_full) {
            MILO_LOG(visualStudioMsg);
        }
    }
    if (msg_full) {
        MILO_LOG("\n");
    }
}

DataArray *DataArray::FindArray(int tag, bool fail) const {
    DataNode *dn;
    DataNode *dn_end = &mNodes[mSize];
    for (dn = mNodes; dn < dn_end; dn++) {
        if (dn->Type() == kDataArray) {
            const DataArray *arr = dn->UncheckedArray();
            if (arr->UncheckedInt(0) == tag) {
                return (DataArray *)arr;
            }
        }
    }
    if (fail)
        MILO_FAIL("Couldn't find %d in array (file %s, line %d)", tag, File(), mLine);
    return nullptr;
}

DataArray *DataArray::FindArray(Symbol tag, bool fail) const {
    DataArray *found = FindArray((int)tag, false);
    if (!found && fail)
        MILO_FAIL(
            "Couldn't find '%s' in array (file %s, line %d)", tag.Str(), File(), mLine
        );
    return found;
}

DataArray *DataArray::FindArray(Symbol s1, Symbol s2) const {
    return FindArray(s1)->FindArray(s2);
}

DataArray *DataArray::FindArray(Symbol s1, Symbol s2, Symbol s3) const {
    return FindArray(s1)->FindArray(s2)->FindArray(s3);
}

DataArray *DataArray::FindArray(Symbol s, const char *c) const {
    return FindArray(s, Symbol(c));
}

bool DataArray::FindData(Symbol s, const char *&ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != 0) {
        ret = arr->Str(1);
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, Symbol &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret = (arr->Sym(1));
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, String &ret, bool b) const {
    const char *c;
    bool found = FindData(s, c, b);
    if (found) {
        ret = c;
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, int &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret = arr->Int(1);
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, float &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret = arr->Float(1);
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, bool &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret = arr->Int(1);
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, Plane &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret.a = arr->Float(1);
        ret.b = arr->Float(2);
        ret.c = arr->Float(3);
        ret.d = arr->Float(4);
        return true;
    } else {
        return false;
    }
}

bool DataArray::FindData(Symbol s, Hmx::Color &ret, bool b) const {
    DataArray *arr = FindArray(s, b);
    if (arr != nullptr) {
        ret.red = arr->Float(1);
        ret.green = arr->Float(2);
        ret.blue = arr->Float(3);
        if (arr->Size() > 4) {
            ret.alpha = arr->Float(4);
        } else {
            ret.alpha = 1;
        }
        return true;
    } else {
        return false;
    }
}

DataArray *DataArray::Clone(bool deep, bool eval, int extra) {
    DataArray *da = new DataArray(mSize + extra);
    for (int i = 0; i < mSize; i++) {
        da->mNodes[i] = (eval) ? mNodes[i].Evaluate() : mNodes[i];
        if (deep) {
            if (da->mNodes[i].Type() == kDataArray) {
                DataArray *cloned = da->mNodes[i].LiteralArray()->Clone(true, eval, 0);
                da->mNodes[i] = cloned;
                cloned->Release();
            }
        }
    }
    return da;
}

void DataArray::RandomSortNodes() {
    if (mSize > 0) {
        for (int i = 0; i < mSize; i++) {
            int newIdx = RandomInt(0, mSize);
            DataNode tmp = mNodes[i];
            mNodes[i] = mNodes[newIdx];
            mNodes[newIdx] = tmp;
        }
    }
}

DataNode DataArray::Execute(bool fail) {
    DataCallStackFrame frame(this);
    START_AUTO_TIMER_CALLBACK("array_exec", DataArrayGlitchCB, this);
    DataNode &node = (DataNode &)Evaluate(0);
    switch (node.Type()) {
    case kDataFunc: {
        return node.UncheckedFunc()(this);
    }
    case kDataObject: {
        if (node.UncheckedObj()) {
            return node.UncheckedObj()->Handle(this, true);
        }
        break;
    }
    case kDataSymbol: {
        const char *str = node.UncheckedStr();
        Hmx::Object *obj = gDataDir->FindObject(str, true, true);
        if (obj) {
            return obj->Handle(this, true);
        }
        std::map<Symbol, DataFunc *>::iterator func = gDataFuncs.find(STR_TO_SYM(str));
        if (func != gDataFuncs.end()) {
            // Cache the function into the array to optimize repeat calls
            node = func->second;
            return func->second(this);
        }
        break;
    }
    case kDataString: {
        Hmx::Object *object = gDataDir->FindObject(node.UncheckedStr(), true, true);
        if (object) {
            return object->Handle(this, true);
        }
        break;
    }
    default:
        break;
    }
    if (sDefaultHandler) {
        DataNode n = sDefaultHandler(this);
        if (n.Type() != kDataUnhandled) {
            return n;
        }
    }
    if (fail) {
        String str;
        Node(0).Print(str, true, 0);
        String str2;
        node.Print(str2, true, 0);
        if (str == str2) {
            MILO_FAIL(
                "%s not function or object (file %s, line %d)", str.c_str(), mFile, mLine
            );
        } else {
            MILO_FAIL(
                "%s = %s not function or object (file %s, line %d)",
                str.c_str(),
                str2.c_str(),
                mFile,
                mLine
            );
        }
    }
    return 0;
}

DataNode DataArray::ExecuteBlock(int len) {
    for (; len < mSize - 1; len++) {
        Command(len)->Execute(true);
    }
    return Evaluate(len);
}

DataNode DataArray::ExecuteScript(
    int index, Hmx::Object *obj, const DataArray *_args, int _argStart
) {
    DataCallStackFrame frame(this);

    int numVars = 0;
    int size = mSize;

    if (index < (size - 1) && mNodes[index].Type() == kDataArray) {
        DataArray *arr = mNodes[index].UncheckedArray();
        numVars = arr->Size();
        MILO_ASSERT(_args != NULL || numVars == 0, 0x4E3);

        for (int i = 0; i < numVars; i++) {
            DataNode *var = arr->Var(i);
            DataPushVar(var);
            *var = _args->Evaluate(i + _argStart);
        }

        index++;
    }

    DataNode ret;
    if (index >= size) {
        ret = DataNode(0);
    } else {
        Hmx::Object *setThis = DataSetThis(obj);
        ret = ExecuteBlock(index);
        DataSetThis(setThis);
    }

    while (numVars-- != 0) {
        DataPopVar();
    }

    return ret;
}

BinStream &operator>>(BinStream &bs, DataArray *&da) {
    bool b;
    bs >> b;
    if (b) {
        da = new DataArray(0);
        da->Load(bs);
    } else
        da = nullptr;
    return bs;
}

bool DataArrayDefined() {
    FOREACH (it, gConditional) {
        if (*it == false)
            return false;
    }
    return true;
}

void DataArray::Load(BinStream &d) {
    mFile = gFile;
    short size;
    d >> size;
    {
        MemTemp tmp;
        Resize(size);
    }
    d >> mLine;
    d >> mDeprecated;
    for (int i = 0; i < size;) {
        DataNode &node = mNodes[i];
        d >> node;
        if (!DataArrayDefined() && node.Type() != kDataIfdef && node.Type() != kDataIfndef
            && node.Type() != kDataElse && node.Type() != kDataEndif) {
            size -= 1;
        } else {
            DataArray *array = nullptr;
            if (node.Type() == kDataSymbol
                && (array = DataGetMacro(node.UncheckedSym()))) {
                size += array->Size() - 1;
                {
                    MemTemp tmp;
                    Resize(size);
                }

                for (int j = 0; j < array->Size(); j++) {
                    mNodes[i++] = array->Node(j);
                }
            } else if (node.Type() == kDataAutorun) {
                DataNode command;
                d >> command;
                command.Command(this)->Execute();
                size -= 2;
            } else if (node.Type() == kDataDefine) {
                DataNode macro;
                d >> macro;
                DataSetMacro(node.UncheckedSym(), macro.Array(this));
                size -= 2;
            } else if (node.Type() == kDataUndef) {
                DataSetMacro(node.UncheckedSym(), nullptr);
                size -= 1;
            } else if (node.Type() == kDataIfdef) {
                gConditional.push_back(DataGetMacro(node.UncheckedSym()) != nullptr);
                size -= 1;
            } else if (node.Type() == kDataIfndef) {
                gConditional.push_back(DataGetMacro(node.UncheckedSym()) == nullptr);
                size -= 1;
            } else if (node.Type() == kDataElse) {
                gConditional.back() = !gConditional.back();
                size -= 1;
            } else if (node.Type() == kDataEndif) {
                gConditional.pop_back();
                size -= 1;
            } else if (node.Type() == kDataInclude || node.Type() == kDataMerge) {
                const char *path = node.UncheckedStr();
                bool readFile = false;
                DataArray *macro = DataGetMacro(path);
                if (!macro) {
                    path = FileMakePath(FileGetPath(File()), path);
                    macro = DataReadFile(path, true);
                    readFile = true;
                    if (!macro) {
                        MILO_FAIL(
                            "Couldn't open embedded file: %s (file %s, line %d)",
                            path,
                            mFile.Str(),
                            mLine
                        );
                    }
                }

                if (node.Type() == kDataInclude) {
                    size += macro->Size() - 1;
                    {
                        MemTemp tmp;
                        Resize(size);
                    }
                    for (int j = 0; j < array->Size(); j++) {
                        mNodes[i++] = array->Node(j);
                    }
                } else {
                    if (macro->Size() == 0) {
                        MILO_FAIL(
                            "Empty merge file (possibly a re-included file): %s", path
                        );
                    }

                    int remaining = size - i - 1;
                    {
                        MemTemp tmp;
                        Resize(i);
                    }
                    DataMergeTags(this, macro);

                    i = mSize;
                    size = mSize + remaining;
                    {
                        MemTemp tmp;
                        Resize(size);
                    }
                }

                if (readFile) {
                    macro->Release();
                }

                gFile = mFile;
            } else {
                i++;
            }
        }
    }
    Resize(size);
}
