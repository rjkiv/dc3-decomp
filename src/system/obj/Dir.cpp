#include "obj/Dir.h"
#include "Msg.h"
#include "Object.h"
#include "Utl.h"
#include "math/Mtx.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/DirLoader.h"
#include "obj/DirUnloader.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "stl/_pair.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Option.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/BinStream.h"

const char *kNotObjectMsg = "Could not find %s in dir \"%s\"";
ObjectDir *ObjectDir::sMainDir;
ObjectDir *gDir;
std::map<std::pair<Symbol, Symbol>, bool> ObjectDir::sSuperClassMap;

BinStream &operator>>(BinStream &bs, InlineDirType &ty) {
    unsigned char uc;
    bs >> uc;
    ty = (InlineDirType)uc;
    return bs;
}

bool ObjectDir::SaveSubdirs() {
    return !IsProxy() || mProxyFile.empty() || gLoadingProxyFromDisk;
}

bool ObjectDir::ShouldSaveProxy(BinStream &bs) {
    return IsProxy() && (!mProxyFile.empty() || InlineProxy(bs));
}

void ObjectDir::SetInlineProxyType(InlineDirType t) {
    MILO_ASSERT(t != kInlineCachedShared, 0x198);
    mInlineProxyType = t;
}

void ObjectDir::Load(BinStream &bs) {
    PreLoad(bs);
    PostLoad(bs);
    if (IsProxy() && mLoader && !mLoader->IsLoaded()) {
        TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
    }
}

void ObjectDir::OldLoadProxies(BinStream &bs, int i) {
    int x;
    bs >> x;
    if (x != 0)
        MILO_FAIL("Proxies not allowed here");
}

BinStreamRev &operator>>(BinStreamRev &bs, ObjectDir::Viewport &v) {
    // bs >> v.mXfm;
    return bs;
}

void ObjectDir::TransferLoaderState(ObjectDir *dir) {
    mProxyFile = dir->mProxyFile;
    mProxyOverride = dir->mProxyOverride;
    mLoader = dir->mLoader;
    dir->mLoader = nullptr;
}

bool ObjectDir::HasDirPtrs() const {
    if (sDeleting == this) {
        return true;
    } else {
        for (ObjRef::iterator it = mRefs.begin(); it != mRefs.end(); ++it) {
            if (it->IsDirPtr())
                return true;
        }
        return false;
    }
}

namespace {
    int gPreloadIdx = 0;
    ObjDirPtr<ObjectDir> gPreloaded[128];

    void DeleteShared() {
        for (; gPreloadIdx > 0; gPreloadIdx--) {
            gPreloaded[gPreloadIdx - 1] = 0;
        }
    }
}

ObjectDir::Viewport &ObjectDir::CurViewport() {
    if (mCurViewportID >= kNumViewports) {
        MILO_FAIL("%s mCurView = %d, >= kNumViewports", PathName(this), mCurViewportID);
    }
    return mViewports[mCurViewportID];
}

bool ObjectDir::HasSubDir(ObjectDir *dir) {
    if (this == dir)
        return true;
    else {
        for (int i = 0; i < mSubDirs.size(); i++) {
            if (mSubDirs[i] && mSubDirs[i]->HasSubDir(dir)) {
                return true;
            }
        }
    }
    return false;
}

void ObjectDir::SaveProxy(BinStream &bs) {
    if (ShouldSaveProxy(bs) && InlineProxy(bs)) {
        gLoadingProxyFromDisk = true;
        const char *path = mProxyFile.empty() ? FilePath::Root().c_str()
                                              : FileGetPath(mProxyFile.c_str());
        FilePathTracker tracker(path);
        DirLoader::SaveObjects(bs, this);
    }
}

