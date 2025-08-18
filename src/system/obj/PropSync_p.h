#pragma once
#include "obj/Data.h"
#include "math/Geo.h"
#include "math/Vec.h"
#include "math/Mtx.h"

enum PropOp {
    kPropGet = 1,
    kPropSet = 2,
    kPropInsert = 4,
    kPropRemove = 8,
    kPropSize = 16,
    kPropHandle = 32,
    kPropUnknown0x40 = 64,
};

bool PropSync(class String &, DataNode &, DataArray *, int, PropOp);
bool PropSync(FilePath &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Color &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Matrix3 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Sphere &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Vector2 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Vector3 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Transform &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Rect &, DataNode &, DataArray *, int, PropOp);
// bool PropSync(Box &, DataNode &, DataArray *, int, PropOp);

inline bool PropSync(float &f, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x17);
    if (op == kPropGet)
        node = f;
    else
        f = node.Float();
    return true;
}

inline bool
PropSync(unsigned char &uc, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x21);
    if (op == kPropGet)
        node = uc;
    else
        uc = node.Int();
    return true;
}

inline bool PropSync(int &iref, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x2C);
    if (op == kPropGet)
        node = iref;
    else
        iref = node.Int();
    return true;
}

inline bool PropSync(short &s, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x36);
    if (op == kPropGet)
        node = s;
    else
        s = node.Int();
    return true;
}

inline bool PropSync(bool &b, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x40);
    if (op == kPropGet)
        node = b;
    else
        b = node.Int() != 0;
    return true;
}

inline bool PropSync(Symbol &sym, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x4A);
    if (op == kPropGet)
        node = sym;
    else
        sym = node.Str();
    return true;
}

// #include "obj/Object.h"
template <class T>
class ObjPtr;

template <class T>
bool PropSync(ObjPtr<T> &, DataNode &, DataArray *, int, PropOp);

template <class T>
class ObjOwnerPtr;

template <class T>
bool PropSync(ObjOwnerPtr<T> &, DataNode &, DataArray *, int, PropOp);

template <class T1, class T2>
class ObjPtrList;

template <class T>
bool PropSync(ObjPtrList<T, ObjectDir> &, DataNode &, DataArray *, int, PropOp);

template <class T1, class T2>
class ObjPtrVec;

template <class T>
bool PropSync(ObjPtrVec<T, ObjectDir> &, DataNode &, DataArray *, int, PropOp);

template <class T>
class ObjList;

template <class T>
bool PropSync(ObjList<T> &objList, DataNode &node, DataArray *prop, int i, PropOp op);
// {
//     if (op == kPropUnknown0x40)
//         return false;
//     else if (i == prop->Size()) {
//         MILO_ASSERT(op == kPropSize, 0x1A6);
//         node = objList.size();
//         return true;
//     } else {
//         int count = prop->Int(i++);
//         typename std::list<T>::iterator it = NextItr(objList.begin(), count);
//         if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
//             return PropSync(*it, node, prop, i, op);
//         } else if (op == kPropRemove) {
//             objList.erase(it);
//             return true;
//         } else if (op == kPropInsert) {
//             T item(objList.Owner());
//             if (PropSync(item, node, prop, i, kPropInsert)) {
//                 objList.insert(it, item);
//                 return true;
//             }
//         }
//         return false;
//     }
// }
