#include "obj/DirLoader.h"
#include "Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Dir.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/BinStream.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemPoint.h"
#include "utl/MemTrack.h"
#include "utl/Str.h"
#include "utl/TextFileStream.h"
#include "utl/TextStream.h"
#include <map>

bool DirLoader::sPrintTimes;
bool DirLoader::sCacheMode;
ObjectDir *DirLoader::sTopSaveDir;
TextFileStream *DirLoader::sObjectMemDumpFile;
TextFileStream *DirLoader::sTypeMemDumpFile;
std::map<String, MemPointDelta> DirLoader::sMemPointMap;

DirLoader::DirLoader(
    const FilePath &fp,
    LoaderPos pos,
    Loader::Callback *cb,
    BinStream *stream,
    ObjectDir *dir,
    bool bbb,
    ObjectDir *dir2
)
    : Loader(fp, pos), mOwnStream(false), mStream(stream), mRev(0), mCounter(0),
      mObjects(nullptr, kObjListAllowNull), mCallback(cb), mDir(dir), mPostLoad(false),
      mLoadDir(true), mDeleteSelf(false), mProxyName(nullptr), unk98(0), unk99(0),
      unk9a(0), unk9b(bbb), unk9c(dir2), mProxyDir(this) {
    if (dir) {
        mDeleteSelf = true;
        mProxyName = dir->Name();
        mProxyDir = dir->Dir();
        mDir->SetLoader(this);
    }
    if (!stream && !dir && !bbb) {
        DataArray *arr = SystemConfig()->FindArray("force_milo_inline", false);
        if (arr) {
            for (int i = 1; i < arr->Size(); i++) {
                const char *str = arr->Str(i);
                if (FileMatch(fp.c_str(), str)) {
                    MILO_FAIL("Can't dynamically load milo files matching %s", str);
                }
            }
        }
    }
    if (fp.empty()) {
        mRoot = FilePath::Root();
    } else {
        char buf[256];
        strcpy(buf, FileGetPath(mFile.c_str()));
        int bufLen = strlen(buf) - 4;
        if (bufLen > 0 && streq("/gen", &buf[bufLen])) {
            buf[bufLen] = 0;
        }
        mRoot = FileMakePath(FileRoot(), buf);
    }
    mState = &DirLoader::OpenFile;
}

DirLoader::~DirLoader() {
    mDeleteSelf = false;
    if (!IsLoaded()) {
        Cleanup(nullptr);
    } else if (mDir) {
        mDir->SetLoader(nullptr);
        if (!unk98 && !mProxyName) {
            RELEASE(mDir);
        }
    }
    mProxyDir = nullptr;
    if (mCallback && unk99) {
        mCallback->FailedLoading(this);
        mCallback = 0;
    }
}

bool DirLoader::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mProxyDir) {
        mProxyDir = nullptr;
        mProxyName = nullptr;
        delete this; // uhhh.
        return true;
    } else
        return false;
}

const char *DirLoader::DebugText() { return MakeString("DL: %s", mFile.c_str()); }
bool DirLoader::IsLoaded() const { return mState == &DirLoader::DoneLoading; }

const char *DirLoader::StateName() const {
    if (mState == &DirLoader::OpenFile)
        return "OpenFile";
    else if (mState == &DirLoader::LoadHeader)
        return "LoadHeader";
    else if (mState == &DirLoader::LoadDir)
        return "LoadDir";
    else if (mState == &DirLoader::LoadResources)
        return "LoadResources";
    else if (mState == &DirLoader::CreateObjects)
        return "CreateObjects";
    else if (mState == &DirLoader::LoadObjs)
        return "LoadObjs";
    else if (mState == &DirLoader::DoneLoading)
        return "DoneLoading";
    else
        return "INVALID";
}

void DirLoader::SetCacheMode(bool mode) { sCacheMode = mode; }

ObjectDir *DirLoader::GetDir() {
    MILO_ASSERT(IsLoaded(), 0x82);
    unk98 = true;
    return mDir;
}

