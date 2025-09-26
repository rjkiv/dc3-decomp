#include "rndobj/PropAnim.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/PropKeys.h"
#include "utl/Std.h"

DataNode sKeyReplace;
bool sRemoveFrame;
bool sReplaceKey;
bool sReplaceFrame;
float sFrameReplace;

RndPropAnim::RndPropAnim()
    : mLastFrame(0), mInSetFrame(false), mLoop(false), mFireFlowLabel(""), mIntensity(1) {
}

RndPropAnim::~RndPropAnim() { DeleteAll(mPropKeys); }

bool RndPropAnim::Replace(ObjRef *ref, Hmx::Object *obj) {
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        if (obj) {
            (*it)->SetTarget(obj);
        } else {
            mPropKeys.erase(it);
            delete *it;
            return true;
        }
    }
    return Hmx::Object::Replace(ref, obj);
}

BEGIN_HANDLERS(RndPropAnim)
    HANDLE_EXPR(remove_keys, RemoveKeys(_msg->Obj<Hmx::Object>(2), _msg->Array(3)))
    HANDLE_EXPR(has_keys, HasKeys(_msg->Obj<Hmx::Object>(2), _msg->Array(3)))
    HANDLE_ACTION(
        add_keys,
        AddKeys(
            _msg->Obj<Hmx::Object>(2), _msg->Array(3), (PropKeys::AnimKeysType)_msg->Int(4)
        )
    )
    HANDLE_ACTION(
        set_key, SetKey(_msg->Obj<Hmx::Object>(2), _msg->Array(3), _msg->Float(4))
    )
    HANDLE_ACTION(
        set_key_val,
        SetKeyVal(
            _msg->Obj<Hmx::Object>(2),
            _msg->Array(3),
            _msg->Float(4),
            _msg->Node(5),
            _msg->Size() > 6 ? _msg->Int(6) : true
        )
    )
    HANDLE_ACTION(
        remove_index, RemoveKey(_msg->Obj<Hmx::Object>(2), _msg->Array(3), _msg->Int(4))
    )
    HANDLE_ACTION(
        remove_range,
        RemoveRange(
            _msg->Obj<Hmx::Object>(2), _msg->Array(3), _msg->Float(4), _msg->Float(5)
        )
    )
    HANDLE_EXPR(keys_type, AnimKeysType(_msg->Obj<Hmx::Object>(2), _msg->Array(3)))
    HANDLE_EXPR(interp_type, InterpType(_msg->Obj<Hmx::Object>(2), _msg->Array(3)))
    HANDLE_ACTION(
        set_interp_type,
        SetInterpType(
            _msg->Obj<Hmx::Object>(2),
            _msg->Array(3),
            (PropKeys::Interpolation)_msg->Int(4)
        )
    )
    HANDLE_EXPR(interp_handler, InterpHandler(_msg->Obj<Hmx::Object>(2), _msg->Array(3)))
    HANDLE_ACTION(
        set_interp_handler,
        SetInterpHandler(_msg->Obj<Hmx::Object>(2), _msg->Array(3), _msg->Sym(4))
    )
    HANDLE_ACTION(
        replace_target,
        _msg->Obj<Hmx::Object>(2)->ReplaceRefsFrom(this, _msg->Obj<Hmx::Object>(3))
    )
    HANDLE_EXPR(foreach_target, ForEachTarget(_msg))
    HANDLE(forall_keyframes, ForAllKeyframes)
    HANDLE(foreach_keyframe, ForeachKeyframe)
    HANDLE(foreach_frame, ForeachFrame)
    HANDLE_EXPR(
        change_prop_path,
        ChangePropPath(_msg->Obj<Hmx::Object>(2), _msg->Array(3), _msg->Array(4))
    )
    HANDLE(remove_keyframe, OnRemoveKeyframe)
    HANDLE(replace_keyframe, OnReplaceKeyframe)
    HANDLE(replace_frame, OnReplaceFrame)
    HANDLE(index_from_frame, OnGetIndexFromFrame)
    HANDLE(frame_from_index, OnGetFrameFromIndex)
    HANDLE(value_from_index, OnGetValueFromIndex)
    HANDLE(value_from_frame, OnGetValueFromFrame)
    HANDLE(num_keys, OnGetNumKeys)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndPropAnim)
    SYNC_PROP(loop, mLoop)
    SYNC_PROP_MODIFY(fire_flow_label, mFireFlowLabel, FireFlowLabel(mFireFlowLabel))
    SYNC_PROP(flow_labels, mFlowLabels)
    SYNC_PROP(intensity, mIntensity)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndPropAnim)
    SAVE_REVS(15, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mPropKeys.size();
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        bs << (*it)->KeysType();
        (*it)->Save(bs);
    }
    bs << mLoop;
    bs << mFlowLabels;
    bs << mIntensity;
