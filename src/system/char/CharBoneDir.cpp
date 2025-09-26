#include "char/CharBoneDir.h"
#include "char/CharBone.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"

ObjectDir *sResources;
DataArray *CharBoneDir::sCharClipTypes;

CharBoneDir::CharBoneDir()
    : mRecenter(this), mMoveContext(0), mBakeOutFacing(true), mFilterContext(0),
      mFilterBones(this) {}

CharBoneDir::~CharBoneDir() {}

BEGIN_HANDLERS(CharBoneDir)
    HANDLE_EXPR(get_context_flags, GetContextFlags())
    HANDLE_SUPERCLASS(ObjectDir)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharBoneDir::Recenter)
    SYNC_PROP(targets, o.mTargets)
    SYNC_PROP(average, o.mAverage)
    SYNC_PROP(slide, o.mSlide)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharBoneDir)
    SYNC_PROP(recenter, mRecenter)
    SYNC_PROP_SET(merge_character, "", MergeCharacter(FilePath(_val.Str())))
    SYNC_PROP(move_context, mMoveContext)
    SYNC_PROP(bake_out_facing, mBakeOutFacing)
    SYNC_PROP_MODIFY(filter_context, mFilterContext, SyncFilter())
    SYNC_PROP(filter_bones, mFilterBones)
    SYNC_PROP(filter_names, mFilterNames)
    SYNC_SUPERCLASS(ObjectDir)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, CharBoneDir::Recenter &r) {
    bs << r.mTargets;
    bs << r.mAverage;
    bs << r.mSlide;
    return bs;
}

BEGIN_SAVES(CharBoneDir)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(ObjectDir)
    bs << mMoveContext;
    bs << mRecenter;
    bs << mBakeOutFacing;
END_SAVES

BEGIN_COPYS(CharBoneDir)
    COPY_SUPERCLASS(ObjectDir)
    CREATE_COPY(CharBoneDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMoveContext)
        COPY_MEMBER(mRecenter)
        COPY_MEMBER(mBakeOutFacing)
    END_COPYING_MEMBERS
END_COPYS

BinStream &operator>>(BinStream &bs, CharBoneDir::Recenter &r) {
    bs >> r.mTargets;
    bs >> r.mAverage;
    bs >> r.mSlide;
    return bs;
}

void CharBoneDir::ListBones(std::list<CharBones::Bone> &bones, int mask, bool b3) {
    if (mMoveContext & mask) {
        bones.push_back(CharBones::Bone("bone_facing.pos", 1.0f));
        bones.push_back(CharBones::Bone("bone_facing.rotz", 1.0f));
        if (b3) {
            bones.push_back(CharBones::Bone("bone_facing_delta.pos", 1.0f));
            bones.push_back(CharBones::Bone("bone_facing_delta.rotz", 1.0f));
        }
    }
    for (ObjDirItr<CharBone> it(this, false); it != 0; ++it) {
        it->StuffBones(bones, mask);
    }
}

DataNode GetClipTypes(DataArray *a) { return CharBoneDir::GetClipTypes(); }

void CharBoneDir::Init() {
    FilePathTracker tracker(FileRoot());
    sResources = ObjectDir::Main()->New<ObjectDir>("char_resources");
    DataArray *cfg = SystemConfig("objects", "CharBoneDir");
    const char *path = "";
    cfg->FindData("resource_path", path, false);
    sCharClipTypes = SystemConfig("objects", "CharClip", "types");
    if (sCharClipTypes && *path != '\0') {
        for (int i = 1; i < sCharClipTypes->Size(); i++) {
            DataArray *foundarr = sCharClipTypes->Array(i)->FindArray("resource", false);
            if (foundarr) {
                Symbol foundsym = foundarr->Sym(1);
                ObjectDir *thedir = sResources->Find<ObjectDir>(foundsym.Str(), false);
                if (!thedir) {
                    const char *milostr = MakeString("%s/%s.milo", path, foundsym);
                    static int _x = MemFindHeap("char");
                    MemHeapTracker tmp(_x);
                    ObjectDir *loadedDir =
                        DirLoader::LoadObjects(milostr, nullptr, nullptr);
                    if (loadedDir) {
                        loadedDir->SetName(foundsym.Str(), sResources);
                    }
                }
            }
        }
    }
    DataRegisterFunc("get_clip_types", ::GetClipTypes);
}

