#include "obj/Utl.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/Str.h"
#include <cstdio>

std::list<String> sFilePaths;
std::list<Symbol> sFiles;

const char *PathName(const Hmx::Object *o) {
    return !o ? "NULL Object" : ((Hmx::Object *)o)->FindPathName();
}

MergeFilter::SubdirAction
MergeFilter::DefaultSubdirAction(ObjectDir *dir, Subdirs subdirs) {
    switch (subdirs) {
    case kNoSubdirs:
        return kMergeKeep;
    case kAllSubdirs:
        return kMergeMerge;
    case kMoveAllSubdirs:
        return kMergeReplace;
    case kInlineSubdirs:
        if (dir->InlineSubDirType() == kInlineNever
            || dir->InlineSubDirType() == kInlineCachedShared)
            return kMergeKeep;
    case kMergeInlinedMoveSharedSubdirs:
        if (dir->InlineSubDirType() == kInlineNever
            || dir->InlineSubDirType() == kInlineCachedShared)
            return kMergeReplace;
    default:
        break;
    }
    return kMergeMerge;
}

bool RecurseSuperClassesSearch(Symbol classSym, Symbol searchClass) {
    DataArray *classCfg = SystemConfig("objects")->FindArray(classSym, false);
    if (!classCfg)
        return false;
    DataArray *parents = classCfg->FindArray("superclasses", false);
    if (parents) {
        for (int i = 1; i < parents->Size(); i++) {
            Symbol curParent = parents->Sym(i);
            if (searchClass == curParent)
                return true;
            if (RecurseSuperClassesSearch(curParent, searchClass))
                return true;
        }
    }
    return false;
}

bool IsASubclass(Symbol child, Symbol parent) {
    if ((parent == "Object") || (child == parent))
        return true;
    else
        return RecurseSuperClassesSearch(child, parent);
}

const char *ClassExt(Symbol s) {
    static Symbol objects("objects");
    DataArray *cfg = SystemConfig(objects, s);
    DataArray *arr = cfg->FindArray("ext", false);
    return arr ? arr->Str(1) : nullptr;
}

void AddClassExt(char *file, Symbol s) {
    const char *ext = FileGetExt(file);
    if (*ext == '\0') {
        ext = ClassExt(s);
        if (ext) {
            strcat(file, ".");
            strcat(file, ext);
        }
    }
}

void MergeObject(
    Hmx::Object *o1, Hmx::Object *o2, ObjectDir *dir, MergeFilter::Action act
) {
    if (o1 == o2 || act == 3)
        return;
    if (o2) {
        o1->ReplaceRefs(o2);
        if (act == 0)
            o2->Copy(o1, Hmx::Object::kCopyFromMax);
        else if (act == 1)
            o2->Copy(o1, Hmx::Object::kCopyDeep);
    } else if (act != 2) {
        o1->SetName(o1->Name(), dir);
    }
}

int SubDirStringUsed(ObjectDir *dir) {
    if (!dir)
        return 0;
    else {
        int size = dir->StrTableUsedSize();
        const std::vector<ObjDirPtr<ObjectDir> > &subdirs = dir->SubDirs();
        for (std::vector<ObjDirPtr<ObjectDir> >::const_iterator it = subdirs.begin();
             it != subdirs.end();
             ++it) {
            size += SubDirStringUsed(*it);
        }
        return size;
    }
}

const char *NextName(const char *old_name, ObjectDir *dir) {
    if (!dir->FindObject(old_name, false, true))
        return old_name;
    const char *base = FileGetBase(old_name);
    const char *ext = FileGetExt(old_name);
    char *ptr;
    for (ptr = (char *)base + strlen(base); (base < ptr && (ptr[-1] - 0x30U <= 9)); ptr--)
        ;
    int atoied = 0;
    if (*ptr != '\0')
        atoied = atoi(ptr);
    char buf[128];
    do {
        atoied++;
        sprintf(ptr, "%02d", atoied);
        if (*ext != '\0') {
            sprintf(buf, "%s.%s", ptr, ext);
        } else {
            strcpy(buf, ptr);
        }
    } while (dir->FindObject(buf, false, true));

    return MakeString(buf);
}