END_SAVES

BEGIN_COPYS(RndPropAnim)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    mLastFrame = GetFrame();
    RemoveKeys();
    CREATE_COPY(RndPropAnim)
    BEGIN_COPYING_MEMBERS
        for (std::list<PropKeys *>::const_iterator it = c->mPropKeys.begin();
             it != c->mPropKeys.end();
             ++it) {
            PropKeys *cur = *it;
            AddKeys(cur->Target(), cur->Prop(), cur->KeysType())->Copy(*it);
        }
        COPY_MEMBER(mLoop)
        COPY_MEMBER(mFlowLabels)
        COPY_MEMBER(mIntensity)
    END_COPYING_MEMBERS
END_COPYS

void RndPropAnim::RemoveKeys() { DeleteAll(mPropKeys); }

void RndPropAnim::AdvanceFrame(float frame) {
    if (mLoop) {
        frame = ModRange(StartFrame(), EndFrame(), frame);
    }
    RndAnimatable::SetFrame(frame, 1.0f);
}

float RndPropAnim::StartFrame() {
    float frame = 0.0f;
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        frame = Min(frame, (*it)->StartFrame());
    }
    return frame;
}

float RndPropAnim::EndFrame() {
    float frame = 0.0f;
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        frame = Max(frame, (*it)->EndFrame());
    }
    return frame;
}

void RndPropAnim::SetKey(float frame) {
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        (*it)->SetKey(frame);
    }
}

void RndPropAnim::StartAnim() {
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        (*it)->ResetLastKeyFrameIndex();
    }
}

void RndPropAnim::Print() {
    int idx = 0;
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        TheDebug << "   Keys " << idx << "\n";
        (*it)->Print();
        idx++;
    }
}

DataNode RndPropAnim::OnListFlowLabels(DataArray *arr) {
    if (mFlowLabels.size() != 0) {
        DataArray *flowArr = new DataArray(mFlowLabels.size());
        int i = 0;
        for (std::list<String>::iterator it = mFlowLabels.begin();
             it != mFlowLabels.end();
             ++it, ++i) {
            flowArr->Node(i) = Symbol(it->c_str());
        }
        DataNode ret = flowArr;
        flowArr->Release();
        return ret;
    } else {
        DataArray *flowArr = new DataArray(1);
        flowArr->Node(0) = Symbol();
        DataNode ret = flowArr;
        flowArr->Release();
        return ret;
    }
}

PropKeys *
RndPropAnim::AddKeys(Hmx::Object *obj, DataArray *prop, PropKeys::AnimKeysType ty) {
    PropKeys *theKeys = GetKeys(obj, prop);
    if (theKeys)
        return theKeys;
    else {
        switch (ty) {
        case PropKeys::kFloat:
            theKeys = new FloatKeys(this, obj);
            break;
        case PropKeys::kColor:
            theKeys = new ColorKeys(this, obj);
            break;
        case PropKeys::kObject:
            theKeys = new ObjectKeys(this, obj);
            break;
        case PropKeys::kBool:
            theKeys = new BoolKeys(this, obj);
            break;
        case PropKeys::kQuat:
            theKeys = new QuatKeys(this, obj);
            break;
        case PropKeys::kVector3:
            theKeys = new Vector3Keys(this, obj);
            break;
        case PropKeys::kSymbol:
            theKeys = new SymbolKeys(this, obj);
            break;
        default:
            MILO_NOTIFY("Unable to create animation for keysType");
            return nullptr;
        }
        if (prop) {
            DataNode node(prop, kDataArray);
            theKeys->SetProp(node);
        }
        mPropKeys.push_back(theKeys);
    }
    return theKeys;
}

std::list<PropKeys *>::iterator RndPropAnim::FindKeys(Hmx::Object *o, DataArray *da) {
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        PropKeys *cur = *it;
        if (!da && !cur->Prop())
            return it;
        if (cur->Target() == o && PathCompare(da, cur->Prop()))
            return it;
    }
    return mPropKeys.end();
}

PropKeys *RndPropAnim::GetKeys(const Hmx::Object *obj, DataArray *prop) {
    if (!prop || !obj)
        return 0;
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        PropKeys *cur = *it;
        if (cur->Target() == obj && PathCompare(prop, cur->Prop()))
            return cur;
    }
    return nullptr;
}

bool RndPropAnim::HasKeys(Hmx::Object *o, DataArray *da) {
    return FindKeys(o, da) != mPropKeys.end();
}