DataNode CharBoneDir::GetClipTypes() {
    DataArray *arr = new DataArray(sCharClipTypes->Size());
    arr->Node(0) = Symbol();
    for (int i = 1; i < sCharClipTypes->Size(); i++) {
        DataArray *currArr = sCharClipTypes->Array(i);
        arr->Node(i) = currArr->Sym(0);
    }
    arr->SortNodes(0);
    DataNode ret(arr);
    arr->Release();
    return ret;
}

void CharBoneDir::Terminate() { delete sResources; }

DataNode CharBoneDir::GetContextFlags() {
    if (mContextFlags.Type() == kDataInt) {
        DataArray *cfg = SystemConfig("objects", "CharClip", "types");
        DataArray *arr = new DataArray(cfg->Size() - 1);
        int count = 0;
        Symbol name(Name());
        for (int i = 1; i < cfg->Size(); i++) {
            DataArray *resourceArr = cfg->Array(i)->FindArray("resource", false);
            if (resourceArr && resourceArr->Sym(1) == name) {
                const char *str = resourceArr->Str(2);
                int j;
                for (j = 0; j < count; j++) {
                    if (streq(str, arr->Str(j)))
                        break;
                }
                if (j == count) {
                    arr->Node(count++) = resourceArr->Str(2);
                }
            }
        }
        arr->Resize(count);
        arr->SortNodes(0);
        mContextFlags = arr;
        arr->Release();
    }
    return mContextFlags;
}

bool SyncSort(CharBone *bone1, CharBone *bone2) {
    return strcmp(bone1->Name(), bone2->Name()) < 0;
}

CharBoneDir *CharBoneDir::FindBoneDirResource(const char *name) {
    return sResources->Find<CharBoneDir>(name, false);
}

CharBoneDir *CharBoneDir::FindResourceFromClipType(Symbol cliptype) {
    DataArray *types = sCharClipTypes->FindArray(cliptype, false);
    if (!types) {
        MILO_NOTIFY("CharClip has no type %s", cliptype);
        return 0;
    } else {
        DataArray *resources = types->FindArray("resource", false);
        if (!resources) {
            MILO_NOTIFY("CharClip %s has no (resource ...) field", cliptype);
            return 0;
        } else {
            CharBoneDir *dir = FindBoneDirResource(resources->Str(1));
            if (!dir)
                MILO_NOTIFY("CharClip %s has no resource", cliptype);
            return dir;
        }
    }
}

void CharBoneDir::StuffBones(CharBones &bones, int i) {
    std::list<CharBones::Bone> blist;
    ListBones(blist, i, true);
    bones.AddBones(blist);
}

void CharBoneDir::StuffBones(CharBones &bones, Symbol sym) {
    DataArray *found = sCharClipTypes->FindArray(sym, false);
    if (!found)
        MILO_NOTIFY("CharClip has no type %s", sym);
    else {
        DataArray *resource = found->FindArray("resource", false);
        if (!resource)
            MILO_NOTIFY("CharClip %s has no (resource ...) field", sym);
        else {
            CharBoneDir *dir = FindBoneDirResource(resource->Str(1));
            if (!dir)
                MILO_NOTIFY("CharClip %s has no resource", sym);
            else {
                dir->StuffBones(bones, DataGetMacro(resource->Str(2))->Int(0));
            }
        }
    }
}

void CharBoneDir::SyncFilter() {
    mFilterBones.clear();
    for (ObjDirItr<CharBone> it(this, true); it != nullptr; ++it) {
        if (mFilterContext & it->PositionContext() || mFilterContext & it->ScaleContext()
            || (it->RotationType() != CharBones::TYPE_END
                && mFilterContext & it->RotationContext())) {
            mFilterBones.push_back(it);
        }
    }
    mFilterBones.sort(SyncSort);
    mFilterNames.clear();
    std::list<CharBones::Bone> bones;
    ListBones(bones, mFilterContext, true);
    for (std::list<CharBones::Bone>::iterator it = bones.begin(); it != bones.end();
         ++it) {
        mFilterNames.push_back(it->name);
    }
    mFilterNames.sort();
    for (std::list<String>::iterator it = mFilterNames.begin(); it != mFilterNames.end();
         ++it) {
        MILO_LOG("%s\n", *it);
    }
}
