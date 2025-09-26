#pragma once
#include "char/CharBone.h"
#include "char/CharBones.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"

/** "A CharBone container, acts as a resource file,
    storing skeleton and DOF for particular contexts" */
class CharBoneDir : public ObjectDir {
public:
    class Recenter {
    public:
        Recenter(Hmx::Object *o) : mTargets(o), mAverage(o), mSlide(0) {}

        /** "bones to recenter, ie, bone_pelvis" */
        ObjPtrList<CharBone> mTargets; // 0x0
        /** "bones to average to find the new center" */
        ObjPtrList<CharBone> mAverage; // 0x10
        /** "Slide the character over the course of the clip.  If false, just uses the
         * start of the clip" */
        bool mSlide; // 0x20
    };

    virtual ~CharBoneDir();
    OBJ_CLASSNAME(CharBoneDir);
    OBJ_SET_TYPE(CharBoneDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &bs) { ObjectDir::Load(bs); }
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(CharBoneDir)

    DataNode GetContextFlags();
    void ListBones(std::list<CharBones::Bone> &, int, bool);
    void StuffBones(CharBones &, int);

    static void Init();
    static void Terminate();
    static DataNode GetClipTypes();
    static CharBoneDir *FindBoneDirResource(const char *);
    static CharBoneDir *FindResourceFromClipType(Symbol);
    static void StuffBones(CharBones &, Symbol);

protected:
    CharBoneDir();

    static DataArray *sCharClipTypes;

    void MergeCharacter(const FilePath &);
    void SyncFilter();

    /** "Used to limit travel.  Moves [targets] bones so that the average of
        the [average] bones will be at (0,0,0) at the start of the clip.
        If slide is true evaluates the [average] bones and
        the start of the clip and end of the clip,
        and recenters [targets] smoothly between those." */
    Recenter mRecenter; // 0x9c
    /** "context in which character should move itself around via bone_facing.pos and
     * bone_facing.rotz bones" */
    int mMoveContext; // 0xc8
    /** "if false, won't bake out facing, will just bake out position" */
    bool mBakeOutFacing; // 0xcc
    DataNode mContextFlags; // 0xd0
    /** "Context to use for listing filter_bones" */
    int mFilterContext; // 0xd8
    /** "bones with context specified in filter_context" */
    ObjPtrList<CharBone> mFilterBones; // 0xdc
    /** "name of bone with context specified in filter_context" */
    std::list<String> mFilterNames; // 0xf0
};