void RndPropAnim::SetKey(Hmx::Object *o, DataArray *da, float f) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        cur->SetKey(f);
    }
}

bool RndPropAnim::RemoveKeys(Hmx::Object *o, DataArray *da) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys == mPropKeys.end())
        return false;
    else {
        PropKeys *cur = *keys;
        mPropKeys.erase(keys);
        delete cur;
        return true;
    }
}

void RndPropAnim::RemoveKey(Hmx::Object *o, DataArray *da, int i3) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        (*keys)->RemoveKey(i3);
    }
}

void RndPropAnim::RemoveRange(Hmx::Object *o, DataArray *da, float f3, float f4) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        (*keys)->RemoveRange(f3, f4);
    }
}

PropKeys::AnimKeysType RndPropAnim::AnimKeysType(Hmx::Object *o, DataArray *da) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        return (PropKeys::AnimKeysType)cur->KeysType();
    } else
        return PropKeys::kFloat;
}

PropKeys::Interpolation RndPropAnim::InterpType(Hmx::Object *o, DataArray *da) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        return (PropKeys::Interpolation)cur->GetInterpolation();
    } else
        return PropKeys::kStep;
}

void RndPropAnim::SetInterpType(
    Hmx::Object *obj, DataArray *prop, PropKeys::Interpolation iter
) {
    std::list<PropKeys *>::iterator keys = FindKeys(obj, prop);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        cur->SetInterpolation(iter);
    }
}

Symbol RndPropAnim::InterpHandler(Hmx::Object *obj, DataArray *prop) {
    std::list<PropKeys *>::iterator keys = FindKeys(obj, prop);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        if (cur->GetExceptionID() == PropKeys::kHandleInterp)
            return cur->InterpHandler();
    }
    return Symbol();
}

void RndPropAnim::SetInterpHandler(Hmx::Object *obj, DataArray *prop, Symbol handler) {
    std::list<PropKeys *>::iterator keys = FindKeys(obj, prop);
    if (keys != mPropKeys.end()) {
        (*keys)->SetInterpHandler(handler);
    }
}

int RndPropAnim::GetNumKeys(Hmx::Object *obj, Symbol sym) {
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        PropKeys *cur = *it;
        if (cur->Target() == obj && cur->Prop()->Node(0).Sym(nullptr) == sym) {
            return cur->NumKeys();
        }
    }
    return 0;
}

int RndPropAnim::ValueFromFrame(PropKeys *keys, float frame, DataNode *node) {
    int ret = -1;
    if (!keys)
        return -1;
    else {
        switch (keys->KeysType()) {
        case PropKeys::kFloat: {
            float fval = 0.0f;
            ret = keys->FloatAt(frame, fval);
            *node = fval;
            break;
        }
        case PropKeys::kColor: {
            Hmx::Color col;
            ret = keys->ColorAt(frame, col);
            *node = col.Pack();
            break;
        }
        case PropKeys::kObject: {
            Hmx::Object *obj = 0;
            ret = keys->ObjectAt(frame, obj);
            *node = obj;
            break;
        }
        case PropKeys::kBool: {
            bool bval = false;
            ret = keys->BoolAt(frame, bval);
            *node = bval;
            break;
        }
        case PropKeys::kQuat: {
            Hmx::Quat quatval;
            ret = keys->QuatAt(frame, quatval);
            *node = DataArrayPtr(quatval.x, quatval.y, quatval.z, quatval.w);
            break;
        }
        case PropKeys::kVector3: {
            Vector3 vecval;
            ret = keys->Vector3At(frame, vecval);
            *node = DataArrayPtr(vecval.x, vecval.y, vecval.z);
            break;
        }
        case PropKeys::kSymbol: {
            Symbol symval;
            ret = keys->SymbolAt(frame, symval);
            *node = symval;
            break;
        }
        default: {
            *node = 0;
            break;
        }
        }
        return ret;
    }
}

bool RndPropAnim::ValueFromIndex(PropKeys *keys, int index, DataNode *node) {
    if (index < 0 || index >= keys->NumKeys())
        return false;
    switch (keys->KeysType()) {
    case PropKeys::kFloat: {
        *node = (*keys->AsFloatKeys())[index].value;
        break;
    }
    case PropKeys::kColor: {
        *node = (*keys->AsColorKeys())[index].value.Pack();
        break;
    }
    case PropKeys::kObject: {
        *node = (*keys->AsObjectKeys())[index].value.Ptr();
        break;
    }
    case PropKeys::kBool: {
        *node = (*keys->AsBoolKeys())[index].value;
        break;
    }
    case PropKeys::kQuat: {
        Hmx::Quat q((*keys->AsQuatKeys())[index].value);
        *node = DataArrayPtr(q.x, q.y, q.z, q.w);
        break;
    }
    case PropKeys::kVector3: {
        Vector3 vec((*keys->AsVector3Keys())[index].value);
        *node = DataArrayPtr(vec.x, vec.y, vec.z);
        break;
    }
    case PropKeys::kSymbol: {
        *node = (*keys->AsSymbolKeys())[index].value;
        break;
    }
    default: {
        *node = 0;
        break;
    }
    }
    return true;
}