// void ObjectDir::ResetViewports() {
// mViewports[1]: (0 -1 0) (1 0 0) (0 0 1) (-768 0 0)
// mViewports[2]: (0 1 0) (-1 0 0) (0 0 1) (768 0 0)
// mViewports[3]: (1 0 0) (0 0 -1) (0 1 0) (0 0 768)
// mViewports[4]: (1 0 0) (0 0 1) (0 -1 0) (0 0 -768)
// mViewports[5]: (1 0 0) (0 1 0) (0 0 1) (0 -768 0)
// mViewports[6]: (-1 0 0) (0 -1 0) (0 0 1) (0 768 0)
// }

DataNode OnLoadObjects(DataArray *a) {
    return DirLoader::LoadObjects(a->Str(1), nullptr, nullptr);
}

DataNode OnPathName(DataArray *a) { return PathName(a->Obj<Hmx::Object>(1)); }

DataNode OnReserveToFit(DataArray *a) {
    ReserveToFit(a->Obj<ObjectDir>(1), a->Obj<ObjectDir>(2), a->Int(3));
    return 0;
}

DataNode OnInitObject(DataArray *a) {
    a->Obj<Hmx::Object>(1)->InitObject();
    return 0;
}

void ObjectDir::Reserve(int hashSize, int stringSize) {
    MemTemp tmp;
    if (mHashTable.Size() < hashSize) {
        mHashTable.Resize(hashSize, 0);
    }
    mStringTable.Reserve(stringSize);
}

void ObjectDir::SyncObjects() {
    static Message sync_objects("sync_objects");
    HandleType(sync_objects);
}

ObjectDir::InlinedDir::InlinedDir() : dir(), file() {}
ObjectDir::InlinedDir::~InlinedDir() {}

void ObjectDir::LoadSubDir(int i, const FilePath &fp, BinStream &bs, bool b) {
    if (IsProxy() && !mProxyFile.empty()) {
        mSubDirs[i] = 0;
    } else {
        FilePath subdirpath = GetSubDirPath(fp, bs);
        if (streq(mPathName, subdirpath.c_str())) {
            MILO_NOTIFY(
                "%s trying to subdir self in slot %d, setting NULL", PathName(this), i
            );
            mSubDirs[i] = 0;
        } else
            mSubDirs[i].LoadFile(subdirpath, true, b, kLoadFront, true);
    }
}

void PreloadArray(DataArray *arr, int idx) {
    for (int i = idx; i < arr->Size(); i++) {
        DataArray *curArr = arr->Array(i);
        if (curArr->Size() != 0) {
            if (curArr->Type(0) == kDataArray) {
                PreloadArray(curArr, 0);
            } else {
                const char *str = curArr->Str(0);
                bool shouldPop = false;
                if (curArr->Size() > 1) {
                    MemPushHeap(MemFindHeap(curArr->Sym(1).Str()));
                    shouldPop = true;
                }
                MILO_ASSERT(gPreloadIdx < DIM(gPreloaded), 0xA35);
                gPreloaded[gPreloadIdx++].LoadFile(str, false, true, kLoadFront, false);
                if (shouldPop) {
                    MemPopHeap();
                }
            }
        }
    }
}

void PreloadSharedSubdirs(Symbol s) {
    DataArray *arr = SystemConfig("preload_subdirs")->FindArray(s, false);
    if (arr) {
        PreloadArray(arr, 1);
    }
}

void ObjectDir::Terminate() {
    DeleteShared();
    sSuperClassMap.clear();
}

void ObjectDir::AddedSubDir(ObjDirPtr<ObjectDir> &subdir) {
    ObjectDir *dir = subdir;
    if (dir) {
        dir->InlineSubDirType();
        dir->SetSubDir(true);
        for (ObjDirItr<Hmx::Object> it(dir, true); it != nullptr; ++it) {
            AddedObject(it);
        }
    }
}

