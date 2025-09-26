#include "rndobj/PropKeys.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/DataUtl.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

Hmx::Object *ObjectStage::sOwner;
Message PropKeys::sInterpMessage(gNullStr, 0, 0, 0, 0, 0);

float CalcSpline(float, float *const);

void PropKeys::Copy(const PropKeys *keys) {
    mInterpolation = keys->mInterpolation;
    mInterpHandler = keys->mInterpHandler;
    mPropExceptionID = keys->mPropExceptionID;
    unk34 = keys->unk34;
}

int PropKeys::SetKey(float f1) {
    float frame = 0;
    for (int i = 0; i < NumKeys(); i++) {
        if (FrameFromIndex(i, frame) != 0 && NearlyEqual(f1, frame)) {
            return i;
        }
    }
    return -1;
}

void PropKeys::Print() {
    TheDebug << "      target: " << mTarget.Ptr() << "\n";
    TheDebug << "      property: " << mProp << "\n";
    TheDebug << "      interpolation: " << mInterpolation << "\n";

    for (int i = 0; i < NumKeys(); i++) {
        float frame = 0.0f;
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
        case kQuat:
            TheDebug << (*AsQuatKeys())[i].value;
            break;
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

PropKeys::ExceptionID PropKeys::PropExceptionID(Hmx::Object *o, DataArray *arr) {
    if (o && arr) {
        static Symbol rotation("rotation");
        static Symbol scale("scale");
        static Symbol position("position");
        static Symbol event("event");
        if (arr->Size() == 1) {
            Symbol sym = arr->Sym(0);
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

void FloatKeys::SetToCurrentVal(int i) {
    (*this)[i].value = mTarget->Property(mProp, true)->Float();
}

void ColorKeys::SetToCurrentVal(int i) {
    (*this)[i].value = Hmx::Color(mTarget->Property(mProp, true)->Int());
}

void BoolKeys::SetToCurrentVal(int i) {
    (*this)[i].value = mTarget->Property(mProp, true)->Int();
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

void SymbolKeys::SetToCurrentVal(int i) {
    if (mPropExceptionID == kMacro) {
        if (0 < i) {
            (*this)[i].value = (*this)[i - 1].value;
        }
    } else
        (*this)[i].value = mTarget->Property(mProp, true)->Sym();
}

PropKeys::~PropKeys() {
    if (mProp) {
        mProp->Release();
        mProp = nullptr;
    }
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
        if ((mProp && GetPropertyVal(o, mProp, false))
            || (mPropExceptionID == kTransQuat || mPropExceptionID == kTransScale
                || mPropExceptionID == kTransPos || mPropExceptionID == kDirEvent)) {
            if (o && mProp) {
                mProp->Release();
                mProp = nullptr;
            }
            mTarget = o;
            SetPropExceptionID();
        }
    }
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

void ColorKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        Hmx::Color col;
        int idx = ColorAt(frame, col);
        Multiply(col, f3, col);
        mTarget->SetProperty(mProp, col.Pack());
        mLastKeyFrameIndex = idx;
    }
}

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

BinStreamRev &operator>>(BinStreamRev &bs, ObjectStage &stage) {
    if (bs.mRev > 8) {
        ObjPtr<ObjectDir> ptr(stage.Owner());
        bs >> ptr;
    }
    bs >> (ObjPtr<Hmx::Object> &)stage;
    return bs;
}

BinStream &operator<<(BinStream &bs, const ObjectStage &stage) {
    ObjPtr<ObjectDir> dirPtr(stage.Owner(), (stage.Ptr()) ? stage.Ptr()->Dir() : nullptr);
    bs << dirPtr;
    bs << ObjPtr<Hmx::Object>(stage);
    return bs;
}

PropKeys::PropKeys(Hmx::Object *targetOwner, Hmx::Object *target)
    : mTarget(targetOwner, target), mProp(nullptr), mKeysType(kFloat),
      mInterpolation(kLinear), mPropExceptionID(kNoException), mTrans(nullptr),
      mLastKeyFrameIndex(-2), unk34(false) {}

void PropKeys::Load(BinStreamRev &bs) {
    if (bs.mRev < 7)
        MILO_FAIL("PropKeys::Load should not be called before version 7");
    else {
        int iVal;
        bs >> iVal;
        mKeysType = (AnimKeysType)iVal;
        bs >> mTarget;
        bs >> mProp;

        if (bs.mRev >= 8)
            bs >> iVal;
        else if (mKeysType == kObject || mKeysType == kBool)
            iVal = 0;
        else
            iVal = 1;

        if (bs.mRev < 11 && iVal == 4) {
            mPropExceptionID = kMacro;
            mInterpolation = kStep;
        } else
            mInterpolation = (Interpolation)iVal;

        if (bs.mRev > 9) {
            Symbol sym;
            bs >> sym;
            if (!sym.Null()) {
                SetInterpHandler(sym);
            }
        }

        if (bs.mRev > 10) {
            bs >> iVal;
            mPropExceptionID = (ExceptionID)iVal;
        }

        if (bs.mRev > 0xC) {
            bs >> unk34;
        }
        SetPropExceptionID();
    }
}

int BoolKeys::BoolAt(float frame, bool &b) {
    MILO_ASSERT(size(), 0x28A);
    return AtFrame(frame, b);
}

void FloatKeys::SetFrame(float frame, float f2, float f3) {
    if (mProp && mTarget && size()) {
        int idx;
        if (mPropExceptionID != kHandleInterp) {
            float val;
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

int ObjectKeys::ObjectAt(float frame, Hmx::Object *&obj) {
    MILO_ASSERT(size(), 0x258);
    return AtFrame(frame, obj);
}

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

int SymbolKeys::SymbolAt(float frame, Symbol &sym) {
    MILO_ASSERT(size(), 0x350);
    return AtFrame(frame, sym);
}

void ObjectKeys::SetToCurrentVal(int i) {
    if (mPropExceptionID != kDirEvent) {
        (*this)[i].value = ObjectStage(mTarget->Property(mProp, true)->GetObj());
    }
}

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
            break;
        }
        switch (mInterpolation) {
        case kStep: {
            int loc8c = -1;
            int loc90 = -1;
            std::vector<Symbol> vec;
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
        mLastKeyFrameIndex = idx;
    }
}

void FloatKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const FloatKeys *newKeys = dynamic_cast<const FloatKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

void ColorKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const ColorKeys *newKeys = dynamic_cast<const ColorKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

void BoolKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const BoolKeys *newKeys = dynamic_cast<const BoolKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
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

void Vector3Keys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const Vector3Keys *newKeys = dynamic_cast<const Vector3Keys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
}

void SymbolKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const SymbolKeys *newKeys = dynamic_cast<const SymbolKeys *>(keys);
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
            if (idx == 0) {
                points[0] = prev->value;
            } else {
                points[0] = this->at(idx - 1).value;
            }
            if (idx == size() - 1) {
                points[3] = next->value;
            } else {
                points[3] = this->at(idx + 1).value;
            }
            fl = CalcSpline(ref, points);
        }
        break;
    case kHermite:
        Interp(prev->value, next->value, ref * ref * (ref * -2.0f + 3.0f), fl);
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

void ObjectKeys::Copy(const PropKeys *keys) {
    PropKeys::Copy(keys);
    clear();
    if (keys->KeysType() == mKeysType) {
        const ObjectKeys *newKeys = dynamic_cast<const ObjectKeys *>(keys);
        insert(begin(), newKeys->begin(), newKeys->end());
    }
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