bool PathCompare(DataArray *arr1, DataArray *arr2) {
    if (arr1 == arr2)
        return true;
    if (!arr1 || !arr2)
        return false;
    int arr1size = arr1->Size();
    if (arr1size != arr2->Size())
        return false;
    for (int i = 0; i < arr1size; i++) {
        DataType arr1type = CONST_ARRAY(arr1)->Node(i).Type();
        if (arr1type != CONST_ARRAY(arr2)->Node(i).Type())
            return false;
        else
            switch (arr1type) {
            case kDataSymbol:
            case kDataString:
                if (arr1->Str(i) != arr2->Str(i))
                    return false;
                break;
            case kDataInt:
                if (arr1->Int(i) != arr2->Int(i))
                    return false;
                break;
            default:
                break;
            }
    }
    return true;
}

const char *PrintPropertyPath(DataArray *arr) {
    StackString<256> str;
    str << arr;
    str[0U] = '[';
    str[str.length() - 1] = ']';
    return MakeString(str.c_str());
}

int SubDirHashUsed(ObjectDir *dir) {
    if (!dir)
        return 0;
    else {
        int size = dir->HashTableUsedSize();
        const std::vector<ObjDirPtr<ObjectDir> > &subdirs = dir->SubDirs();
        for (std::vector<ObjDirPtr<ObjectDir> >::const_iterator it = subdirs.begin();
             it != subdirs.end();
             ++it) {
            size += SubDirHashUsed(*it);
        }
        return size;
    }
}

int GetPropSize(Hmx::Object *o, DataArray *arr, int size) {
    DataArrayPtr ptr(new DataArray(size));
    for (int x = 0; x < size; x++) {
        ptr->Node(x) = arr->Node(x);
    }
    int ret = o->PropertySize(ptr);
    return ret;
}

bool IsPropPathValid(Hmx::Object *o, DataArray *prop) {
    for (int i = 0; i < prop->Size(); i++) {
        if (prop->Type(i) == kDataInt) {
            if (prop->Int(i) + 1 > GetPropSize(o, prop, i))
                return false;
        }
    }
    return true;
}

const DataNode *GetPropertyVal(Hmx::Object *o, DataArray *prop, bool fail) {
    return !IsPropPathValid(o, prop) ? nullptr : o->Property(prop, fail);
}

void ReserveToFit(ObjectDir *src, ObjectDir *dst, int extraObjects) {
    int stringSize = dst->StrTableUsedSize() + SubDirStringUsed(src) + extraObjects * 10;
    int hashSize = (dst->HashTableUsedSize() + SubDirHashUsed(src) + extraObjects) * 2;
    dst->Reserve(hashSize, stringSize);
}

void WalkProps(DataArray *ed, std::list<Symbol> &props, std::list<Symbol> *arrayProps) {
    for (int i = 1; i < ed->Size(); i++) {
        DataArray *arr = ed->Array(i);
        if ((arr->Type(1) == kDataSymbol) && arr->Sym(1) != "script")
            props.push_back(arr->Sym(0));
        else if (arr->Type(1) == kDataArray) {
            if (arr->Array(1)->Sym(0) == "indent") {
                WalkProps(arr->Array(1), props, arrayProps);
            } else if (arrayProps && arr->Array(1)->Sym(0) == "array") {
                arrayProps->push_back(arr->Sym(0));
            }
        }
    }
}