void ObjectDir::RemovingSubDir(ObjDirPtr<ObjectDir> &subdir) {
    ObjectDir *dir = subdir;
    if (dir) {
        dir->SetSubDir(false);
        for (ObjDirItr<Hmx::Object> it(dir, true); it != nullptr; ++it) {
            RemovingObject(it);
        }
    }
}

void ObjectDir::DeleteObjects() {
    for (ObjDirItr<Hmx::Object> it(this, false); it != nullptr; ++it) {
        if (it != this) {
            delete it;
        }
    }
}

void ObjectDir::RemoveSubDir(const ObjDirPtr<ObjectDir> &dPtr) {
    std::vector<ObjDirPtr<ObjectDir> >::iterator it = mSubDirs.begin();
    while (it != mSubDirs.end()) {
        if ((*it) == dPtr) {
            RemovingSubDir(*it);
            it = mSubDirs.erase(it);
            if (it == mSubDirs.end())
                break;
        }
        ++it;
    }
}

void ObjectDir::DeleteSubDirs() {
    for (int i = 0; i < mSubDirs.size(); i++) {
        RemovingSubDir(mSubDirs[i]);
    }
    mSubDirs.clear();
}

void CheckForDuplicates() {
    DataArray *cfg = SystemConfig("objects");
    std::list<Symbol> syms;
    for (int i = 1; i < cfg->Size(); i++) {
        syms.push_back(cfg->Array(i)->Sym(0));
    }
    syms.sort();
    Symbol previous;
    bool fail = false;
    for (std::list<Symbol>::iterator it = syms.begin(); it != syms.end();
         previous = *it, ++it) {
        Symbol cur = *it;
        if (cur == previous) {
            MILO_NOTIFY("Duplicate object %s in config", cur);
            fail = true;
        }
    }
    if (fail)
        MILO_FAIL("duplicate objects found in configs, bailing");
    syms.unique();
}

void ObjectDir::Init() {
    MessageTimer::Init();
    TheLoadMgr.RegisterFactory("milo", DirLoader::New);
    TheLoadMgr.RegisterFactory("milo_xbox", DirLoader::New);
    TheLoadMgr.RegisterFactory("milo_ps3", DirLoader::New);
    TheLoadMgr.RegisterFactory("milo_pc", DirLoader::New);
    TheLoadMgr.RegisterFactory("milo_ps2", DirLoader::New);
    TheLoadMgr.RegisterFactory("milo_wii", DirLoader::New);
    TheLoadMgr.RegisterFactory("rnd", DirLoader::New);
    TheLoadMgr.RegisterFactory("m2", DirLoader::New);
    TheLoadMgr.RegisterFactory("gh", DirLoader::New);
    TheLoadMgr.RegisterFactory("kr", DirLoader::New);
    CheckForDuplicates();
    DataRegisterFunc("load_objects", OnLoadObjects);
    DataRegisterFunc("init_object", OnInitObject);
    DataRegisterFunc("path_name", OnPathName);
    DataRegisterFunc("reserve_to_fit", OnReserveToFit);
    DirLoader::sPrintTimes = OptionBool("loader_times", false);
}

void ObjectDir::Iterate(DataArray *arr, bool b) {
    const DataNode &n = arr->Evaluate(2);
    Symbol s2;
    Symbol s8;
    if (n.Type() == kDataSymbol) {
        const char *str = n.UncheckedStr();
        s2 = STR_TO_SYM(str);
    } else {
        DataArray *a2 = n.UncheckedArray();
        s2 = a2->Sym(0);
        s8 = a2->Sym(1);
    }
    static DataArray *objects = SystemConfig("objects");
    objects->FindArray(s2);
    DataNode *var = arr->Var(3);
    DataNode varNode(*var);
    for (ObjDirItr<Hmx::Object> it(this, b); it != nullptr; ++it) {
        bool bbb;
        Symbol first = it->ClassName();
        std::pair<Symbol, Symbol> key = std::make_pair(first, s2);
        std::map<std::pair<Symbol, Symbol>, bool>::iterator superclassIt =
            sSuperClassMap.find(key);
        if (superclassIt == sSuperClassMap.end()) {
            bbb = IsASubclass(first, s2);
            sSuperClassMap[key] = bbb;
        } else
            bbb = superclassIt->second;
        if (bbb && (s2.Null() || it->Type() == s2)) {
            *var = &*it;
            for (int i = 4; i < arr->Size(); i++) {
                arr->Command(i)->Execute(true);
            }
        }
    }
    *var = varNode;
}

