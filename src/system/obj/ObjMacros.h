#pragma once
#include "os/System.h" /* IWYU pragma: keep */
#include "obj/PropSync_p.h" /* IWYU pragma: keep */
#include "obj/MessageTimer.h" /* IWYU pragma: keep */
#include "utl/Symbol.h"

namespace Hmx {
    class Object;
}

/** Get this Object's path name.
 * @param [in] obj The Object.
 * @returns The Object's path name, or "NULL Object" if it doesn't exist.
 */
const char *PathName(const class Hmx::Object *obj);

// BEGIN CLASSNAME MACRO
// -------------------------------------------------------------------------------

#define OBJ_CLASSNAME(classname)                                                         \
    virtual Symbol ClassName() const { return StaticClassName(); }                       \
    static Symbol StaticClassName() {                                                    \
        static Symbol name(#classname);                                                  \
        return name;                                                                     \
    }

// END CLASSNAME MACRO
// ---------------------------------------------------------------------------------

// BEGIN SET TYPE MACRO
// --------------------------------------------------------------------------------
#define OBJ_SET_TYPE(classname)                                                           \
    virtual void SetType(Symbol classname) {                                              \
        static DataArray *types = SystemConfig("objects", StaticClassName(), "types");    \
        if (classname.Null())                                                             \
            SetTypeDef(0);                                                                \
        else {                                                                            \
            DataArray *found = types->FindArray(classname, false);                        \
            if (found != 0)                                                               \
                SetTypeDef(found);                                                        \
            else {                                                                        \
                MILO_WARN(                                                                \
                    "%s:%s couldn't find type %s", ClassName(), PathName(this), classname \
                );                                                                        \
                SetTypeDef(0);                                                            \
            }                                                                             \
        }                                                                                 \
    }

// END SET TYPE MACRO
// ----------------------------------------------------------------------------------

// BEGIN HANDLE MACROS
// ---------------------------------------------------------------------------------

#define BEGIN_HANDLERS(objType)                                                          \
    DataNode objType::Handle(DataArray *_msg, bool _warn) {                              \
        Symbol sym = _msg->Sym(1);                                                       \
        MessageTimer timer(                                                              \
            (MessageTimer::Active()) ? static_cast<Hmx::Object *>(this) : 0, sym         \
        );

#define _NEW_STATIC_SYMBOL(str) static Symbol _s(#str);

#define _HANDLE_CHECKED(expr)                                                            \
    {                                                                                    \
        DataNode result = expr;                                                          \
        if (result.Type() != kDataUnhandled)                                             \
            return result;                                                               \
    }

#define HANDLE(s, func)                                                                  \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            _HANDLE_CHECKED(func(_msg))                                                  \
    }

#define HANDLE_EXPR(s, expr)                                                             \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            return expr;                                                                 \
    }

#define HANDLE_ACTION(symbol, action)                                                    \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            /* for style, require any side-actions to be performed via comma operator */ \
            (action);                                                                    \
            return 0;                                                                    \
        }                                                                                \
    }

#define HANDLE_ARRAY(array)                                                              \
    {                                                                                    \
        /* this needs to be placed up here to match Hmx::Object::Handle */               \
        DataArray *found;                                                                \
        if (array && (found = array->FindArray(sym, false))) {                           \
            _HANDLE_CHECKED(found->ExecuteScript(1, this, _msg, 2))                      \
        }                                                                                \
    }

#define HANDLE_FORWARD(func) _HANDLE_CHECKED(func(_msg, false))

#define HANDLE_SUPERCLASS(parent) HANDLE_FORWARD(parent::Handle)

#define END_HANDLERS                                                                     \
    if (_warn)                                                                           \
        MILO_WARN("%s unhandled msg: %s", PathName(this), sym);                          \
    return DataNode(kDataUnhandled, 0);                                                  \
    }

// BEGIN SYNCPROPERTY MACROS
// ---------------------------------------------------------------------------

#define BEGIN_PROPSYNCS(objType)                                                         \
    bool objType::SyncProperty(DataNode &_val, DataArray *_prop, int _i, PropOp _op) {   \
        if (_i == _prop->Size())                                                         \
            return true;                                                                 \
        else {                                                                           \
            Symbol sym = _prop->Sym(_i);

#define BEGIN_CUSTOM_PROPSYNC(objType)                                                   \
    bool PropSync(objType &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op) {    \
        if (_i == _prop->Size())                                                         \
            return true;                                                                 \
        else {                                                                           \
            Symbol sym = _prop->Sym(_i);

#define SYNC_PROP(s, member)                                                             \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s)                                                                   \
            return PropSync(member, _val, _prop, _i + 1, _op);                           \
    }

// for propsyncs that do something extra if the prop op is specifically kPropSet
#define SYNC_PROP_SET(s, member, func)                                                   \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (_op == kPropSet) {                                                       \
                func;                                                                    \
            } else {                                                                     \
                if (_op == (PropOp)0x40)                                                 \
                    return false;                                                        \
                _val = DataNode(member);                                                 \
            }                                                                            \
            return true;                                                                 \
        }                                                                                \
    }

