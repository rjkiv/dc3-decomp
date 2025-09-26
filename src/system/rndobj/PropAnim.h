#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/PropKeys.h"

/**
 * @brief: A property animator.
 * Original _objects description:
 * "Animate any properties on target object"
 */
class RndPropAnim : public RndAnimatable {
public:
    virtual ~RndPropAnim();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(PropAnim);
    OBJ_SET_TYPE(PropAnim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();
    virtual bool Loop() { return mLoop; }
    virtual void StartAnim();
    virtual void EndAnim() {}
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual void SetKey(float);

    void AdvanceFrame(float);
    void RemoveKeys();
    void SetKey(Hmx::Object *, DataArray *, float);
    void RemoveKey(Hmx::Object *, DataArray *, int);
    void RemoveRange(Hmx::Object *, DataArray *, float, float);
    bool ChangePropPath(Hmx::Object *, DataArray *, DataArray *);
    int GetNumKeys(Hmx::Object *, Symbol);

    /** Get the PropKeys from our collection that has the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @returns The PropKeys with the above target and prop, if it exists.
     */
    PropKeys *GetKeys(const Hmx::Object *obj, DataArray *prop);

    /** Add a new set of keys to our collection.
     * @param [in] obj The target object for the new keys.
     * @param [in] prop The property of the target object the new keys should animate.
     * @param [in] keysType The new keys' type.
     * @returns The newly created and added keys.
     */
    PropKeys *AddKeys(Hmx::Object *obj, DataArray *prop, PropKeys::AnimKeysType keysType);

    /** Remove the PropKeys that has the supplied target object and property.
     * @param [in] obj The target object for the keys.
     * @param [in] prop The property of the target object the keys should animate.
     * @returns True if the keys existed and have been removed, false otherwise.
     */
    bool RemoveKeys(Hmx::Object *obj, DataArray *prop);

    /** Determine whether or not we have a PropKeys with the supplied target object and
     * property.
     * @param [in] obj The target object for the keys.
     * @param [in] prop The property of the target object the keys should animate.
     * @returns True if such PropKeys exists in our collection, false if not.
     */
    bool HasKeys(Hmx::Object *obj, DataArray *prop);

    /** Get the PropKeys with the supplied target and prop, and add a new keyframe with
     * the supplied value/frame pair.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @param [in] frame The frame of the keyframe we want to modify.
     * @param [in] val The value of the keyframe we want to modify.
     * @param [in] unique If true, and a keyframe at the supplied frame exists, modify the
     * value. Else, create a new keyframe.
     * @returns An iterator of the PropKeys collection with the corresponding PropKeys.
     */
    void SetKeyVal(
        Hmx::Object *obj, DataArray *prop, float frame, const DataNode &val, bool unique
    );
    /** Get the animation keys type of the PropKeys with the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @returns The found PropKeys' animation type. If the PropKeys doesn't exist, return
     * kFloat.
     */
    PropKeys::AnimKeysType AnimKeysType(Hmx::Object *obj, DataArray *prop);
    /** Get the interpolation type of the PropKeys with the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @returns The found PropKeys' interpolation type. If the PropKeys doesn't exist,
     * return kStep.
     */
    PropKeys::Interpolation InterpType(Hmx::Object *obj, DataArray *prop);
    /** Set the interpolation type of the PropKeys with the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @param [in] interp The interpolation type to set.
     */
    void SetInterpType(Hmx::Object *obj, DataArray *prop, PropKeys::Interpolation interp);
    /** Get the handler of the PropKeys with the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @returns The found PropKeys' handler. If the PropKeys doesn't exist, return a null
     * Symbol.
     */
    Symbol InterpHandler(Hmx::Object *obj, DataArray *prop);
    /** Set the handler of the PropKeys with the supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @param [in] handler The handler to set.
     */
    void SetInterpHandler(Hmx::Object *obj, DataArray *prop, Symbol handler);
    /** Get the value at the given frame of the given keys.
     * @param [in] keys The PropKeys to search in.
     * @param [in] frame The frame to get the value from.
     * @param [out] var The DataNode that will store the corresponding value.
     * @returns The index of the PropKeys in which this frame resides.
     */
    int ValueFromFrame(PropKeys *keys, float frame, DataNode *var);
    /** Get the value at the given index of the given keys.
     * @param [in] keys The PropKeys to search in.
     * @param [in] index The index to get the value from.
     * @param [out] var The DataNode that will store the corresponding value.
     * @returns True if a value was successfully retrieved, false otherwise.
     */
    bool ValueFromIndex(PropKeys *keys, int index, DataNode *var);

    DataNode ForEachTarget(const DataArray *);
    DataNode ForAllKeyframes(const DataArray *);
    DataNode ForeachKeyframe(const DataArray *);
    DataNode ForeachFrame(const DataArray *);
    DataNode OnRemoveKeyframe(DataArray *);
    DataNode OnReplaceKeyframe(DataArray *);
    DataNode OnReplaceFrame(DataArray *);
    DataNode OnGetIndexFromFrame(const DataArray *);
    DataNode OnGetFrameFromIndex(const DataArray *);
    DataNode OnGetValueFromIndex(const DataArray *);
    DataNode OnGetValueFromFrame(const DataArray *);
    DataNode OnGetNumKeys(const DataArray *);

protected:
    RndPropAnim();

    virtual DataNode OnListFlowLabels(DataArray *);

    /** Get the position of the PropKeys collection containing a PropKeys with the
     * supplied target and prop.
     * @param [in] obj The target object the PropKeys must have.
     * @param [in] prop The property that the PropKeys must be animating.
     * @returns An iterator of the PropKeys collection with the corresponding PropKeys.
     */
    std::list<PropKeys *>::iterator FindKeys(Hmx::Object *obj, DataArray *prop);

    /** A collection of PropKeys. */
    std::list<PropKeys *> mPropKeys; // 0x10
    float mLastFrame; // 0x18
    bool mInSetFrame; // 0x1c
    /** "Do I self loop on SetFrame" */
    bool mLoop; // 0x1d
    // "fire flow labels in sync with the anim"
    Symbol mFireFlowLabel; // 0x20
    // "Scales all animation keyframe values by this #"
    float mIntensity; // 0x24
    // "the names of possible flow labels you can place on this timeline (i.e.
    // 'footstep')"
    std::list<String> mFlowLabels; // 0x28
};