void RndPropAnim::SetKeyVal(
    Hmx::Object *o, DataArray *da, float frame, const DataNode &node, bool unique
) {
    std::list<PropKeys *>::iterator keys = FindKeys(o, da);
    if (keys != mPropKeys.end()) {
        PropKeys *cur = *keys;
        switch (cur->KeysType()) {
        case PropKeys::kFloat:
            cur->AsFloatKeys()->Add(node.Float(), frame, unique);
            break;
        case PropKeys::kColor:
            cur->AsColorKeys()->Add(Hmx::Color(node.Int()), frame, unique);
            break;
        case PropKeys::kObject:
            cur->AsObjectKeys()->Add(node.GetObj(), frame, unique);
            break;
        case PropKeys::kBool:
            cur->AsBoolKeys()->Add(node.Int(), frame, unique);
            break;
        case PropKeys::kSymbol:
            cur->AsSymbolKeys()->Add(node.Sym(), frame, unique);
            break;
        case PropKeys::kVector3:
            cur->AsVector3Keys()->Add(
                Vector3(
                    node.Array()->Float(0), node.Array()->Float(1), node.Array()->Float(2)
                ),
                frame,
                unique
            );
            break;
        case PropKeys::kQuat:
            cur->AsQuatKeys()->Add(
                Hmx::Quat(
                    node.Array()->Float(0),
                    node.Array()->Float(1),
                    node.Array()->Float(2),
                    node.Array()->Float(3)
                ),
                frame,
                unique
            );
            break;
        default: {
            String text;
            text << "Unable to set key value pair for ";
            da->Print(text, kDataArray, true, 0);
            text << " value is ";
            node.Print(text, true, 0);
            MILO_NOTIFY(text.c_str());
            break;
        }
        }
    }
}

void RndPropAnim::SetFrame(float frame, float blend) {
    if (!mInSetFrame) {
        mInSetFrame = true;
        AdvanceFrame(frame);
        float myframe = GetFrame();
        for (std::list<PropKeys *>::iterator it = mPropKeys.begin();
             it != mPropKeys.end();
             ++it) {
            if ((*it)->GetExceptionID() == PropKeys::kDirEvent) {
                ObjKeys *objkeys = (*it)->AsObjectKeys();
                for (int i = 0; i < objkeys->size(); i++) {
                    float objFrame = (*objkeys)[i].frame;
                    if (objFrame > myframe)
                        break;
                    if (objFrame >= mLastFrame && mLastFrame != myframe) {
                        EventTrigger *trig =
                            dynamic_cast<EventTrigger *>((*objkeys)[i].value.Ptr());
                        if (trig)
                            trig->Trigger();
                    }
                }
            }
            (*it)->SetFrame(myframe, blend, mIntensity);
        }
        mLastFrame = myframe;
        mInSetFrame = false;
    }
}

bool RndPropAnim::ChangePropPath(Hmx::Object *o, DataArray *a1, DataArray *a2) {
    if (!a2 || a2->Size() == 0)
        return RemoveKeys(o, a1);
    else {
        std::list<PropKeys *>::iterator keys = FindKeys(o, a1);
        if (keys != mPropKeys.end()) {
            DataNode node(a2, kDataArray);
            (*keys)->SetProp(node);
            return true;
        } else
            return false;
    }
}

DataNode RndPropAnim::ForeachFrame(const DataArray *da) {
    Hmx::Object *obj2 = da->Obj<Hmx::Object>(2);
    DataArray *arr3 = da->Array(3);
    float f4 = da->Float(4);
    float f5 = da->Float(5);
    float f6 = da->Float(6);
    DataNode *var7 = da->Var(7);
    PropKeys *theKeys = GetKeys(obj2, arr3);
    if (!theKeys)
        return 0;
    else {
        for (float fIt = f4; fIt < f5; fIt += f6) {
            ValueFromFrame(theKeys, fIt, var7);
            for (int i = 8; i < da->Size(); i++) {
                da->Command(i)->Execute(true);
            }
        }
        return 1;
    }
}