int DirLoader::ClassAndNameSort::ClassIndex(Hmx::Object *obj) {
    static DataArray *cfg = SystemConfig("system", "dir_sort");
    Symbol name = obj->ClassName();
    for (int i = cfg->Size() - 1; i != 0; i--) {
        DataNode &n = cfg->Node(i);
        if ((unsigned int)n.UncheckedInt() == name) {
            return i;
        }
    }
    return cfg->Size();
}

void WriteDeadAndMark(BinStream &bs) {
    bs << (unsigned char)0xAD << (unsigned char)0xDE << (unsigned char)0xAD
       << (unsigned char)0xDE;
    MarkChunk(bs);
}

const char *DirLoader::CachedPath(const char *cc, bool b) {
    const char *ext = FileGetExt(cc);
    if ((sCacheMode || b) && ext) {
        if (streq(ext, "milo")) {
            return MakeString(
                "%s/gen/%s.milo_%s",
                FileGetPath(cc),
                FileGetBase(cc),
                PlatformSymbol(TheLoadMgr.GetPlatform())
            );
        }
    }
    return cc;
}

bool DirLoader::ShouldBlockSubdirLoad(const FilePath &fp) {
    if (!fp.c_str()) {
        return false;
    } else if (!sPathEval) {
        return false;
    } else {
        return sPathEval(fp.c_str());
    }
}

