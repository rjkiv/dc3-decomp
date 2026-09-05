#include "rndobj/PropKeys.h"
#include "math/Key.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/DataUtl.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

Hmx::Object *ObjectStage::sOwner;
Message PropKeys::sInterpMessage(gNullStr, 0, 0, 0, 0, 0);

float CalcSpline(float t, float *const pts) {
    // from the dwarf dump
    float t2 = t * t; // f9
    float t3 = t2 * t; // f10

    float paren1 = pts[2] - pts[0];
    float paren2 = -(pts[2] * 3.0f - (pts[1] * 3.0f - pts[0]));
    paren2 += pts[3];
    float paren3 = (pts[2] * 4.0f + (pts[0] * 2.0f - pts[1] * 5.0f)) - pts[3];
    return (pts[1] * 2.0f + paren1 * t + paren2 * t3 + paren3 * t2) / 2;
}

#pragma region PropKeys

// ------------------------------------------------
// PropKeys
// ------------------------------------------------

PropKeys::PropKeys(Hmx::Object *targetOwner, Hmx::Object *target)
    : mTarget(targetOwner, target), mProp(nullptr), mKeysType(kFloat),
      mInterpolation(kLinear), mPropExceptionID(kNoException), mTrans(nullptr),
      mLastKeyFrameIndex(-2), unk34(false) {}

PropKeys::~PropKeys() {
    if (mProp) {
        mProp->Release();
        mProp = nullptr;
    }
}

int PropKeys::SetKey(float frame) {
    float f = 0;
    for (int i = 0; i < NumKeys(); i++) {
        if (FrameFromIndex(i, f) != 0 && NearlyEqual(frame, f)) {
            return i;
        }
    }
    return -1;
}

void PropKeys::Save(BinStream &bs) {
    bs << mKeysType;
    bs << mTarget;
    bs << mProp;
    bs << mInterpolation;
    bs << mInterpHandler;
    bs << mPropExceptionID;
    bs << unk34;
}

void PropKeys::Load(BinStreamRev &d) {
    if (d.rev < 7)
        MILO_FAIL("PropKeys::Load should not be called before version 7");
    else {
        int iVal;
        d >> iVal;
        mKeysType = (AnimKeysType)iVal;
        d >> mTarget;
        d >> mProp;

        if (d.rev >= 8)
            d >> iVal;
        else if (mKeysType == kObject || mKeysType == kBool)
            iVal = 0;
        else
            iVal = 1;

        if (d.rev < 11 && iVal == 4) {
            mPropExceptionID = kMacro;
            mInterpolation = kStep;
        } else
            mInterpolation = (Interpolation)iVal;

        if (d.rev > 9) {
            Symbol sym;
            d >> sym;
            if (!sym.Null()) {
                SetInterpHandler(sym);
            }
        }

        if (d.rev > 10) {
            d >> iVal;
            mPropExceptionID = (ExceptionID)iVal;
        }

        if (d.rev > 0xC) {
            d >> unk34;
        }
        SetPropExceptionID();
    }
}

void PropKeys::Copy(const PropKeys *keys) {
    mInterpolation = keys->mInterpolation;
    mInterpHandler = keys->mInterpHandler;
    mPropExceptionID = keys->mPropExceptionID;
    unk34 = keys->unk34;
}

void PropKeys::ReSort() {
    switch (mKeysType) {
    case kFloat:
        std::sort(AsFloatKeys()->begin(), AsFloatKeys()->end());
        break;
    case kColor:
        std::sort(AsColorKeys()->begin(), AsColorKeys()->end());
        break;
    case kObject:
        std::sort(AsObjectKeys()->begin(), AsObjectKeys()->end());
        break;
    case kBool:
        std::sort(AsBoolKeys()->begin(), AsBoolKeys()->end());
        break;
    case kSymbol:
        std::sort(AsSymbolKeys()->begin(), AsSymbolKeys()->end());
        break;
    case kVector3:
        std::sort(AsVector3Keys()->begin(), AsVector3Keys()->end());
        break;
    case kQuat:
        std::sort(AsQuatKeys()->begin(), AsQuatKeys()->end());
        break;
    }
}