DataNode RndPropAnim::OnGetIndexFromFrame(const DataArray *da) {
    Hmx::Object *obj = da->GetObj(2);
    DataArray *prop = da->Array(3);
    float f = da->Float(4);
    PropKeys *keys = GetKeys(obj, prop);
    if (!keys)
        return -1;
    else
        return ValueFromFrame(keys, f, &DataNode(0));
}

DataNode RndPropAnim::OnGetFrameFromIndex(const DataArray *da) {
    Hmx::Object *obj = da->GetObj(2);
    DataArray *prop = da->Array(3);
    int i = da->Int(4);
    float frame = -1.0f;
    PropKeys *keys = GetKeys(obj, prop);
    if (!keys)
        return frame;
    else {
        keys->FrameFromIndex(i, frame);
        return frame;
    }
}

DataNode RndPropAnim::OnGetValueFromIndex(const DataArray *da) {
    Hmx::Object *obj = da->GetObj(2);
    DataArray *prop = da->Array(3);
    PropKeys *keys = GetKeys(obj, prop);
    if (!keys)
        return -1;
    else
        return ValueFromIndex(keys, da->Int(4), da->Var(5));
}

DataNode RndPropAnim::OnGetValueFromFrame(const DataArray *da) {
    Hmx::Object *obj = da->GetObj(2);
    DataArray *prop = da->Array(3);
    float f = da->Float(4);
    PropKeys *keys = GetKeys(obj, prop);
    if (!keys)
        return -1;
    else {
        DataNode node(0);
        ValueFromFrame(keys, f, &node);
        return node;
    }
}

DataNode RndPropAnim::OnRemoveKeyframe(DataArray *) {
    sRemoveFrame = true;
    return 0;
}

DataNode RndPropAnim::OnReplaceKeyframe(DataArray *da) {
    sReplaceKey = true;
    sKeyReplace = da->Evaluate(2);
    return 0;
}

DataNode RndPropAnim::OnReplaceFrame(DataArray *da) {
    sReplaceFrame = true;
    sFrameReplace = da->Float(2);
    return 0;
}

DataNode RndPropAnim::OnGetNumKeys(const DataArray *da) {
    Hmx::Object *obj = da->GetObj(2);
    DataArray *prop = da->Array(3);
    PropKeys *keys = GetKeys(obj, prop);
    if (!keys)
        return 0;
    else
        return keys->NumKeys();
}

DataNode RndPropAnim::ForEachTarget(const DataArray *da) {
    ObjPtrList<Hmx::Object> objList(this);
    const char *arrstr = da->Str(2);
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        PropKeys *cur = *it;
        if (arrstr == gNullStr || cur->Target()->ClassName() == arrstr) {
            objList.push_back(cur->Target());
        }
    }
    DataNode *var = da->Var(3);
    DataNode node(*var);
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        *var = (*it)->Target();
        for (int i = 4; i < da->Size(); i++) {
            da->Command(i)->Execute(true);
        }
    }
    *var = node;
    return 0;
}

struct ForAllKeyframesSorter {
    bool operator()(const DataArray *arr1, const DataArray *arr2) {
        return arr1->Float(2) < arr2->Float(2);
    }
};

DataNode RndPropAnim::ForAllKeyframes(const DataArray *da) {
    std::vector<DataArrayPtr> ptrs;
    for (std::list<PropKeys *>::iterator it = mPropKeys.begin(); it != mPropKeys.end();
         ++it) {
        PropKeys *cur = *it;
        if (cur->Target() && cur->Prop()) {
            for (int i = 0; i < cur->NumKeys(); i++) {
                float frame = 0;
                cur->FrameFromIndex(i, frame);
                DataArrayPtr ptr(nullptr);
                ptr->Resize(4);
                ptr->Node(0) = cur->Target();
                ptr->Node(1) = cur->Prop();
                ptr->Node(2) = frame;
                ValueFromIndex(cur, i, &ptr->Node(3));
                ptrs.push_back(ptr);
            }
        }
    }
    std::sort(ptrs.begin(), ptrs.end(), ForAllKeyframesSorter());
    DataNode *var2 = da->Var(2);
    DataNode *var3 = da->Var(3);
    DataNode *var4 = da->Var(4);
    DataNode *var5 = da->Var(5);
    for (int i = 0; i < ptrs.size(); i++) {
        DataArray *curPtr = ptrs[i];
        *var2 = curPtr->Node(0);
        *var3 = curPtr->Node(1);
        *var4 = curPtr->Node(2);
        *var5 = curPtr->Node(3);
        for (int j = 6; j < da->Size(); j++) {
            da->Command(j)->Execute(true);
        }
    }
    return 0;
}