// I am WELL aware that this is terrible
// however, the alternate to this would be using gotos,
// which I would argue is more terrible
// so unless you can finnagle this in such a way
// that the decomp % matches, and there aren't any gotos,
// this will have to do.
Symbol DirLoader::FixClassName(Symbol orig) {
    if (mRev < 0x1C) {
        static Symbol CharClip("CharClip");
        static Symbol CharClipSamples("CharClipSamples");
        if (orig == CharClipSamples)
            orig = CharClip;
        if (mRev < 0x1B) {
            static Symbol BandMeshLauncher("BandMeshLauncher");
            static Symbol PartLauncher("PartLauncher");
            if (orig == BandMeshLauncher) {
                orig = PartLauncher;
            }
            if (mRev < 0x1A) {
                static Symbol P9TransDraw("P9TransDraw");
                static Symbol CharTransDraw("CharTransDraw");
                if (orig == P9TransDraw)
                    orig = CharTransDraw;
                if (mRev < 0x19) {
                    static Symbol CompositeTexture("CompositeTexture");
                    static Symbol RenderedTex("RenderedTex");
                    static Symbol TexRenderer("TexRenderer");
                    static Symbol LayerDir("LayerDir");
                    if (orig == RenderedTex)
                        orig = TexRenderer;
                    else if (orig == CompositeTexture)
                        orig = LayerDir;
                    if (mRev < 0x18) {
                        static Symbol WorldFx("WorldFx");
                        static Symbol BandFx("BandFx");
                        if (orig == BandFx)
                            return WorldFx;
                        if (mRev < 0x16) {
                            static Symbol Slider("Slider");
                            static Symbol BandSlider("BandSlider");
                            if (orig == Slider)
                                return BandSlider;
                            if (mRev < 0x15) {
                                static Symbol TextEntry("TextEntry");
                                static Symbol BandTextEntry("BandTextEntry");
                                if (orig == TextEntry)
                                    return BandTextEntry;
                                if (mRev < 0x14) {
                                    static Symbol Placer("Placer");
                                    static Symbol BandPlacer("BandPlacer");
                                    if (orig == Placer)
                                        return BandPlacer;
                                    if (mRev < 0x13) {
                                        static Symbol ButtonEx("ButtonEx");
                                        static Symbol BandButton("BandButton");
                                        if (orig == ButtonEx)
                                            return BandButton;

                                        static Symbol LabelEx("LabelEx");
                                        static Symbol BandLabel("BandLabel");
                                        if (orig == LabelEx)
                                            return BandLabel;

                                        static Symbol PictureEx("PictureEx");
                                        static Symbol BandPicture("BandPicture");
                                        if (orig == PictureEx)
                                            return BandPicture;
                                        if (mRev < 0x12) {
                                            static Symbol UIPanel("UIPanel");
                                            static Symbol PanelDir("PanelDir");
                                            if (orig == UIPanel)
                                                return PanelDir;
                                            if (mRev < 0x10) {
                                                static Symbol WorldInstance(
                                                    "WorldInstance"
                                                );
                                                static Symbol WorldObject("WorldObject");
                                                if (orig == WorldInstance)
                                                    return WorldObject;

                                                if (mRev < 0xF) {
                                                    static Symbol Group("Group");
                                                    static Symbol View("View");
                                                    if (orig == View)
                                                        return Group;

                                                    if (mRev < 7) {
                                                        static Symbol String("String");
                                                        static Symbol Line("Line");
                                                        if (orig == String)
                                                            return Line;
                                                        if (mRev < 6) {
                                                            static Symbol MeshGenerator(
                                                                "MeshGenerator"
                                                            );
                                                            static Symbol Generator(
                                                                "Generator"
                                                            );
                                                            if (orig == MeshGenerator)
                                                                return Generator;
                                                            if (mRev < 5) {
                                                                static Symbol TexMovie(
                                                                    "TexMovie"
                                                                );
                                                                static Symbol Movie(
                                                                    "Movie"
                                                                );
                                                                if (orig == TexMovie)
                                                                    return Movie;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return orig;
}

void ReadDead(BinStream &bs) {
    unsigned char val;
    bs >> val;
    while (true) {
        if (val == 0xAD) {
            bs >> val;
            if (val == 0xDE) {
                bs >> val;
                if (val == 0xAD) {
                    bs >> val;
                    if (val == 0xDE) {
                        return;
                    }
                }
            }
        } else {
            bs >> val;
        }
    }
}

void ReadEditorDirDead(BinStream &bs) {
    unsigned char buf;
    for (int i = 0; i < 20; i++) {
        while (true) {
            EofType t;
            while ((t = bs.Eof()) != NotEof) {
                MILO_ASSERT(t == TempEof, 0x470);
            }
            bs >> buf;
            const char *str = "%#@EndOfEditorDir@#%";
            if ((unsigned char)str[i] != buf) {
                i = 0;
            } else {
                break;
            }
        }
    }
}

void DirLoader::DumpObjectMemDelta(
    const Hmx::Object *object, const MemPointDelta &memDelta
) const {
    MILO_ASSERT(object, 0x56A);
    MILO_ASSERT(sObjectMemDumpFile, 0x56B);
    if (memDelta.AnyGreaterThan(0)) {
        const char *name = object->Name();
        if (!name || !*name)
            name = "Unknown";
        const char *objPtrStr = MakeString("0x%X", (void *)object);
        *sObjectMemDumpFile << objPtrStr << "," << object->ClassName() << "," << name
                            << "," << mFile.c_str() << "," << memDelta.ToString(1)
                            << "\n";
    }
}

void SyncObjectsGlitchCB(float ms, void *v) {
    ObjectDir *dir = (ObjectDir *)v;
    const char *path = PathName(dir);
    MILO_LOG("%s %s SyncObjects took %.2f ms\n", dir->ClassName(), path, ms);
}

DirLoader *DirLoader::Find(const FilePath &fp) {
    if (!fp.empty()) {
        FOREACH (it, TheLoadMgr.Loaders()) {
            if ((*it)->LoaderFile() == fp) {
                DirLoader *dl = dynamic_cast<DirLoader *>(*it);
                if (dl)
                    return dl;
            }
        }
    }
    return nullptr;
}

bool DirLoader::ClassAndNameSort::operator()(Hmx::Object *o1, Hmx::Object *o2) {
    static Symbol ObjectDir("ObjectDir");
    bool o1sub = IsASubclass(o1->ClassName(), ObjectDir);
    if (o1sub != IsASubclass(o2->ClassName(), ObjectDir)) {
        return o1sub;
    } else {
        int idx1 = ClassIndex(o1);
        int idx2 = ClassIndex(o2);
        if (idx1 != idx2) {
            return idx1 < idx2;
        } else
            return strcmp(o1->Name(), o2->Name()) < 0;
    }
    return false;
}

DirLoader *DirLoader::FindLast(const FilePath &fp) {
    if (!fp.empty()) {
        FOREACH_REVERSE(it, TheLoadMgr.Loaders()) {
            if ((*it)->LoaderFile() == fp) {
                DirLoader *dl = dynamic_cast<DirLoader *>(*it);
                if (dl)
                    return dl;
            }
        }
    }
    return nullptr;
}

void DirLoader::WriteTypeMemDump(TextFileStream *file) {
    MILO_ASSERT(file, 0x5B1);
    *file << "Class," << MemPointDelta::HeaderString("") << "\n";
    FOREACH (it, sMemPointMap) {
        MemPointDelta pt = it->second;
        *file << it->first << "," << pt.ToString(1) << "\n";
    }
    file->File().Flush();
}

void DirLoader::Cleanup(const char *str) {
    if (str) {
        MILO_NOTIFY(str);
    }
    mObjects.clear();
    if (mOwnStream)
        RELEASE(mStream);
    if (mDir) {
        if (!IsLoaded()) {
            mDir->SetLoader(nullptr);
            if (!mProxyName) {
                RELEASE(mDir);
            }
        }
        if (mProxyName) {
            if (mDir->Dir() == mDir) {
                mDir->SetName(mProxyName, mProxyDir);
            }
        }
        if (IsLoaded() && mDir) {
            AutoGlitchReport report(50.0f, SyncObjectsGlitchCB, mDir);
            mDir->SetSubDirFlag(unk9b);
            mDir->SyncObjects();
            mDir->SetSubDirFlag(false);
        }
    }
    mState = &DirLoader::DoneLoading;
    mTimer.Stop();
    if (sPrintTimes) {
        MILO_LOG("%s: %f ms\n", mFile, mTimer.Ms());
    }
    if (mCallback && (str || unk99)) {
        mCallback->FailedLoading(this);
        mCallback = nullptr;
    }
    if (mDeleteSelf) {
        delete this;
    }
}

void DirLoader::AddTypeObjectMemDelta(
    const Hmx::Object *object, const MemPointDelta &memDelta
) const {
    MILO_ASSERT(object, 0x584);
    MILO_ASSERT(sTypeMemDumpFile, 0x585);
    if (memDelta.AnyGreaterThan(0)) {
        const char *name = object->ClassName().Str();
        if (!name || !*name) {
            name = "Unknown";
        }
        auto it = sMemPointMap.find(String(name));
        if (it != sMemPointMap.end()) {
            it->second += memDelta;
        } else {
            it = sMemPointMap.insert(std::make_pair(name, MemPointDelta())).first;
            it->second += memDelta;
        }
    }
}

void DirLoader::SaveObjects(BinStream &bs, ObjectDir *dir) {
    char name[256];
    MILO_ASSERT(sTopSaveDir != dir, 0x10C);
    if (!sTopSaveDir)
        sTopSaveDir = dir;
    ObjectDir *parentDir = dir->Dir();
    strcpy(name, dir->Name());
    if (parentDir != dir) {
        dir->SetName(NextName(dir->Name(), dir), dir);
    }
    int hashSize = dir->HashTableUsedSize();
    int strSize = dir->StrTableUsedSize();
    for (ObjDirItr<Hmx::Object> it(dir, false); it != nullptr; ++it) {
        if (it != dir) {
            it->PreSave(bs);
        }
    }
    dir->PreSave(bs);
    bs << 0x20;
    bs << dir->ClassName() << dir->Name();
    bs << hashSize * 2;
    bs << strSize;
    bs << false;
    std::list<Hmx::Object *> objects;
    for (ObjDirItr<Hmx::Object> it(dir, false); it != nullptr; ++it) {
        if (it != dir) {
            objects.push_back(it);
        }
    }
    objects.sort(ClassAndNameSort());
    bs << objects.size();
    for (std::list<Hmx::Object *>::const_iterator it = objects.begin();
         it != objects.end();
         it++) {
        bs << (*it)->ClassName() << (*it)->Name();
    }
    SetActiveChunkObject(dir);
    dir->Save(bs);
    WriteDeadAndMark(bs);
    for (std::list<Hmx::Object *>::const_iterator it = objects.begin();
         it != objects.end();
         it++) {
        SetActiveChunkObject(*it);
        (*it)->Save(bs);
        WriteDeadAndMark(bs);
        ObjectDir *proxy = dynamic_cast<ObjectDir *>(*it);
        if (proxy)
            proxy->SaveProxy(bs);
    }
    if (!bs.Cached()) {
        dir->PostSave(bs);
        for (std::list<Hmx::Object *>::iterator it = objects.begin(); it != objects.end();
             it++) {
            (*it)->PostSave(bs);
        }
    }
    if (parentDir != dir) {
        dir->SetName(name, parentDir);
    }
    if (sTopSaveDir == dir) {
        sTopSaveDir = nullptr;
    }
}

bool DirLoader::SaveObjects(const char *file, ObjectDir *dir, bool) {
    if (sCacheMode && dir->InlineSubDirType() != kInlineNever) {
        MILO_LOG("Not caching %s because it is an inlined subdir.\n", file);
        return false;
    } else {
        FilePathTracker tracker(FileGetPath(file));
        file = CachedPath(file, false);
        if (sCacheMode) {
            FileMkDir(FileGetPath(file));
        }
        Platform p = sCacheMode ? TheLoadMgr.GetPlatform() : kPlatformPC;
        MILO_ASSERT(p != kPlatformNone, 0x1B3);
        bool noNulls = !gNullFiles;
        ChunkStream cs(
            file,
            ChunkStream::kWrite,
            p == kPlatformWii ? 0x10000 : 0x20000,
            noNulls,
            p,
            sCacheMode
        );
        if (cs.Fail()) {
            MILO_NOTIFY("Could not open file: %s", file);
            return false;
        } else {
            SaveObjects(cs, dir);
            return true;
        }
    }
}

ObjectDir *DirLoader::LoadObjects(const FilePath &fp, Callback *cb, BinStream *bs) {
    if (sTypeMemDumpFile) {
        sMemPointMap.clear();
    }
    DirLoader dirLoader(fp, kLoadFront, cb, bs, nullptr, false, nullptr);
    TheLoadMgr.PollUntilLoaded(&dirLoader, nullptr);
    if (sTypeMemDumpFile) {
        WriteTypeMemDump(sTypeMemDumpFile);
    }
    return dirLoader.GetDir();
}

Symbol DirLoader::GetDirClass(const char *cc1) {
    ChunkStream stream(cc1, ChunkStream::kRead, 0x10000, true, kPlatformNone, false);
    if (stream.Fail()) {
        return "";
    } else {
        EofType t;
        while ((t = stream.Eof()) != NotEof) {
            MILO_ASSERT(t == TempEof, 0xC3);
        }
        int x;
        stream >> x;
        Symbol ret;
        stream >> ret;
        return ret;
    }
}

bool DirLoader::SetupDir(Symbol s1) {
    MemPoint pt(MemPoint::kInitType0);
    if (sObjectMemDumpFile || sTypeMemDumpFile) {
        pt = MemPoint(MemPoint::kInitType1);
    }
    if (mDir) {
        if (mDir->ClassName() != s1) {
            if (mDir->IsProxy()) {
                MILO_NOTIFY(MakeString(
                    "%s: Proxy %s class %s not %s, converting",
                    PathName(mDir->Dir()),
                    LoaderFile().c_str(),
                    mDir->ClassName(),
                    s1
                ));
            } else {
                MILO_NOTIFY(MakeString(
                    "%s: Proxy class %s not %s, converting",
                    LoaderFile().c_str(),
                    mDir->ClassName(),
                    s1
                ));
            }
            ObjectDir *newDir = dynamic_cast<ObjectDir *>(Hmx::Object::NewObject(s1));
            if (!newDir) {
                Cleanup(MakeString(
                    "%s: Trying to make non ObjectDir proxy class %s s",
                    LoaderFile().c_str(),
                    mDir->ClassName(),
                    s1
                ));
                return false;
            }
            newDir->TransferLoaderState(mDir);
            ReplaceObject(mDir, newDir, true, true, false);
            mDir = newDir;
        }
    } else {
        mDir = dynamic_cast<ObjectDir *>(Hmx::Object::NewObject(s1));
    }
    mDir->SetPathName(LoaderFile().c_str());
    if (sObjectMemDumpFile) {
        MemPoint dumpPt;
        DumpObjectMemDelta(mDir, dumpPt - pt);
    }
    if (sTypeMemDumpFile) {
        MemPoint dumpPt;
        AddTypeObjectMemDelta(mDir, dumpPt - pt);
    }
    return true;
}

void DirLoader::LoadObjs() {
    FilePathTracker tracker(mRoot.c_str());
    EofType t;
    while (!mObjects.empty()) {
        t = mStream->Eof();
        if (t != NotEof) {
            MILO_ASSERT(t == TempEof, 0x4C0);
        } else {
            Hmx::Object *obj = mObjects.front();
            if (obj) {
                if (!mPostLoad) {
                    MemPoint pt(MemPoint::kInitType0);
                    if (sObjectMemDumpFile || sTypeMemDumpFile) {
                        pt = MemPoint(MemPoint::kInitType1);
                    }
                    if (streq(obj->Name(), "")) {
                        BeginMemTrackObjectName(mProxyName);
                    } else {
                        BeginMemTrackObjectName(obj->Name());
                    }
                    if (mDir) {
                        BeginMemTrackFileName(mDir->GetPathName());
                    }
                    obj->PreLoad(*mStream);
                    mPostLoad = true;
                    if (sObjectMemDumpFile) {
                        MemPoint start;
                        DumpObjectMemDelta(obj, start - pt);
                    }
                    if (sTypeMemDumpFile) {
                        MemPoint start;
                        AddTypeObjectMemDelta(obj, start - pt);
                    }
                    EndMemTrackFileName();
                    EndMemTrackObjectName();
                }
                if (TheLoadMgr.GetFirstLoading() == this) {
                    MemPoint pt(MemPoint::kInitType0);
                    if (sObjectMemDumpFile || sTypeMemDumpFile) {
                        pt = MemPoint(MemPoint::kInitType1);
                    }
                    if (streq(obj->Name(), "")) {
                        BeginMemTrackObjectName(mProxyName);
                    } else {
                        BeginMemTrackObjectName(obj->Name());
                    }
                    if (mDir) {
                        BeginMemTrackFileName(mDir->GetPathName());
                    }
                    obj->PostLoad(*mStream);
                    mPostLoad = false;
                    if (sObjectMemDumpFile) {
                        MemPoint start;
                        DumpObjectMemDelta(obj, start - pt);
                    }
                    if (sTypeMemDumpFile) {
                        MemPoint start;
                        AddTypeObjectMemDelta(obj, start - pt);
                    }
                    EndMemTrackFileName();
                    EndMemTrackObjectName();
                    if (mRev > 1) {
                        ReadDead(*mStream);
                    }
                } else {
                    return;
                }

            } else {
                MILO_ASSERT(mRev > 1, 0x507);
                ReadDead(*mStream);
            }
            mObjects.pop_front();
        }
        if (TheLoadMgr.CheckSplit() || TheLoadMgr.GetFirstLoading() != this) {
            return;
        }
    }
    mState = &DirLoader::DoneLoading;
    if (mRev > 0x1D) {
        if (mRev == 0x1E) {
            EofType t;
            while ((t = mStream->Eof()) != NotEof) {
                MILO_ASSERT(t == TempEof, 0x524);
            }
            ReadDead(*mStream);
        } else if (mRev == 0x1F) {
            ReadEditorDirDead(*mStream);
        }
        if (unk9a && mRev > 0x1F) {
            ReadEditorDirDead(*mStream);
        }
    }
    Cleanup(0);
    if (TheLoadMgr.GetFirstLoading() != this)
        return;
    if (mCallback)
        mCallback->FinishLoading(this);
}

void DirLoader::LoadDir() {
    if (mLoadDir) {
        FilePathTracker tracker(mRoot.c_str());
        bool oldproxy = gLoadingProxyFromDisk;
        gLoadingProxyFromDisk = mProxyName;
        if (!mPostLoad) {
            MemPoint pt(MemPoint::kInitType0);
            if (sObjectMemDumpFile || sTypeMemDumpFile) {
                pt = MemPoint(MemPoint::kInitType1);
            }
            mDir->PreLoad(*mStream);
            mPostLoad = true;
            if (sObjectMemDumpFile) {
                MemPoint start;
                DumpObjectMemDelta(mDir, start - pt);
            }
            if (sTypeMemDumpFile) {
                MemPoint start;
                AddTypeObjectMemDelta(mDir, start - pt);
            }
        }

        EofType t = TempEof;
        if (TheLoadMgr.GetFirstLoading() != this || (t = mStream->Eof(), t != NotEof)) {
            MILO_ASSERT(t == TempEof, 0x49F);
            gLoadingProxyFromDisk = oldproxy;
            return;
        }
        if (t == NotEof) {
            MemPoint pt(MemPoint::kInitType0);
            if (sObjectMemDumpFile || sTypeMemDumpFile) {
                pt = MemPoint(MemPoint::kInitType1);
            }
            mDir->PostLoad(*mStream);
            gLoadingProxyFromDisk = oldproxy;
            mPostLoad = false;
            if (sObjectMemDumpFile) {
                MemPoint start;
                DumpObjectMemDelta(mDir, start - pt);
            }
            if (sTypeMemDumpFile) {
                MemPoint start;
                AddTypeObjectMemDelta(mDir, start - pt);
            }
        }
    }
    ReadDead(*mStream);
    mState = &DirLoader::LoadObjs;
}

void DirLoader::LoadResources() {
    if (mCounter-- != 0) {
        FilePathTracker fpt(mRoot.c_str());
        FilePath fp2;
        *mStream >> fp2;
        if (!fp2.empty()) {
            TheLoadMgr.AddLoader(fp2, kLoadFront);
        }
    } else {
        if (mRev > 0xD)
            mState = &DirLoader::LoadDir;
        else
            mState = &DirLoader::LoadObjs;
    }
}

void DirLoader::CreateObjects() {
    while (mCounter-- != 0) {
        Symbol classSym;
        *mStream >> classSym;
        classSym = FixClassName(classSym);
        char buf[0x80];
        mStream->ReadString(buf, 0x80);
        bool b8;
        if (mRev > 0 && mRev < 8) {
            *mStream >> b8;
        }
        Hmx::Object *obj;
        if (!Hmx::Object::RegisteredFactory(classSym)) {
            MILO_NOTIFY("%s: Can't make %s", mFile.c_str(), classSym);
            obj = nullptr;
        } else {
            MemPoint pt(MemPoint::kInitType0);
            if (sObjectMemDumpFile || sTypeMemDumpFile) {
                pt = MemPoint(MemPoint::kInitType1);
            }
            BeginMemTrackObjectName(buf);
            obj = Hmx::Object::NewObject(classSym);
            EndMemTrackObjectName();
            if (mRev == 0x16 && dynamic_cast<class ObjectDir *>(obj)) {
                RELEASE(obj);
            } else {
                obj->SetName(buf, mDir);
                if (sObjectMemDumpFile) {
                    MemPoint start;
                    DumpObjectMemDelta(obj, start - pt);
                }
                if (sTypeMemDumpFile) {
                    MemPoint start;
                    AddTypeObjectMemDelta(obj, start - pt);
                }
            }
        }
        mObjects.push_back(obj);
        if (TheLoadMgr.CheckSplit())
            return;
    }
    if (mRev > 16) {
        mState = &DirLoader::LoadDir;
    } else {
        *mStream >> mCounter;
        mState = &DirLoader::LoadResources;
    }
}

void DirLoader::LoadHeader() {
    EofType t;
    while (t = mStream->Eof(), t != NotEof) {
        if (t != TempEof) {
            Cleanup(MakeString(
                "%s: Unexpected end of file. Proceeding as if file were empty.",
                mStream->Name()
            ));
            return;
        }
        if (TheLoadMgr.CheckSplit())
            return;
    }
    *mStream >> mRev;
    if (mRev < 7) {
        Cleanup(MakeString("Can't load old ObjectDir %s", LoaderFile()));
        return;
    }
    Symbol dirSym("RndDir");
    if (!Hmx::Object::RegisteredFactory(dirSym)) {
        dirSym = "ObjectDir";
    }
    if (mRev > 0xD) {
        Symbol symRead;
        *mStream >> symRead;
        symRead = FixClassName(symRead);
        char buf[0x80];
        mStream->ReadString(buf, 0x80);
        if (!Hmx::Object::RegisteredFactory(symRead)) {
            MILO_NOTIFY(
                "%s: %s not registered, defaulting to %s", mFile.c_str(), symRead, dirSym
            );
            symRead = dirSym;
            mLoadDir = false;
        }
        if (!SetupDir(symRead))
            return;
        int size1, size2;
        *mStream >> size1 >> size2;
        unk9a = false;
        if (mRev > 0x1C) {
            *mStream >> unk9a;
        }
        size1 += mDir->HashTableUsedSize() + 0x10;
        size2 += mDir->StrTableUsedSize() + 0x98;
        mDir->Reserve(size1, size2);
        mDir->SetName(buf, mDir);
    } else if (mRev > 0xC) {
        Symbol sa8;
        *mStream >> sa8;
        if (!SetupDir("ObjectDir"))
            return;
        mDir->SetName(FileGetBase(mFile.c_str()), mDir);
        mDir->ObjectDir::Load(*mStream);
    } else {
        if (!SetupDir(dirSym))
            return;
        mDir->SetName(FileGetBase(mFile.c_str()), mDir);
    }
    mDir->SetLoader(this);
    *mStream >> mCounter;
    if (mRev < 0xE) {
        mDir->Reserve(mCounter * 2, mCounter * 25);
    }
    mState = &DirLoader::CreateObjects;
}

void DirLoader::OpenFile() {
    mTimer.Start();
    if (!mStream) {
        Archive *theArchive = TheArchive;
        bool using_cd = UsingCD();
        bool cache_mode = sCacheMode;
        const char *fileStr = mFile.c_str();
        bool matches = gHostFile && FileMatch(fileStr, gHostFile);
        if (matches) {
            SetCacheMode(gHostCached);
            SetUsingCD(false);
            TheArchive = nullptr;
        }

        const char *path = CachedPath(fileStr, false);
        mStream = new ChunkStream(
            path, ChunkStream::kRead, 0x10000, true, kPlatformNone, false
        );
        mOwnStream = true;
        if (matches) {
            SetCacheMode(cache_mode);
            SetUsingCD(using_cd);
            TheArchive = theArchive;
        }
        if (mStream->Fail()) {
            if (mProxyDir) {
                Cleanup(MakeString("%s: could not load: %s", PathName(mProxyDir), path));
            } else {
                Cleanup(MakeString("Could not load: %s", path));
            }
            return;
        }
    }
    mState = &DirLoader::LoadHeader;
}
