#pragma once
#include "obj/Data.h"
#include "obj/Object.h"

/** A DataArray container to send to other objects for handling. */
class Message {
public:
    // Message(); // if there IS a void ctor for Msg i can't find it

    Message(Symbol type) {
        mData = new DataArray(2);
        mData->Node(1) = type;
    }

    Message(Symbol type, const DataNode &arg1) {
        mData = new DataArray(3);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
    }

    Message(Symbol type, const DataNode &arg1, const DataNode &arg2) {
        mData = new DataArray(4);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
    }

    Message(
        Symbol type, const DataNode &arg1, const DataNode &arg2, const DataNode &arg3
    ) {
        mData = new DataArray(5);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4
    ) {
        mData = new DataArray(6);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4,
        const DataNode &arg5
    ) {
        mData = new DataArray(7);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
        mData->Node(6) = arg5;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4,
        const DataNode &arg5,
        const DataNode &arg6
    ) {
        mData = new DataArray(8);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
        mData->Node(6) = arg5;
        mData->Node(7) = arg6;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4,
        const DataNode &arg5,
        const DataNode &arg6,
        const DataNode &arg7
    ) {
        mData = new DataArray(9);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
        mData->Node(6) = arg5;
        mData->Node(7) = arg6;
        mData->Node(8) = arg7;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4,
        const DataNode &arg5,
        const DataNode &arg6,
        const DataNode &arg7,
        const DataNode &arg8
    ) {
        mData = new DataArray(10);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
        mData->Node(6) = arg5;
        mData->Node(7) = arg6;
        mData->Node(8) = arg7;
        mData->Node(9) = arg8;
    }

    Message(
        Symbol type,
        const DataNode &arg1,
        const DataNode &arg2,
        const DataNode &arg3,
        const DataNode &arg4,
        const DataNode &arg5,
        const DataNode &arg6,
        const DataNode &arg7,
        const DataNode &arg8,
        const DataNode &arg9
    ) {
        mData = new DataArray(11);
        mData->Node(1) = type;
        mData->Node(2) = arg1;
        mData->Node(3) = arg2;
        mData->Node(4) = arg3;
        mData->Node(5) = arg4;
        mData->Node(6) = arg5;
        mData->Node(7) = arg6;
        mData->Node(8) = arg7;
        mData->Node(9) = arg8;
        mData->Node(10) = arg9;
    }

    Message(DataArray *da) : mData(da) { da->AddRef(); }

    Message(int i) { mData = new DataArray(i + 2); }

    virtual ~Message() { mData->Release(); }

    DataArray *mData; // 0x0

    operator DataArray *() const { return mData; }
    DataArray *operator->() const { return mData; }
    DataArray *Data() const { return mData; }

    void SetType(Symbol type) { mData->Node(1) = type; }

    Symbol Type() const { return mData->Sym(1); }

    DataNode &operator[](int idx) { return mData->Node(idx + 2); }
};

#define DECLARE_MESSAGE(classname, type)                                                 \
    class classname : public Message {                                                   \
    public:                                                                              \
        classname(DataArray *da) : Message(da) {}                                        \
        virtual ~classname() {}                                                          \
        static Symbol Type() {                                                           \
            static Symbol t(type);                                                       \
            return t;                                                                    \
        }

#define END_MESSAGE                                                                      \
    }                                                                                    \
    ;

#include "obj/Object.h"
#include "utl/MemMgr.h"

class ObjRef;

class MsgSinks {
public:
    struct Sink {
        Sink(Hmx::Object *owner) : obj(owner, nullptr) {}

        Sink &operator=(const Sink &s) {
            obj.SetObjConcrete(s.obj);
            mode = s.mode;
            return *this;
        }

        ObjOwnerPtr<Hmx::Object> obj; // 0x0
        Hmx::Object::SinkMode mode; // 0x14
    };
    struct EventSinkElem : public Sink {
        EventSinkElem(Hmx::Object *owner) : Sink(owner) {}
        EventSinkElem &operator=(const EventSinkElem &);

        Symbol handler; // 0x18
    };
    struct EventSink {
        EventSink(Hmx::Object *owner) : sinks(owner) {}
        void Add(Hmx::Object *, Hmx::Object::SinkMode, Symbol, bool);
        void Remove(Hmx::Object *, bool);

        Symbol event; // 0x0
        bool chainProxy; // 0x4
        ObjList<EventSinkElem> sinks; // 0x8
    };

    MsgSinks(Hmx::Object *);
    ~MsgSinks();
    bool Replace(ObjRef *, Hmx::Object *);
    void RemovePropertySink(Hmx::Object *, DataArray *);
    bool HasPropertySink(Hmx::Object *, DataArray *);
    void RemoveSink(Hmx::Object *, Symbol);
    void AddSink(
        Hmx::Object *,
        Symbol,
        Symbol = Symbol(),
        Hmx::Object::SinkMode = Hmx::Object::kHandle,
        bool = true
    );
    void AddPropertySink(Hmx::Object *, DataArray *, Symbol);
    void MergeSinks(Hmx::Object *);
    Symbol GetPropSyncHandler(DataArray *);
    void Export(DataArray *);
    bool HasSink(Hmx::Object *) const;
    void ChainEventSinks(Hmx::Object *, Hmx::Object *);

    ObjList<Sink> &Sinks() { return mSinks; }

    MEM_OVERLOAD(MsgSinks, 0xAF);

private:
    DataArray *unk0;
    ObjList<Sink> mSinks; // 0x4
    ObjList<EventSink> mEventSinks; // 0x10
    int mExporting; // 0x1c
    Hmx::Object *mOwner; // 0x20

    static Symbol sCurrentExportEvent;
};

#include "obj/PropSync_p.h"
bool PropSync(MsgSinks &, DataNode &, DataArray *, int, PropOp);
