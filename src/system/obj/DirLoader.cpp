#include "obj/DirLoader.h"
#include "Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Platform.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemPoint.h"
#include "utl/TextFileStream.h"
#include "utl/TextStream.h"
#include <map>

bool DirLoader::sPrintTimes;
bool DirLoader::sCacheMode;
ObjectDir *DirLoader::sTopSaveDir;
TextFileStream *DirLoader::sObjectMemDumpFile;
TextFileStream *DirLoader::sTypeMemDumpFile;
std::map<String, MemPointDelta> DirLoader::sMemPointMap;

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

bool DirLoader::IsLoaded() const { return mState == &DirLoader::DoneLoading; }

bool DirLoader::ShouldBlockSubdirLoad(const FilePath &fp) {
    return fp.c_str() && sPathEval ? sPathEval(fp.c_str()) : false;
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
                if (orig == CharTransDraw)
                    orig = P9TransDraw;
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
            if ("%#@EndOfEditorDir@#%"[i] == buf)
                break;
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
        const std::list<Loader *> &ldrs = TheLoadMgr.Loaders();
        for (std::list<Loader *>::const_iterator it = ldrs.begin(); it != ldrs.end();
             ++it) {
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

bool DirLoader::Replace(ObjRef *ref, Hmx::Object *obj) {
    //   if (param_1 == this + 0x84) {
    //     if (*(this + 0x90) != 0) {
    //       *(*(this + 0x8c) + 4) = *(this + 0x88);
    //       *(*(this + 0x88) + 8) = *(this + 0x8c);
    //     }
    //     pDVar2 = this + -0x1c;
    //     *(this + 0x90) = 0;
    //     *(this + 0x44) = 0;
    //     if (pDVar2 != 0x0) {
    //       (***pDVar2)(pDVar2,1,param_2);
    //     }
    //     bVar1 = true;
    //   }
    //   else {
    //     bVar1 = false;
    //   }
    //   return bVar1;
    return false;
}

DirLoader *DirLoader::FindLast(const FilePath &fp) {
    if (!fp.empty()) {
        const std::list<Loader *> &ldrs = TheLoadMgr.Loaders();
        for (std::list<Loader *>::const_reverse_iterator it = ldrs.rbegin();
             it != ldrs.rend();
             ++it) {
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
    for (std::map<String, MemPointDelta>::iterator it = sMemPointMap.begin();
         it != sMemPointMap.end();
         ++it) {
        MemPointDelta pt = it->second;
        file->Print(it->first.c_str());
        *file << "," << pt.ToString(1) << "\n";
    }
    file->mFile.Flush();
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
            //   AutoGlitchReport::AutoGlitchReport(aAStack_1090,50.0,p_Var6,SyncObjectsGlitchCB);
            mDir->SetSubDirFlag(unk9b);
            mDir->SyncObjects();
            mDir->SetSubDirFlag(false);
            //   AutoGlitchReport::~AutoGlitchReport(aAStack_1090);
        }
    }
    mState = &DirLoader::DoneLoading;
    mTimer.Stop();
    if (sPrintTimes) {
        MILO_LOG("%s: %f ms\n", mFile, mTimer.Ms());
    }
}

void DirLoader::AddTypeObjectMemDelta(
    const Hmx::Object *object, const MemPointDelta &memDelta
) const {
    MILO_ASSERT(object, 0x584);
    MILO_ASSERT(sTypeMemDumpFile, 0x585);
    if (memDelta.AnyGreaterThan(0)) {
        const char *name = object->ClassName().Str();
        if (!name || !*name)
            name = "Unknown";
        std::map<String, MemPointDelta>::iterator it = sMemPointMap.find(name);
        if (it == sMemPointMap.end()) {
            sMemPointMap[name] = MemPointDelta();
        }
    }
}

DirLoader::~DirLoader() {
    mDeleteSelf = 0;
    if (!IsLoaded()) {
        Cleanup(nullptr);
    } else if (mDir) {
        mDir->SetLoader(nullptr);
        if (!unk98 && !mProxyName) {
            RELEASE(mDir);
        }
    }
    if (mProxyDir) {
        mProxyDir->Release(nullptr);
        // mProxyDir = nullptr;
    }
    if (mCallback && unk99) {
        mCallback->FailedLoading(this);
        mCallback = 0;
    }
}

const char *DirLoader::DebugText() { return MakeString("DL: %s", mFile.c_str()); }

void DirLoader::SaveObjects(BinStream &bs, ObjectDir *dir) {
    char buf[256];
    MILO_ASSERT(sTopSaveDir != dir, 0x10C);
    if (!sTopSaveDir)
        sTopSaveDir = dir;
    ObjectDir *dirdir = dir->Dir();
    strcpy(buf, dir->Name());
    if (dirdir != dir) {
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
        for (std::list<Hmx::Object *>::const_iterator it = objects.begin();
             it != objects.end();
             it++) {
            (*it)->PostSave(bs);
        }
    }
    if (dirdir != dir) {
        dir->SetName(buf, dir);
    }
    if (sTopSaveDir == dir) {
        sTopSaveDir = nullptr;
    }
}

bool DirLoader::SaveObjects(const char *cc, ObjectDir *dir, bool b3) {
    if (sCacheMode && dir->InlineSubDirType() != kInlineNever) {
        MILO_LOG("Not caching %s because it is an inlined subdir.\n", cc);
        return false;
    } else {
        FilePathTracker tracker(FileGetPath(cc));
        cc = CachedPath(cc, false);
        if (sCacheMode) {
            FileMkDir(FileGetPath(cc));
        }
        Platform p = sCacheMode ? TheLoadMgr.GetPlatform() : kPlatformPC;
        MILO_ASSERT(p != kPlatformNone, 0x1B3);
        ChunkStream cs(
            cc,
            ChunkStream::kWrite,
            p == kPlatformWii ? 0x10000 : 0x20000,
            gNullFiles,
            p,
            sCacheMode
        );
        if (cs.Fail()) {
            MILO_NOTIFY("Could not open file: %s", cc);
            return false;
        } else {
            SaveObjects(cs, dir);
            return true;
        }
    }
}

DirLoader::DirLoader(
    const FilePath &fp,
    LoaderPos pos,
    Loader::Callback *cb,
    BinStream *stream,
    class ObjectDir *dir,
    bool bbb,
    class ObjectDir *dir2
)
    : Loader(fp, pos), mOwnStream(false), mStream(stream), mRev(0), mCounter(0),
      mObjects(nullptr, kObjListAllowNull), mCallback(cb), mDir(dir), mPostLoad(false),
      mLoadDir(false), mDeleteSelf(false), mProxyName(nullptr), unk98(0), unk99(0),
      unk9a(0), unk9b(bbb), unk9c(dir2), mProxyDir(this, nullptr) {
    if (mDir) {
        mDeleteSelf = true;
        mProxyName = mDir->Name();
        mProxyDir.SetObjConcrete(mDir);
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
        const char *filePath = FileGetPath(mFile.c_str());
        char buf[256];
        strcpy(buf, filePath);
        if (strlen(buf) - 4 > 0) {
            // strcpy(&buf[strlen(buf)], "/gen");
            strcat(buf, "/gen");
        }
        mRoot = FileMakePath(FileRoot(), buf);
    }
    mState = &DirLoader::OpenFile;
}

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
