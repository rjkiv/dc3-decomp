#pragma once
#include "PropSync.h"
#include "math/Key.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "utl/Std.h"

// DO NOT try to include this header directly!
// include obj/PropSync.h instead

inline bool PropSync(float &f, DataNode &node, DataArray *prop, int i, PropOp op) {
    MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x2F);
    if (op == kPropGet)
        node = f;
    else
        f = node.Float();
    return true;
}

inline bool PropSync(DataNode &obj, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else {
        MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x19);
        if (op == kPropGet) {
            node = obj;
        } else {
            obj = node;
        }
        return true;
    }
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

inline bool
PropSync(DataNodeObjTrack &objTrack, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else {
        MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x25);
        // lol, yet another circular dependency moment
        // DataNodeObjTrack is in Object.h,
        // which depends on PropSync.h for the PropOp enum
        if (op == kPropGet) {
            // node = objTrack.Node();
        } else {
            // objTrack = node;
        }
        return true;
    }
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

template <class T>
bool PropSync(std::vector<T> &vec, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0xC9);
        node = (int)vec.size();
        return true;
    } else {
        int idx = prop->Int(i++);
        if (idx >= vec.size() + (op == kPropInsert))
            return false;
        else {
            typename std::vector<T>::iterator it = vec.begin() + idx;
            if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
                return PropSync(*it, node, prop, i, op);
            } else if (op == kPropRemove) {
                vec.erase(it);
                return true;
            } else if (op == kPropInsert) {
                T item;
                if (PropSync(item, node, prop, i, op)) {
                    vec.insert(it, item);
                    return true;
                }
            }
        }
        return false;
    }
}

template <class T>
bool PropSync(std::list<T> &pList, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0x9E);
        node = (int)pList.size();
        return true;
    } else {
        int idx = prop->Int(i++);
        if (idx >= pList.size() + (op == kPropInsert))
            return false;
        else {
            typename std::list<T>::iterator it = pList.begin();
            while (idx-- > 0)
                it++;
            if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
                return PropSync(*it, node, prop, i, op);
            } else if (op == kPropRemove) {
                pList.erase(it);
                return true;
            } else if (op == kPropInsert) {
                T item;
                if (PropSync(item, node, prop, i, op)) {
                    pList.insert(it, item);
                    return true;
                }
            }
        }
        return false;
    }
}

template <class T>
bool PropSync(Key<T> &key, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        return true;
    } else {
        Symbol sym = prop->Sym(i++);
        {
            static Symbol frame("frame");
            if (sym == frame)
                return PropSync(key.frame, node, prop, i, op);
        }
        {
            static Symbol value("value");
            if (sym == value)
                return PropSync(key.value, node, prop, i, op);
        }
    }
    return false;
}

template <class T>
bool PropSync(Keys<T, T> &keys, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0x10A);
        node = keys.NumKeys();
        return true;
    } else {
        typename Keys<T, T>::iterator it = keys.begin() + prop->Int(i++);
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            bool ret = PropSync(*it, node, prop, i, op);
            if (op & kPropSet) {
                std::sort(keys.begin(), keys.end());
            }
            return ret;
        } else if (op == kPropRemove) {
            keys.erase(it);
            return true;
        } else if (op == kPropInsert) {
            Key<T> key;
            if (PropSync(key, node, prop, i, kPropInsert)) {
                keys.insert(it, key);
                std::sort(keys.begin(), keys.end());
                return true;
            }
        }
        return false;
    }
}

template <class T>
bool PropSync(T *&obj, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else {
        MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x66);
        if (op == kPropGet)
            node = obj;
        else
            obj = node.Obj<T>();
        return true;
    }
}

template <class T>
bool PropSync(ObjPtr<T> &ptr, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else {
        MILO_ASSERT(i == prop->Size() && op <= kPropInsert, 0x133);
        if (op == kPropGet)
            node = ptr.Ptr();
        else
            ptr = dynamic_cast<T *>(node.GetObj());
        return true;
    }
}

template <class T>
bool PropSync(ObjOwnerPtr<T> &ptr, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else {
        MILO_ASSERT(op <= kPropInsert, 0x140);
        if (op == kPropGet)
            node = ptr.Ptr();
        else
            ptr = dynamic_cast<T *>(node.GetObj());
        return true;
    }
}

template <class T>
bool PropSync(
    ObjPtrList<T, ObjectDir> &ptr, DataNode &node, DataArray *prop, int i, PropOp op
) {
    if (op == kPropUnknown0x40)
        return ptr.Mode() == kObjListNoNull;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0x154);
        node = ptr.size();
        return true;
    } else {
        typename ObjPtrList<T, ObjectDir>::iterator it = ptr.begin();
        for (int cnt = prop->Int(i++); cnt > 0; cnt--)
            it++;
        MILO_ASSERT(i == prop->Size(), 0x15E);
        switch (op) {
        case kPropGet: {
            T *item = *it;
            return PropSync(item, node, prop, i, op);
        }
        case kPropSet: {
            T *objToSet = nullptr;
            if (PropSync(objToSet, node, prop, i, op)) {
                ptr.Set(it, objToSet);
                return true;
            }
            break;
        }
        case kPropRemove: {
            ptr.erase(it);
            return true;
        }
        case kPropInsert: {
            T *objToInsert = 0;
            if (PropSync(objToInsert, node, prop, i, op)) {
                ptr.insert(it, objToInsert);
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }
}

template <class T>
bool PropSync(ObjPtrVec<T, ObjectDir> &, DataNode &, DataArray *, int, PropOp);

template <class T>
bool PropSync(ObjVector<T> &objVec, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0x18B);
        node = (int)objVec.size();
        return true;
    } else {
        typename ObjVector<T>::iterator it = objVec.begin() + prop->Int(i++);
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            return PropSync(*it, node, prop, i, op);
        } else if (op == kPropRemove) {
            objVec.erase(it);
            return true;
        } else if (op == kPropInsert) {
            T item(objVec.Owner());
            if (PropSync(item, node, prop, i, op)) {
                objVec.insert(it, item);
                return true;
            }
        }
        return false;
    }
}

template <class T>
bool PropSync(ObjList<T> &objList, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize || op == kPropInsert, 0x1B2);
        node = (int)objList.size();
        return true;
    } else {
        typename std::list<T>::iterator it = NextItr(objList.begin(), prop->Int(i++));
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            return PropSync(*it, node, prop, i, op);
        } else if (op == kPropRemove) {
            objList.erase(it);
            return true;
        } else if (op == kPropInsert) {
            T item(objList.Owner());
            if (PropSync(item, node, prop, i, kPropInsert)) {
                objList.insert(it, item);
                return true;
            }
        }
        return false;
    }
}
