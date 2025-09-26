#pragma once
#include "char/Waypoint.h"
#include "char/CharClip.h"
#include "char/CharClipGroup.h"
#include "char/CharDriver.h"
#include "obj/Data.h"
#include "rndobj/Overlay.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

class Character;

class CharacterTest : public RndOverlay::Callback {
    friend bool
    PropSync(CharacterTest &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);

public:
    CharacterTest(Character *);
    virtual ~CharacterTest();
    virtual float UpdateOverlay(RndOverlay *, float);
    virtual DataNode Handle(DataArray *, bool);

    void Save(BinStream &);

    ObjectDir *Clips() const { return mDriver ? mDriver->ClipDir() : nullptr; }
    bool MovingSelf() const;
    void SetMoveSelf(bool);
    void TeleportTo(Waypoint *);
    void SetDistMap(Symbol);
    void AddDefaults();
    void Walk();
    void Recenter();

protected:
    void Sync();
    DataNode OnGetFilteredClips(DataArray *);

    Character *mMe; // 0x4
    /** "The driver to animate" */
    ObjPtr<CharDriver> mDriver; // 0x8
    /** "Clip to play" */
    ObjPtr<CharClip> mClip1; // 0x1c
    /** "Clip to transition to, if any" */
    ObjPtr<CharClip> mClip2; // 0x30
    /** "If set, group to use as filter for clips" */
    ObjPtr<CharClipGroup> mFilterGroup; // 0x44
    /** "Teleport to this Waypoint" */
    ObjPtr<Waypoint> mTeleportTo; // 0x58
    ObjPtrList<Waypoint> mWalkPath; // 0x6c
    /** "Displays the transition distance map between clip1 and clip2, raw means the raw
     * graph, no nodes". Options are: none, nodes, raw */
    Symbol mShowDistMap; // 0x80
    /** "Which transition to use between clip1 and clip2" */
    int mTransition; // 0x84
    /** "Cycle through all the transitions" */
    bool mCycleTransition; // 0x88
    /** "Click on every beat transition" */
    bool mMetronome; // 0x89
    /** "Character does not travel, constantly zeros out position and facing" */
    bool mZeroTravel; // 0x8a
    /** "graphically displays the screensize and lod next to the character" */
    bool mShowScreenSize; // 0x8b
    bool mShowFootExtents; // 0x8c
    int unk90;
    int unk94;
    int unk98;
    RndOverlay *mOverlay; // 0x9c
};

bool PropSync(CharacterTest &, DataNode &, DataArray *, int, PropOp);