void PropKeys::ChangeFrame(int idx, float new_frame, bool sort) {
    switch (mKeysType) {
    case kFloat:
        (*AsFloatKeys())[idx].frame = new_frame;
        break;
    case kColor:
        (*AsColorKeys())[idx].frame = new_frame;
        break;
    case kObject:
        (*AsObjectKeys())[idx].frame = new_frame;
        break;
    case kBool:
        (*AsBoolKeys())[idx].frame = new_frame;
        break;
    case kSymbol:
        (*AsSymbolKeys())[idx].frame = new_frame;
        break;
    case kVector3:
        (*AsVector3Keys())[idx].frame = new_frame;
        break;
    case kQuat:
        (*AsQuatKeys())[idx].frame = new_frame;
        break;
    default:
        MILO_NOTIFY("can not replace frame, unknown type");
        break;
    }
    if (sort)
        ReSort();
}

void PropKeys::Print() {
    TheDebug << "      target: " << mTarget.Ptr() << "\n";
    TheDebug << "      property: " << mProp << "\n";
    TheDebug << "      interpolation: " << mInterpolation << "\n";
    for (int i = 0; i < NumKeys(); i++) {
        float frame = 0;
        FrameFromIndex(i, frame);
        TheDebug << "      " << frame << " -> ";
        switch (mKeysType) {
        case kFloat:
            TheDebug << (*AsFloatKeys())[i].value;
            break;
        case kColor:
            TheDebug << (*AsColorKeys())[i].value;
            break;
        case kObject:
            TheDebug << (*AsObjectKeys())[i].value;
            break;
        case kBool:
            TheDebug << (*AsBoolKeys())[i].value;
            break;
        case kQuat: {
            // stupid but it matches
            const Hmx::Quat &q = (*AsQuatKeys())[i].value;
            TheDebug << q;
            break;
        }
        case kVector3:
            TheDebug << (*AsVector3Keys())[i].value;
            break;
        case kSymbol:
            TheDebug << (*AsSymbolKeys())[i].value;
            break;
        }
        TheDebug << "\n";
    }
}

void PropKeys::SetPropExceptionID() {
    if (!mInterpHandler.Null())
        mPropExceptionID = kHandleInterp;
    else {
        if (mPropExceptionID != kMacro) {
            mPropExceptionID = PropExceptionID(mTarget.Ptr(), mProp);
            if (mPropExceptionID == kTransQuat || mPropExceptionID == kTransScale
                || mPropExceptionID == kTransPos) {
                if ((Hmx::Object *)mTrans != mTarget.Ptr()) {
                    mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
                }
            }
        }
    }
}

void PropKeys::SetInterpHandler(Symbol sym) {
    mInterpHandler = sym;
    SetPropExceptionID();
}

void PropKeys::SetProp(DataNode &node) {
    if (node.Type() == kDataArray) {
        DataArray *nodeArr = node.Array();
        if (mProp) {
            mProp->Release();
            mProp = nullptr;
        }
        mProp = nodeArr->Clone(true, false, 0);

    } else
        MILO_NOTIFY("unknown prop set type");
    SetPropExceptionID();
}

void PropKeys::SetTarget(Hmx::Object *o) {
    if (mTarget.Ptr() != o) {
        bool release = (mProp && GetPropertyVal(o, mProp, false))
            || (mPropExceptionID == kTransQuat || mPropExceptionID == kTransPos
                || mPropExceptionID == kTransScale || mPropExceptionID == kDirEvent);
        if ((!o || !release) && mProp) {
            mProp->Release();
            mProp = nullptr;
        }
        mTarget = o;
        SetPropExceptionID();
    }
}

PropKeys::ExceptionID PropKeys::PropExceptionID(Hmx::Object *o, DataArray *path) {
    if (o && path) {
        static Symbol rotation("rotation");
        static Symbol scale("scale");
        static Symbol position("position");
        static Symbol event("event");
        if (path->Size() == 1) {
            Symbol sym = path->Sym(0);
            if (sym == rotation && IsASubclass(o->ClassName(), "Trans")) {
                return kTransQuat;
            }
            if (sym == scale && IsASubclass(o->ClassName(), "Trans")) {
                return kTransScale;
            }
            if (sym == position && IsASubclass(o->ClassName(), "Trans")) {
                return kTransPos;
            }
            if (sym == event && IsASubclass(o->ClassName(), "ObjectDir")) {
                return kDirEvent;
            }
        }
    }
    return kNoException;
}

