#pragma once
#include <vector>
#include <list>

namespace Hmx {
    class Matrix3;
    class Color;
    class Rect;
}

class DataNode;
class DataArray;
class ObjectDir;

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
bool PropSync(class FilePath &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Color &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Matrix3 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Sphere &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Vector2 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Vector3 &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Transform &, DataNode &, DataArray *, int, PropOp);
bool PropSync(Hmx::Rect &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class Box &, DataNode &, DataArray *, int, PropOp);

bool PropSync(float &f, DataNode &node, DataArray *prop, int i, PropOp op);
bool PropSync(unsigned char &uc, DataNode &node, DataArray *prop, int i, PropOp op);
bool PropSync(int &iref, DataNode &node, DataArray *prop, int i, PropOp op);
bool PropSync(short &s, DataNode &node, DataArray *prop, int i, PropOp op);
bool PropSync(bool &b, DataNode &node, DataArray *prop, int i, PropOp op);
bool PropSync(class Symbol &sym, DataNode &node, DataArray *prop, int i, PropOp op);

bool PropSync(DataNode &, DataNode &, DataArray *, int, PropOp);
bool PropSync(class DataNodeObjTrack &, DataNode &, DataArray *, int, PropOp);

template <class T>
bool PropSync(std::vector<T> &vec, DataNode &node, DataArray *prop, int i, PropOp op);

template <class T>
bool PropSync(std::list<T> &pList, DataNode &node, DataArray *prop, int i, PropOp op);

template <class T>
class Key;

template <class T>
bool PropSync(Key<T> &, DataNode &, DataArray *, int, PropOp);

template <class T1, class T2>
class Keys;

template <class T>
bool PropSync(Keys<T, T> &, DataNode &, DataArray *, int, PropOp);

template <class T>
class ObjPtr;

template <class T>
bool PropSync(class ObjPtr<T> &, DataNode &, DataArray *, int, PropOp);

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
class ObjVector;

template <class T>
bool PropSync(ObjVector<T> &objVec, DataNode &node, DataArray *prop, int i, PropOp op);

template <class T>
class ObjList;

template <class T>
bool PropSync(ObjList<T> &objList, DataNode &node, DataArray *prop, int i, PropOp op);

#include "PropSync_p.h"
