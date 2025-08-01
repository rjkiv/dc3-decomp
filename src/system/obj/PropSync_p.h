#pragma once

#include "obj/Data.h"
#include "math/Vec.h"

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
// bool PropSync(Hmx::Matrix3 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Sphere &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Vector2 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Vector3 &, DataNode &, DataArray *, int, PropOp);
// bool PropSync(Transform &, DataNode &, DataArray *, int, PropOp);
// bool PropSync(Hmx::Rect &, DataNode &, DataArray *, int, PropOp);
// bool PropSync(Box &, DataNode &, DataArray *, int, PropOp);

inline bool PropSync(float &f, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x17);
    if (op == kPropGet)
        node = DataNode(f);
    else
        f = node.Float();
    return true;
}

inline bool
PropSync(unsigned char &uc, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x21);
    if (op == kPropGet)
        node = DataNode(uc);
    else
        uc = node.Int();
    return true;
}

inline bool PropSync(int &iref, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x2C);
    if (op == kPropGet)
        node = DataNode(iref);
    else
        iref = node.Int();
    return true;
}

inline bool PropSync(short &s, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x36);
    if (op == kPropGet)
        node = DataNode(s);
    else
        s = node.Int();
    return true;
}

inline bool PropSync(bool &b, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x40);
    if (op == kPropGet)
        node = DataNode(b);
    else
        b = node.Int() != 0;
    return true;
}

inline bool PropSync(Symbol &sym, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x4A);
    if (op == kPropGet)
        node = DataNode(sym);
    else
        sym = node.Str();
    return true;
}