ObjDirPtr<ObjectDir> ObjectDir::PostLoadInlined() {
    MILO_ASSERT(mInlinedDirs.size() > 0, 0x296);
    InlinedDir iDir = mInlinedDirs.back();
    mInlinedDirs.pop_back();
    if (mInlinedDirs.size() == 0) {
        ClearAndShrink(mInlinedDirs);
    }
    if (iDir.shared && iDir.file.length() != 0 && !iDir.dir) {
        MILO_NOTIFY("Couldn't load shared inlined file %s\n", iDir.file);
    }
    return iDir.dir;
}

void ObjectDir::SetProxyFile(const FilePath &fp, bool b) {
    if (!IsProxy()) {
        MILO_NOTIFY("Can't set proxy file if own dir");
    } else {
        mProxyFile = fp;
        mProxyOverride = b;
        if (!b) {
            DeleteObjects();
            DeleteSubDirs();
            if (!mProxyFile.empty()) {
                DirLoader *dl = new DirLoader(
                    mProxyFile, kLoadFront, nullptr, nullptr, this, false, nullptr
                );
                TheLoadMgr.PollUntilLoaded(dl, nullptr);
            }
        }
    }
}

ObjectDir::ObjectDir()
    : mHashTable(0, Entry(), Entry(), 0), mStringTable(0), mProxyOverride(false),
      mInlineProxyType(kInlineCached), mLoader(nullptr), mIsSubDir(false),
      mInlineSubDirType(kInlineNever), mPathName(gNullStr), mViewports(7),
      mCurViewportID((ViewportId)0), unk8c(nullptr), mCurCam(nullptr), mAlwaysInlined(0),
      mAlwaysInlineHash(gNullStr) {
    ResetViewports();
}

ObjectDir::~ObjectDir() {
    mSubDirs.clear();
    delete mLoader;
    if (TheLoadMgr.AsyncUnload()) {
        new DirUnloader(this);
    } else {
        DeleteObjects();
        DeleteSubDirs();
    }
    if (!IsProxy()) {
        SetName(nullptr, nullptr);
    }
    if (mPathName != gNullStr) {
        MemOrPoolFree(strlen(mPathName) + 1, (void *)mPathName);
    }
    if (mAlwaysInlineHash != gNullStr) {
        MemOrPoolFree(strlen(mAlwaysInlineHash) + 1, (void *)mAlwaysInlineHash);
    }
}

ObjectDir::Entry *ObjectDir::FindEntry(const char *name, bool add) {
    if (name == 0 || *name == '\0')
        return 0;
    else {
        Entry *entry = mHashTable.Find(name);
        if (!entry && add) {
            Entry newEntry;
            newEntry.name = mStringTable.Add(name);
            entry = mHashTable.Insert(newEntry);
        }
        return entry;
    }
}

Hmx::Object *ObjectDir::FindObject(const char *name, bool parentDirs, bool b3) {
    Entry *entry = FindEntry(name, false);
    if (entry && entry->obj)
        return entry->obj;
    if (b3) {
        for (int i = 0; i < mSubDirs.size(); i++) {
            if (mSubDirs[i]) {
                Hmx::Object *found = mSubDirs[i]->FindObject(name, false, true);
                if (found)
                    return found;
            }
        }
    }
    if (strlen(name) != 0) {
        if (strcmp(name, Name()) == 0) {
            return this;
        }
    }
    if (parentDirs) {
        if (Dir() && Dir() != this) {
            return Dir()->FindObject(name, parentDirs, true);
        }
        if (this != sMainDir) {
            return sMainDir->FindObject(name, false, true);
        }
    }
    return nullptr;
}

