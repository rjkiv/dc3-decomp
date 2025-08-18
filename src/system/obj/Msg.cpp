#include "obj/Msg.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

Symbol MsgSinks::sCurrentExportEvent(gNullStr);

Symbol MsgSinks::GetPropSyncHandler(DataArray *arr) {
    if (unk0) {
        for (int i = 0; i < unk0->Size(); i += 2) {
            DataArray *array = unk0->Array(i);
            if (array->Size() == arr->Size()) {
                bool ret = true;
                for (int j = 0; j < array->Size(); j++) {
                    if (array->UncheckedInt(j) != arr->UncheckedInt(j)) {
                        ret = false;
                        break;
                    }
                }
                if (ret)
                    return unk0->Sym(0);
            }
        }
    }
    return 0;
}

Symbol PathToEventName(DataArray *arr) {
    StackString<100> str("on_");
    str += arr->Sym(0).Str();
    for (int i = 1; i < arr->Size(); i++) {
        if (arr->Type(i) == kDataSymbol) {
            str += arr->LiteralSym(i).Str();
        } else {
            str += MakeString("%i", arr->Int(i));
        }
    }
    str += "_change";
    return str.c_str();
}

bool MsgSinks::HasPropertySink(Hmx::Object *o, DataArray *a) {
    Symbol path = PathToEventName(a);
    if (unk0) {
        for (int i = 1; i < unk0->Size(); i += 2) {
            if (path == a->Sym(i)) {
                return true;
            }
        }
    }
    return false;
}

bool MsgSinks::HasSink(Hmx::Object *o) const {
    for (ObjList<Sink>::const_iterator it = mSinks.begin(); it != mSinks.end(); ++it) {
        if (it->obj == o) {
            return true;
        }
    }
    return false;
}

void MsgSinks::ChainEventSinks(Hmx::Object *from, Hmx::Object *to) {
    for (ObjList<EventSink>::const_iterator it = mEventSinks.begin();
         it != mEventSinks.end();
         ++it) {
        if (it->chainProxy) {
            from->AddSink(to, it->event);
        }
    }
}

void MsgSinks::EventSink::Remove(Hmx::Object *o, bool b) {
    for (ObjList<EventSinkElem>::iterator it = sinks.begin(); it != sinks.end(); ++it) {
        if (it->obj == o) {
            it->obj->Release(nullptr);
            if (!b) {
                sinks.erase(it);
            }
            return;
        }
    }
}

void MsgSinks::EventSink::Add(
    Hmx::Object *obj, Hmx::Object::SinkMode mode, Symbol s, bool b4
) {
    EventSinkElem elem(sinks.Owner());
    elem.obj.SetObjConcrete(obj);
    elem.mode = mode;
    elem.handler = s;
    if (b4) {
        sinks.push_front();
        sinks.front() = elem;
    } else {
        sinks.push_back(elem);
    }
}

MsgSinks::~MsgSinks() {
    if (unk0)
        unk0->Release();
}

MsgSinks::MsgSinks(Hmx::Object *o)
    : unk0(nullptr), mSinks(o), mEventSinks(o), mExporting(0), mOwner(o) {}

// BEGIN_CUSTOM_PROPSYNC(MsgSinks::Sink)
//     SYNC_PROP(obj, (Hmx::Object *&)o.obj)
//     SYNC_PROP(mode, (int &)o.mode)
// END_CUSTOM_PROPSYNC

// BEGIN_CUSTOM_PROPSYNC(MsgSinks)
//     SYNC_PROP(sinks, o.mSinks)
//     SYNC_PROP(event_sinks, o.mEventSinks)
// END_PROPSYNCS

void MsgSinks::AddSink(
    Hmx::Object *s, Symbol ev, Symbol handler, Hmx::Object::SinkMode mode, bool chainProxy
) {
    MILO_ASSERT(s, 0x9C);
    if (ev.Null() && !chainProxy) {
        MILO_NOTIFY("%s can't have chainProxy false with no event", PathName(mOwner));
    }
    RemoveSink(s, ev);
    if (ev.Null()) {
        MILO_ASSERT(s != mOwner, 0xA6);
        Sink sink(mOwner);
        sink.obj.SetObjConcrete(s);
        sink.mode = mode;
        if (mExporting != 0) {
            mSinks.push_front(sink);
        } else {
            mSinks.push_back(sink);
        }
    } else {
        if (handler.Null())
            handler = ev;
        MILO_ASSERT((s != mOwner) || (handler != ev), 0xB9);
        for (ObjList<EventSink>::iterator it = mEventSinks.begin();
             it != mEventSinks.end();
             ++it) {
            if (it->event == ev) {
                if (chainProxy != it->chainProxy) {
                    MILO_NOTIFY("%s mismatched proxy chain for %s", PathName(mOwner), ev);
                    mEventSinks.back().Add(s, mode, handler, mExporting);
                    return;
                }
            }
        }
        mEventSinks.push_back();
        mEventSinks.back().event = ev;
        mEventSinks.back().chainProxy = chainProxy;
        mEventSinks.back().Add(s, mode, handler, mExporting);
    }
}

void MsgSinks::AddPropertySink(Hmx::Object *o, DataArray *a, Symbol s) {
    Symbol handler = GetPropSyncHandler(a);
    Symbol path = PathToEventName(a);
    if (!unk0) {
        unk0 = new DataArray(2);
    } else {
        unk0->Resize(unk0->Size() + 2);
    }
    unk0->Node(unk0->Size() - 2) = DataNode(a->Clone(true, false, 0), kDataArray);
    unk0->Node(unk0->Size() - 2).LiteralArray()->Release();
    unk0->Node(unk0->Size() - 1) = path;
    AddSink(o, path, s, Hmx::Object::kHandle, false);
}
