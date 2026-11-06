#pragma once
#include "char/CharClip.h"
#include "char/Character.h"
#include "char/Waypoint.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Graph.h"
#include <vector>
#include "utl/MemMgr.h"

/** "Analyzes character clips in a venue" */
class ClipCollide : public Hmx::Object {
public:
    struct Report { // From RB3 Decomp
        // total size: 0x160
        char name[32]; // offset 0x0, size 0x20
        char charPath[256]; // offset 0x20, size 0x100
        char clip[32]; // offset 0x120, size 0x20
        class Waypoint *waypoint; // offset 0x140, size 0x4
        class Symbol position; // offset 0x144, size 0x4
        class Vector3 pos; // offset 0x150, size 0x10
    };

    virtual ~ClipCollide();
    OBJ_CLASSNAME(ClipCollide);
    OBJ_SET_TYPE(ClipCollide);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x1D)
    NEW_OBJ(ClipCollide);
    REGISTER_OBJ(ClipCollide);

protected:
    virtual void SetTypeDef(DataArray *);

    ClipCollide();
    void SyncChar();
    void SyncWaypoint();
    void ClearReport();
    void SyncMode();
    /** "Run the animation" */
    void Demonstrate();
    bool ValidWaypoint(Waypoint *);
    bool ValidClip(CharClip *);
    /** "Test all the characters against all the waypoints in this venue" */
    void TestChars();
    /** "Test the current character against all the waypoints in the venue" */
    void TestWaypoints();
    /** "Test all clips for the current character and waypoint" */
    void TestClips();
    ObjectDir *Clips();
    /** "Run the collision" */
    void Collide();
    void AddReport(Vector3);
    void PickReport(const char *);
    DataNode OnVenueName(DataArray *);
    DataNode OnListReport(DataArray *);
    DataNode OnListClips(DataArray *);
    DataNode OnListWaypoints(DataArray *);

    std::vector<Report> mReports; // 0x2c
    RndGraph *mGraph; // 0x38
    /** "Pointer to the character to move around" */
    ObjPtr<Character> mChar; // 0x3c
    /** "Changes the character to one of the listed ones". The list is "", "" (lol) */
    String mCharPath; // 0x50
    /** "Waypoint to stand at" */
    ObjPtr<Waypoint> mWaypoint; // 0x58
    /** "Where in the waypoint to stand". Options are: front, back, left, right */
    Symbol mPosition; // 0x6c
    /** "Clip to test" */
    ObjPtr<CharClip> mClip; // 0x70
    /** "Pick a report entry to look at" */
    String mReportString; // 0x84
    /** "show red world lines" */
    bool mWorldLines; // 0x8c
    /** "move the camera when you pick a report" */
    bool mMoveCamera; // 0x8d
    /** "mode to run in". List is "" (again, lol) */
    Symbol mMode; // 0x90
};