void ReloadObjectType(Hmx::Object *obj, DataArray *arr) {
    if (obj) {
        DataArray *def = obj->TypeDef();
        if (def) {
            std::list<DataArray *> arrs;
            if (!arr) {
                DataArray *file = DataReadFile(SystemConfig()->File(), true);
                arr = file->FindArray("objects");
            }
            arrs.push_back(arr);
            static Symbol types("types");
            DataArray *objArr = arr->FindArray(obj->ClassName(), types, obj->Type());
            DataUpdateArray(def, objArr);
            obj->SetTypeDef(def);
            if (arr)
                arr->Release();
        }
    }
}

DataNode ObjectList(ObjectDir *dir, Symbol parentSym, bool b) {
    std::list<const char *> sList;
    if (dir) {
        for (ObjDirItr<Hmx::Object> it(dir, true); it != 0; ++it) {
            if (IsASubclass(it->ClassName(), parentSym)) {
                sList.push_back(it->Name());
            }
        }
    }
    DataArrayPtr ptr(new DataArray(b + sList.size()));
    int idx = 0;
    if (b)
        ptr->Node(idx++) = "";
    for (std::list<const char *>::iterator it = sList.begin(); it != sList.end(); ++it) {
        ptr->Node(idx++) = *it;
    }
    ptr->SortNodes(0);
    return ptr;
}

void RecurseSuperClasses(Symbol classSym, std::vector<Symbol> &classes) {
    DataArray *cfg = SystemConfig("objects", classSym);
    DataArray *found = cfg->FindArray("superclasses", false);
    if (found) {
        for (int i = 1; i < found->Size(); i++) {
            Symbol foundSym = found->Sym(i);
            if (classes.end() == std::find(classes.begin(), classes.end(), foundSym)) {
                classes.push_back(foundSym);
            }
            RecurseSuperClasses(foundSym, classes);
        }
    }
}

void ListSuperClasses(Symbol classSym, std::vector<Symbol> &classes) {
    RecurseSuperClasses(classSym, classes);
    classes.push_back("Object");
}

void MergeDirs(ObjectDir *fromDir, ObjectDir *toDir, MergeFilter &filt) {
    MILO_ASSERT(fromDir && toDir, 0x193);
    Hmx::Object *toObj = toDir;
    Hmx::Object *fromObj = fromDir;
    if (toObj != fromObj) {
        MergeObject(fromObj, toObj, toDir, filt.Filter(fromObj, toObj, toDir));
    }
    CopyTypeProperties(fromDir, toDir);
    MergeObjectsRecurse(fromDir, toDir, filt, true);
}

Hmx::Object *CopyObject(
    Hmx::Object *from, Hmx::Object *to, Hmx::Object::CopyType ty, bool setProxyFile
) {
    to->Copy(from, ty);
    if (setProxyFile) {
        ObjectDir *dir2 = dynamic_cast<ObjectDir *>(to);
        if (dir2 && dir2->InlineProxyType() != kInlineAlways) {
            ObjectDir *dir1 = dynamic_cast<ObjectDir *>(from);
            dir2->SetProxyFile(dir1->ProxyFile(), false);
        }
    } else
        CopyTypeProperties(from, to);
    return to;
}

Hmx::Object *CloneObject(Hmx::Object *from, bool instance) {
    MILO_ASSERT(from, 0x1EE);
    return CopyObject(
        from,
        Hmx::Object::NewObject(from->ClassName()),
        instance ? Hmx::Object::kCopyShallow : Hmx::Object::kCopyDeep,
        true
    );
}

void ReplaceObject(
    Hmx::Object *from, Hmx::Object *to, bool copyDeep, bool deleteFrom, bool setProxyFile
) {
    const char *name = from->Name();
    ObjectDir *dir = from->Dir();
    from->SetName(nullptr, nullptr);
    if (to) {
        to->SetName(name, dir);
        if (copyDeep)
            CopyObject(from, to, Hmx::Object::kCopyDeep, setProxyFile);
    }
    from->ReplaceRefs(to);
    if (deleteFrom)
        delete from;
}
