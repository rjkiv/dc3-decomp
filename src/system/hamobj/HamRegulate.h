#pragma once
#include "char/CharIKFoot.h"
#include "char/CharPollable.h"
#include "char/Character.h"
#include "char/Waypoint.h"
#include "rndobj/Highlight.h"
#include "utl/MemMgr.h"

/** "Class to do regulation on a HamCharacter.  Has two modes of operation" */
class HamRegulate : public RndHighlightable, public CharPollable {
public:
    enum RegulateMode {
        kModeNormal = 0,
        kModePos = 1
    };

    // Hmx::Object
    virtual ~HamRegulate();
    OBJ_CLASSNAME(HamRegulate);
    OBJ_SET_TYPE(HamRegulate);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight() {}
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x18)
    NEW_OBJ(HamRegulate)

    void RegulateWay(Waypoint *, float);
    void SetWaypoint(Waypoint *w) { mWay = w; }

protected:
    HamRegulate();

    virtual void SetName(const char *, ObjectDir *);

    void Regulate(Vector3 &, float &);

    Character *mMe; // 0x10
    ObjPtr<Waypoint> mWay; // 0x14
    RegulateMode mMode; // 0x28
    float mBeats; // 0x2c
    Vector3 mPos; // 0x30
    Vector3 mAccumulator; // 0x40
    int mLastFrozenFeet; // 0x50
    float mMaxSpeed; // 0x54
    ObjPtr<CharIKFoot> mLeftFoot; // 0x58
    ObjPtr<CharIKFoot> mRightFoot; // 0x6c
};