// for propsyncs that do NOT use size or get - aka, any combo of set, insert, remove, and
// handle is used
#define SYNC_PROP_MODIFY(symbol, member, func)                                           \
    if (sym == symbol) {                                                                 \
        bool synced = PropSync(member, _val, _prop, _i + 1, _op);                        \
        if (!synced)                                                                     \
            return false;                                                                \
        else {                                                                           \
            if (!(_op & (kPropSize | kPropGet))) {                                       \
                func;                                                                    \
            }                                                                            \
            return true;                                                                 \
        }                                                                                \
    }

// for SYNC_PROP_MODIFY uses where the condition order is flipped
// if you know how to make this macro and SYNC_PROP_MODIFY into one singular macro,
// while still matching every instance of SYNC_PROP_MODIFY being used regardless of
// condition order, by all means please do so, because idk how to do it here
#define SYNC_PROP_MODIFY_ALT(symbol, member, func)                                       \
    if (sym == symbol) {                                                                 \
        bool synced = PropSync(member, _val, _prop, _i + 1, _op);                        \
        if (synced) {                                                                    \
            if (!(_op & (kPropSize | kPropGet))) {                                       \
                func;                                                                    \
            }                                                                            \
            return true;                                                                 \
        } else                                                                           \
            return false;                                                                \
    }

#define SYNC_PROP_BITFIELD(symbol, mask_member, line_num)                                \
    if (sym == symbol) {                                                                 \
        _i++;                                                                            \
        if (_i < _prop->Size()) {                                                        \
            DataNode &node = _prop->Node(_i);                                            \
            int res = 0;                                                                 \
            switch (node.Type()) {                                                       \
            case kDataInt:                                                               \
                res = node.Int();                                                        \
                break;                                                                   \
            case kDataSymbol: {                                                          \
                const char *bitstr = node.Sym().Str();                                   \
                MILO_ASSERT_FMT(                                                         \
                    strncmp("BIT_", bitstr, 4) == 0,                                     \
                    "%s does not begin with BIT_",                                       \
                    bitstr                                                               \
                );                                                                       \
                Symbol bitsym(bitstr + 4);                                               \
                const Symbol &test = Symbol(bitsym);                                     \
                DataArray *macro = DataGetMacro(test);                                   \
                MILO_ASSERT_FMT(                                                         \
                    macro, "PROPERTY_BITFIELD %s could not find macro %s", symbol, test  \
                );                                                                       \
                res = macro->Int(0);                                                     \
                break;                                                                   \
            }                                                                            \
            default:                                                                     \
                MILO_ASSERT(0, line_num);                                                 \
                break;                                                                   \
            }                                                                            \
            MILO_ASSERT(_op <= kPropInsert, line_num);                                      \
            if (_op == kPropGet) {                                                       \
                int final = mask_member & res;                                           \
                _val = DataNode(final > 0);                                              \
            } else {                                                                     \
                if (_val.Int() != 0)                                                     \
                    mask_member |= res;                                                  \
                else                                                                     \
                    mask_member &= ~res;                                                 \
            }                                                                            \
            return true;                                                                 \
        } else                                                                           \
            return PropSync(mask_member, _val, _prop, _i, _op);                          \
    }

#define SYNC_PROP_MODIFY_STATIC(symbol, member, func)                                    \
    { _NEW_STATIC_SYMBOL(symbol) SYNC_PROP_MODIFY(_s, member, func) }

#define SYNC_PROP_BITFIELD_STATIC(symbol, mask_member, line_num)                         \
    { _NEW_STATIC_SYMBOL(symbol) SYNC_PROP_BITFIELD(_s, mask_member, line_num) }

#define SYNC_SUPERCLASS(parent)                                                          \
    if (parent::SyncProperty(_val, _prop, _i, _op))                                      \
        return true;

#define END_PROPSYNCS                                                                    \
    return false;                                                                        \
    }                                                                                    \
    }

#define END_CUSTOM_PROPSYNC                                                              \
    return false;                                                                        \
    }                                                                                    \
    }

// END SYNCPROPERTY MACROS
// -----------------------------------------------------------------------------