#pragma endregion
#pragma region FloatKeys

// ------------------------------------------------
// FloatKeys
// ------------------------------------------------

void FloatKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        float val;
        int idx;
        if (mPropExceptionID != kHandleInterp) {
            idx = FloatAt(frame, val);
            mTarget->SetProperty(mProp, val * f3);
        } else {
            const Key<float> *prev;
            const Key<float> *next;
            float ref = 0;
            idx = AtFrame(frame, prev, next, ref);
            sInterpMessage.SetType(mInterpHandler);
            sInterpMessage[0] = prev->value * f3;
            sInterpMessage[1] = next->value * f3;
            sInterpMessage[2] = ref;
            sInterpMessage[3] = next->frame;
            if (idx >= 1)
                sInterpMessage[4] = (*this)[idx - 1].value * f3;
            else
                sInterpMessage[4] = 0;
            mTarget->Handle(sInterpMessage, true);
        }
        mLastKeyFrameIndex = idx;
    }
}

int FloatKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(0, frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void FloatKeys::SetToCurrentVal(int i) {
    (*this)[i].value = mTarget->Property(mProp, true)->Float();
}

void FloatKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const FloatKeys *newKeys = dynamic_cast<const FloatKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int FloatKeys::FloatAt(float frame, float &fl) {
    MILO_ASSERT(size(), 0x1B5);
    fl = 0.0f;
    float ref = 0.0f;
    const Key<float> *prev;
    const Key<float> *next;
    int at = AtFrame(frame, prev, next, ref);
    switch (mInterpolation) {
    case kStep:
        fl = prev->value;
        break;
    case kLinear:
        Interp(prev->value, next->value, ref, fl);
        break;
    case kSpline:
        if (size() < 3 || prev == next) {
            Interp(prev->value, next->value, ref, fl);
        } else {
            float points[4];
            points[1] = prev->value;
            points[2] = next->value;
            int idx = (prev - begin());
            int idx1 = idx + 1;
            points[0] = idx == 0 ? prev->value : this->at(idx - 1).value;
            points[3] = idx1 == size() - 1 ? next->value : this->at(idx1 + 1).value;
            fl = CalcSpline(ref, points);
        }
        break;
    case kHermite:
        Interp(prev->value, next->value, (ref * ref) * (ref * -2.0f + 3.0f), fl);
        break;
    case kEaseIn:
        Interp(prev->value, next->value, ref * ref * ref, fl);
        break;
    case kEaseOut:
        ref = 1.0f - ref;
        Interp(prev->value, next->value, -(ref * ref * ref - 1.0f), fl);
        break;
    }
    return at;
}

#pragma endregion
#pragma region ColorKeys

// ------------------------------------------------
// ColorKeys
// ------------------------------------------------

void ColorKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        Hmx::Color col;
        int idx = ColorAt(frame, col);
        Multiply(col, f3, col);
        mTarget->SetProperty(mProp, col.Pack());
        mLastKeyFrameIndex = idx;
    }
}

int ColorKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(Hmx::Color(0), frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void ColorKeys::SetToCurrentVal(int i) {
    (*this)[i].value = Hmx::Color(mTarget->Property(mProp, true)->Int());
}

void ColorKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const ColorKeys *newKeys = dynamic_cast<const ColorKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int ColorKeys::ColorAt(float frame, Hmx::Color &color) {
    MILO_ASSERT(size(), 0x215);
    color.Set(0, 0, 0);
    int at = 0;
    switch (mInterpolation) {
    case kStep:
        const Key<Hmx::Color> *prevstep;
        const Key<Hmx::Color> *nextstep;
        float refstep;
        at = AtFrame(frame, prevstep, nextstep, refstep);
        color = prevstep->value;
        break;
    case kLinear:
        at = AtFrame(frame, color);
        break;
    case kEaseIn:
        const Key<Hmx::Color> *prev5;
        const Key<Hmx::Color> *next5;
        float ref5;
        AtFrame(frame, prev5, next5, ref5);
        if (prev5)
            Interp(prev5->value, next5->value, ref5 * ref5 * ref5, color);
        break;
    case kEaseOut:
        const Key<Hmx::Color> *prev;
        const Key<Hmx::Color> *next;
        float ref;
        AtFrame(frame, prev, next, ref);
        ref = 1.0f - ref;
        if (prev)
            Interp(prev->value, next->value, -(ref * ref * ref - 1.0f), color);
        break;
    default:
        break;
    }
    return at;
}

#pragma endregion
#pragma region ObjectStage

// ------------------------------------------------
// ObjectStage
// ------------------------------------------------

BinStream &operator<<(BinStream &bs, const ObjectStage &stage) {
    ObjPtr<ObjectDir> dirPtr(stage.Owner(), (stage.Ptr()) ? stage.Ptr()->Dir() : nullptr);
    bs << dirPtr;
    bs << ObjPtr<Hmx::Object>(stage);
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &bs, ObjectStage &stage) {
    ObjectDir *dir = nullptr;
    if (bs.rev > 8) {
        ObjPtr<ObjectDir> dirPtr(stage.Owner(), nullptr);
        dirPtr.Load(bs.stream, true, dir);
        dir = dirPtr.Ptr();
    }
    stage.Load(bs.stream, true, dir);
    return bs;
}

#pragma endregion
#pragma region ObjectKeys

// ------------------------------------------------
// ObjectKeys
// ------------------------------------------------

void ObjectKeys::SetFrame(float frame, float blend, float) {
    if (!mProp || !mTarget || !size())
        return;
    int idx = 0;
    switch (mPropExceptionID) {
    case kDirEvent:
        break;
    case kHandleInterp: {
        const Key<ObjectStage> *prev;
        const Key<ObjectStage> *next;
        float ref = 0.0f;
        idx = AtFrame(frame, prev, next, ref);
        sInterpMessage.SetType(mInterpHandler);
        sInterpMessage[0] = prev->value.Ptr();
        sInterpMessage[1] = next->value.Ptr();
        sInterpMessage[2] = ref;
        sInterpMessage[3] = next->frame;
        if (idx >= 1)
            sInterpMessage[4] = (*this)[idx - 1].value.Ptr();
        else
            sInterpMessage[4] = 0;
        mTarget->Handle(sInterpMessage, true);
        break;
    }
    default: {
        Hmx::Object *obj;
        idx = ObjectAt(frame, obj);
        if (mInterpolation != kStep || mLastKeyFrameIndex != idx) {
            mTarget->SetProperty(mProp, obj);
        }
        break;
    }
    }
    mLastKeyFrameIndex = idx;
}

int ObjectKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = ObjKeys::Add(0, frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void ObjectKeys::SetToCurrentVal(int i) {
    if (mPropExceptionID != kDirEvent) {
        (*this)[i].value = mTarget->Property(mProp)->GetObj();
    }
}

void ObjectKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const ObjectKeys *newKeys = dynamic_cast<const ObjectKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int ObjectKeys::ObjectAt(float frame, Hmx::Object *&obj) {
    MILO_ASSERT(size(), 0x258);
    return AtFrame(frame, obj);
}

#pragma endregion
#pragma region BoolKeys

// ------------------------------------------------
// BoolKeys
// ------------------------------------------------

void BoolKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        int idx = 0;
        if (mPropExceptionID == kNoException) {
            bool b;
            idx = BoolAt(frame, b);
            if (mInterpolation != kStep || mLastKeyFrameIndex != idx) {
                mTarget->SetProperty(mProp, b);
            }
        } else if (mPropExceptionID == kHandleInterp) {
            bool b;
            idx = BoolAt(frame, b);
            if (mLastKeyFrameIndex != idx) {
                sInterpMessage.SetType(mInterpHandler);
                sInterpMessage[0] = b;
                sInterpMessage[1] = frame;
                mTarget->Handle(sInterpMessage, true);
            }
        }
        mLastKeyFrameIndex = idx;
    }
}

int BoolKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(true, frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void BoolKeys::SetToCurrentVal(int i) {
    (*this)[i].value = mTarget->Property(mProp, true)->Int();
}

void BoolKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const BoolKeys *newKeys = dynamic_cast<const BoolKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int BoolKeys::BoolAt(float frame, bool &b) {
    MILO_ASSERT(size(), 0x28A);
    return AtFrame(frame, b);
}

#pragma endregion
#pragma region QuatKeys

// ------------------------------------------------
// QuatKeys
// ------------------------------------------------

void QuatKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        int idx = 0;
        if (mPropExceptionID == kTransQuat) {
            if (mTrans != mTarget) {
                mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
            }
            Vector3 v48;
            MakeScale(mTrans->LocalXfm().m, v48);
            if (NearlyEqual(mVec, v48, 0.001f)) {
                v48 = mVec;
            } else
                mVec = v48;
            Hmx::Quat quat;
            Hmx::Matrix3 mtx;
            idx = QuatAt(frame, quat);
            MakeRotMatrix(quat, mtx);
            Scale(v48, mtx, mtx);
            mTrans->SetLocalRot(mtx);
        }
        mLastKeyFrameIndex = idx;
    }
}

int QuatKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(Hmx::Quat(0, 0, 0, 0), frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void QuatKeys::SetToCurrentVal(int i) {
    if (mPropExceptionID == kTransQuat) {
        if (mTrans != mTarget) {
            mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
        }
        Hmx::Matrix3 m38;
        Normalize(mTrans->LocalXfm().m, m38);
        Hmx::Quat q48;
        Hmx::Quat q58(m38);
        Normalize(q58, q48);
        (*this)[i].value = q48;
    }
}

void QuatKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const QuatKeys *newKeys = dynamic_cast<const QuatKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int QuatKeys::QuatAt(float frame, Hmx::Quat &q) {
    MILO_ASSERT(size(), 0x2AF);
    float ref = 0.0f;
    const Key<Hmx::Quat> *prev;
    const Key<Hmx::Quat> *next;
    int at = AtFrame(frame, prev, next, ref);
    if (mInterpolation == kSpline) {
        QuatSpline(*this, prev, next, ref, q);
    } else
        switch (mInterpolation) {
        case kStep:
            q = prev->value;
            break;
        case kLinear:
            FastInterp(prev->value, next->value, ref, q);
            break;
        case kSlerp:
            Interp(prev->value, next->value, ref, q);
            break;
        case kEaseIn:
            FastInterp(prev->value, next->value, ref * ref * ref, q);
            break;
        case kEaseOut:
            ref = 1 - ref;
            FastInterp(prev->value, next->value, -(ref * ref * ref - 1), q);
            break;
        }

    return at;
}

#pragma endregion
#pragma region Vector3Keys

// ------------------------------------------------
// Vector3Keys
// ------------------------------------------------

void Vector3Keys::SetFrame(float frame, float blend, float) {
    if (!mProp || !mTarget || !size())
        return;
    int idx = 0;
    switch (mPropExceptionID) {
    case kTransScale: {
        if (mTrans != mTarget)
            mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
        Vector3 v70;
        Hmx::Matrix3 m40;
        Normalize(mTrans->LocalXfm().m, m40);
        MakeEuler(m40, v70);
        Hmx::Matrix3 m64;
        MakeRotMatrix(v70, m64, false);
        Vector3 v7c;
        idx = Vector3At(frame, v7c);
        Scale(v7c, m64, m64);
        mTrans->SetLocalRot(m64);
        break;
    }
    case kTransPos: {
        if (mTrans != mTarget)
            mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
        Vector3 v88;
        idx = Vector3At(frame, v88);
        mTrans->SetLocalPos(v88);
        break;
    }
    default:
        break;
    }
    mLastKeyFrameIndex = idx;
}

int Vector3Keys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(Vector3(0, 0, 0), frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void Vector3Keys::SetToCurrentVal(int i) {
    switch (mPropExceptionID) {
    case kTransScale: {
        if (mTrans != mTarget) {
            mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
        }
        Vector3 v28;
        MakeScale(mTrans->LocalXfm().m, v28);
        (*this)[i].value = v28;
        break;
    }
    case kTransPos: {
        if (mTrans != mTarget) {
            mTrans = dynamic_cast<RndTransformable *>(mTarget.Ptr());
        }
        (*this)[i].value = mTrans->LocalXfm().v;
        break;
    }
    default:
        break;
    }
}

void Vector3Keys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const Vector3Keys *newKeys = dynamic_cast<const Vector3Keys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int Vector3Keys::Vector3At(float frame, Vector3 &vec) {
    MILO_ASSERT(size(), 0x306);
    float ref = 0.0f;
    const Key<Vector3> *prev;
    const Key<Vector3> *next;
    int idx = AtFrame(frame, prev, next, ref);
    switch (mInterpolation) {
    case kNoException:
        vec = prev->value;
        break;
    case kTransQuat:
        Interp(prev->value, next->value, ref, vec);
        break;
    case kTransScale:
        InterpVector(*this, prev, next, ref, true, vec, 0);
        break;
    case kDirEvent:
        Interp(prev->value, next->value, ref * ref * (ref * -2.0f + 3.0f), vec);
        break;
    case kHandleInterp:
        Interp(prev->value, next->value, ref * ref * ref, vec);
        break;
    case kMacro:
        ref = 1.0f - ref;
        Interp(prev->value, next->value, -(ref * ref * ref - 1.0f), vec);
        break;
    }
    return idx;
}

#pragma endregion
#pragma region SymbolKeys

// ------------------------------------------------
// SymbolKeys
// ------------------------------------------------

void SymbolKeys::SetFrame(float frame, float blend, float) {
    if (mProp && mTarget && size()) {
        int idx = 0;
        switch (mPropExceptionID) {
        case kHandleInterp: {
            const Key<Symbol> *prev;
            const Key<Symbol> *next;
            float ref = 0.0f;
            idx = AtFrame(frame, prev, next, ref);
            sInterpMessage.SetType(mInterpHandler);
            sInterpMessage[0] = prev->value;
            sInterpMessage[1] = next->value;
            sInterpMessage[2] = ref;
            sInterpMessage[3] = next->frame;
            if (idx >= 1)
                sInterpMessage[4] = (*this)[idx - 1].value;
            else
                sInterpMessage[4] = 0;
            mTarget->Handle(sInterpMessage, true);
            break;
        }
        case kMacro: {
            Symbol s;
            idx = SymbolAt(frame, s);
            if (mInterpolation != kStep || mLastKeyFrameIndex != idx) {
                mTarget->SetProperty(mProp, DataGetMacro(s)->Int(0));
            }
            break;
        }
        default:
            switch (mInterpolation) {
            case kStep: {
                int loc8c = -1;
                int loc90 = -1;
                KeysLessEq(frame, loc8c, loc90);
                if (loc8c != -1) {
                    int i = loc8c;
                    if (unk30) {
                        MinEq(loc8c, unk2c + 1);
                        i = loc8c;
                    }
                    for (; i <= loc90; i++) {
                        Key<Symbol> &cur = (*this)[i];
                        if (i < unk28 || i > unk2c) {
                            mTarget->SetProperty(mProp, cur.value);
                        }
                    }
                }
                unk28 = loc8c;
                unk2c = loc90;
                break;
            }
            case kLinear: {
                Symbol s;
                idx = SymbolAt(frame, s);
                mTarget->SetProperty(mProp, s);
                break;
            }
            default:
                break;
            }
            break;
        }
        mLastKeyFrameIndex = idx;
    }
}

int SymbolKeys::SetKey(float frame) {
    if (!mProp || !mTarget.Ptr())
        return -1;
    else {
        int retVal = PropKeys::SetKey(frame);
        if (retVal < 0)
            retVal = Add(Symbol(), frame, false);
        SetToCurrentVal(retVal);
        return retVal;
    }
}

void SymbolKeys::SetToCurrentVal(int i) {
    if (mPropExceptionID == kMacro) {
        if (i > 0) {
            (*this)[i].value = (*this)[i - 1].value;
        }
    } else
        (*this)[i].value = mTarget->Property(mProp, true)->Sym();
}

void SymbolKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const SymbolKeys *newKeys = dynamic_cast<const SymbolKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

int SymbolKeys::SymbolAt(float frame, Symbol &sym) {
    MILO_ASSERT(size(), 0x350);
    return AtFrame(frame, sym);
}

#pragma endregion
