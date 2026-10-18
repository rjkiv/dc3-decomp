#pragma once
#include "FileMerger.h"
#include "obj/Object.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

class FileMergerOrganizerLoader : public Loader {
public:
    FileMergerOrganizerLoader() : Loader("", kLoadFront) {}
    virtual ~FileMergerOrganizerLoader() {}
    virtual const char *DebugText() { return "FileMergerOrganizerLoader"; }
    virtual bool IsLoaded() const { return false; }
    virtual const char *StateName() const { return "FileMergerOrganizerLoader"; }

protected:
    virtual void PollLoading();
};

class FileMergerOrganizer : public Hmx::Object, public Loader::Callback {
public:
    enum OrganizedState {
        kStartLoad = 0,
        kFinishLoad = 1,
        kFailedLoad = 2,
        kPendingLoad = 3
    };
    struct OrganizedFileMerger {
        OrganizedFileMerger() : merger(nullptr) {}
        ObjPtr<FileMerger> merger; // 0x0
        OrganizedState state; // 0x14
    };

    // Hmx::Object
    OBJ_CLASSNAME(FileMergerOrganizer);
    OBJ_SET_TYPE(FileMergerOrganizer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // Loader::Callback
    virtual void FinishLoading(Loader *);
    virtual void FailedLoading(Loader *);

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(FileMergerOrganizer)
    static void Init();

    void StartLoad();
    void AddFileMerger(FileMerger *);

protected:
    FileMergerOrganizer();

    FileMerger::Merger *FrontInactiveMerger(OrganizedFileMerger *);

private:
    void Dispatch(OrganizedFileMerger *);
    void RemoveFileMerger(OrganizedFileMerger *);
    void CheckDone();

    std::list<OrganizedFileMerger> mFileMergers; // 0x30
    OrganizedFileMerger *mActiveOrg; // 0x38
    FileMergerOrganizerLoader *mStartOrg; // 0x3c
};

extern FileMergerOrganizer *TheFileMergerOrganizer;