ObjectDir *ObjectDir::FindContainingDir(const char *name) {
    if (FindEntry(name, false))
        return this;
    for (int i = 0; i < mSubDirs.size(); i++) {
        if (mSubDirs[i]) {
            ObjectDir *subdir = mSubDirs[i]->FindContainingDir(name);
            if (subdir)
                return subdir;
        }
    }
    return nullptr;
}

void ObjectDir::AppendSubDir(const ObjDirPtr<ObjectDir> &subdir) {
    mSubDirs.push_back(subdir);
    AddedSubDir(mSubDirs.back());
}

ObjectDir *SyncSubDir(const FilePath &fp, ObjectDir *dir) {
    Loader *loader = TheLoadMgr.GetLoader(fp);
    DirLoader *dirLoader = dir->IsProxy()
        ? dynamic_cast<DirLoader *>(loader)
        : dynamic_cast<DirLoader *>(TheLoadMgr.ForceGetLoader(fp));
    if (dirLoader) {
        ObjectDir *retDir = dirLoader->GetDir();
        if (retDir) {
            for (ObjDirItr<Hmx::Object> it(dir, false); it != nullptr; ++it) {
                Hmx::Object *found = retDir->FindObject(it->Name(), false, true);
                if (found && found != retDir && &*it != dir) {
                    MILO_NOTIFY(
                        "%s exists in dir and subdir, so replacing %s with %s",
                        it->Name(),
                        PathName(it),
                        PathName(found)
                    );
                    it->ReplaceRefs(found);
                    delete it;
                }
            }
        }
        return retDir;
    }
    // i would think you'd wanna return nullptr here if there's no dirLoader
    // but this is what HMX did
}

DataNode ObjectDir::OnFind(DataArray *da) {
    Hmx::Object *found = FindObject(da->Str(2), false, true);
    if (da->Size() > 3) {
        if (da->Int(3) != 0 && !found) {
            MILO_FAIL("Couldn't find %s in %s", da->Str(2), Name());
        }
    }
    return found;
}

bool PropSyncSubDirs(
    std::vector<ObjDirPtr<ObjectDir> > &, DataNode &, DataArray *, int, PropOp
);

BEGIN_PROPSYNCS(ObjectDir)
    gDir = this;
    {
        static Symbol _s("subdirs");
        if (sym == _s) {
            PropSyncSubDirs(mSubDirs, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    SYNC_PROP_SET(
        proxy_file,
        FileRelativePath(FilePath::Root().c_str(), ProxyFile().c_str()),
        SetProxyFile(_val.Str(), false)
    )
    SYNC_PROP(inline_proxy, (int &)mInlineProxyType)
    SYNC_PROP_SET(path_name, mPathName, )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void ObjectDir::PreInit(int hashSize, int stringSize) {
    REGISTER_OBJ_FACTORY(Hmx::Object);
    REGISTER_OBJ_FACTORY(ObjectDir);
    sMainDir = new ObjectDir();
    sMainDir->Reserve(hashSize, stringSize);
    sMainDir->SetName("main", sMainDir);
    DataSetThis(sMainDir);
    sSuperClassMap.clear();
    if (UsingCD()) {
        DirLoader::SetCacheMode(true);
    }
}

BEGIN_HANDLERS(ObjectDir)
    HANDLE_ACTION(iterate, Iterate(_msg, true))
    HANDLE_ACTION(iterate_self, Iterate(_msg, false))
    HANDLE_ACTION(save_objects, DirLoader::SaveObjects(_msg->Str(2), this, false))
    HANDLE(find, OnFind)
    HANDLE_EXPR(exists, FindObject(_msg->Str(2), false, true) != nullptr)
    HANDLE_ACTION(sync_objects, SyncObjects())
    HANDLE_EXPR(is_proxy, IsProxy())
    HANDLE_EXPR(proxy_dir, mLoader ? mLoader->ProxyDir() : NULL_OBJ)
    HANDLE_EXPR(
        proxy_name, mLoader ? (mLoader->ProxyName() ? mLoader->ProxyName() : "") : ""
    )
    HANDLE_ACTION(
        add_names,
        Reserve(
            mHashTable.Size() + _msg->Int(2) * 2,
            mStringTable.Size() + _msg->Int(2) * 0x14
        )
    )
    HANDLE_ACTION(override_proxy, SetProxyFile(_msg->Str(2), true))
    HANDLE_ACTION(delete_loader, RELEASE(mLoader))
    HANDLE_ACTION(reset_editor_state, ResetEditorState())
    HANDLE_EXPR(get_path_name, mPathName)
    HANDLE_EXPR(get_file_name, FileGetName(mPathName))
    // "get_file_name"
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void ObjectDir::SaveInlined(const FilePath &fp, bool share, InlineDirType type) {
    MILO_ASSERT(type != kInlineNever, 0x26A);
    if (type == kInlineAlways && share) {
        MILO_NOTIFY("Can't share kInlineAlways Dirs");
        share = false;
    }
    InlinedDir dir;
    dir.file = fp;
    dir.shared = share;
    dir.inlineDirType = type;
    mInlinedDirs.push_back(dir);
}

void ObjectDir::PreLoadInlined(const FilePath &fp, bool share, InlineDirType type) {
    MILO_ASSERT(type != kInlineNever, 0x27C);
    if (type == kInlineAlways && share) {
        MILO_NOTIFY("Can't share kInlineAlways Dirs");
        share = false;
    }
    InlinedDir dir;
    dir.file = fp;
    dir.shared = share;
    dir.inlineDirType = type;
    mInlinedDirs.push_back(dir);
}

void ObjectDir::ResetEditorState() {
    mViewports.resize(7);
    unk8c = 0;
    mCurCam = 0;
    mAlwaysInlined = 0;
    ResetViewports();
}

inline BinStream &operator<<(BinStream &bs, const ObjectDir::Viewport &v) {
    bs << v.mXfm;
    return bs;
}

void ObjectDir::Save(BinStream &bs) {
    bs << 0x1C;
    SaveType(bs);
    bs << mAlwaysInlined;
    if (mAlwaysInlineHash && !bs.Cached()) {
        int len = strlen(mAlwaysInlineHash);
        bs << len;
        bs.Write(mAlwaysInlineHash, len);
    } else {
        bs << 0;
    }
    bs << mViewports;
    bs << mCurViewportID;
    bs << mIsSubDir;
    bs << FileRelativePath(FilePath::Root().c_str(), mProxyFile.c_str());
    std::vector<ObjDirPtr<ObjectDir> > inlinedSubDirs;
    std::vector<ObjDirPtr<ObjectDir> > notInlinedSubDirs;
    if (SaveSubdirs()) {
        for (int i = 0; i < mSubDirs.size(); i++) {
            if (mSubDirs[i]) {
                std::vector<ObjDirPtr<ObjectDir> > &vec =
                    mSubDirs[i]->InlineSubDirType() ? inlinedSubDirs : notInlinedSubDirs;
                vec.push_back(mSubDirs[i]);
            }
        }
    }
    bs << notInlinedSubDirs;
    bs << mInlineSubDirType;
    bs << inlinedSubDirs;

    for (int i = 0; i < inlinedSubDirs.size(); i++) {
        unsigned char iType = inlinedSubDirs[i]->InlineSubDirType();
        bs << iType;
    }

    std::vector<bool> boolVec;
    boolVec.resize(mInlinedDirs.size(), false);
}
