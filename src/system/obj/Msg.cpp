#include "obj/Msg.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

Symbol MsgSinks::sCurrentExportEvent(gNullStr);

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

#pragma region EventSink

void MsgSinks::EventSink::Add(
    Hmx::Object *obj, Hmx::Object::SinkMode mode, Symbol handler, bool front
) {
    EventSinkElem elem(sinks.Owner());
    elem.obj = obj;
    elem.mode = mode;
    elem.handler = handler;
    if (front) {
        sinks.push_front(elem);
    } else {
        sinks.push_back(elem);
    }
}

void MsgSinks::EventSink::Remove(Hmx::Object *o, bool b) {
    FOREACH (it, sinks) {
        if (it->obj == o) {
            it->obj = nullptr;
            if (!b) {
                sinks.erase(it);
            }
            return;
        }
    }
}

BEGIN_CUSTOM_PROPSYNC(MsgSinks::EventSinkElem)
    SYNC_PROP(handler, o.handler)
    SYNC_PROP(obj, o.obj)
    SYNC_PROP(mode, (int &)o.mode)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(MsgSinks::EventSink)
    SYNC_PROP(event, o.event)
    SYNC_PROP(sinks, o.sinks)
END_CUSTOM_PROPSYNC

#pragma endregion
#pragma region Sink

void MsgSinks::Sink::Export(DataArray *da) {
    switch (mode) {
    case Hmx::Object::kHandle:
        obj->Handle(da, false);
        break;
    case Hmx::Object::kExport:
        obj->Export(da, false);
        break;
    case Hmx::Object::kType:
        obj->HandleType(da);
        break;
    case Hmx::Object::kExportType:
        obj->Export(da, true);
        break;
    default:
        break;
    }
}

BEGIN_CUSTOM_PROPSYNC(MsgSinks::Sink)
    SYNC_PROP(obj, o.obj)
    SYNC_PROP(mode, (int &)o.mode)
END_CUSTOM_PROPSYNC

#pragma endregion
#pragma region MsgSinks

MsgSinks::MsgSinks(Hmx::Object *owner)
    : mPropertySinks(nullptr), mSinks(owner), mEventSinks(owner), mExporting(0),
      mOwner(owner) {}

MsgSinks::~MsgSinks() {
    if (mPropertySinks) {
        mPropertySinks->Release();
    }
}

bool MsgSinks::Replace(ObjRef *from, Hmx::Object *to) {
    FOREACH (it, mSinks) {
        if (&it->obj == from) {
            mSinks.erase(it);
            return true;
        }
    }
    FOREACH (it, mEventSinks) {
        FOREACH (elem, it->sinks) {
            if (&elem->obj == from) {
                it->sinks.erase(elem);
                return true;
            }
        }
    }
    return false;
}

BEGIN_CUSTOM_PROPSYNC(MsgSinks)
    SYNC_PROP(sinks, o.mSinks)
    SYNC_PROP(event_sinks, o.mEventSinks)
END_PROPSYNCS

Symbol MsgSinks::GetPropSyncHandler(DataArray *arr) {
    if (mPropertySinks) {
        for (int i = 0; i < mPropertySinks->Size(); i += 2) {
            DataArray *array = mPropertySinks->Array(i);
            bool ret;
            if (array->Size() != arr->Size()) {
                for (int j = 0; j < array->Size(); j++) {
                    if (array->UncheckedInt(j) != arr->UncheckedInt(j)) {
                        goto lol;
                    }
                }
                ret = true;
            } else {
            lol:
                ret = false;
            }
            if (ret) {
                return mPropertySinks->Sym(i + 1);
            }
        }
    }
    return 0;
}

bool MsgSinks::HasPropertySink(Hmx::Object *o, DataArray *a) {
    Symbol path = PathToEventName(a);
    if (mPropertySinks) {
        for (int i = 1; i < mPropertySinks->Size(); i += 2) {
            if (path == mPropertySinks->Sym(i)) {
                return true;
            }
        }
    }
    return false;
}

bool MsgSinks::HasSink(Hmx::Object *o) const {
    FOREACH (it, mSinks) {
        if (it->obj == o) {
            return true;
        }
    }
    return false;
}

void MsgSinks::ChainEventSinks(Hmx::Object *from, Hmx::Object *to) {
    FOREACH (it, mEventSinks) {
        if (it->chainProxy) {
            from->AddSink(to, it->event);
        }
    }
}

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
        sink.obj = s;
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
        FOREACH (it, mEventSinks) {
            if (it->event == ev) {
                if (chainProxy != it->chainProxy) {
                    MILO_NOTIFY("%s mismatched proxy chain for %s", PathName(mOwner), ev);
                }
                it->Add(s, mode, handler, mExporting);
                return;
            }
        }
        mEventSinks.push_back();
        mEventSinks.back().event = ev;
        mEventSinks.back().chainProxy = chainProxy;
        mEventSinks.back().Add(s, mode, handler, mExporting);
    }
}

void MsgSinks::AddPropertySink(Hmx::Object *o, DataArray *a, Symbol s) {
    Symbol path = GetPropSyncHandler(a);
    path = PathToEventName(a);
    if (!mPropertySinks) {
        mPropertySinks = new DataArray(2);
    } else {
        mPropertySinks->Resize(mPropertySinks->Size() + 2);
    }
    mPropertySinks->Node(mPropertySinks->Size() - 2) = a->Clone(true, false, 0);
    mPropertySinks->Node(mPropertySinks->Size() - 2).LiteralArray()->Release();
    mPropertySinks->Node(mPropertySinks->Size() - 1) = path;
    AddSink(o, path, s, Hmx::Object::kHandle, false);
}

void MsgSinks::MergeSinks(Hmx::Object *from) {
    if (from->Sinks()) {
        auto &objSinks = from->Sinks()->mSinks;
        FOREACH (it, objSinks) {
            AddSink(it->obj, Symbol(), Symbol(), it->mode);
        }
        auto &objEventSinks = from->Sinks()->mEventSinks;
        FOREACH (sink, objEventSinks) {
            FOREACH (elem, sink->sinks) {
                AddSink(elem->obj, sink->event, elem->handler, elem->mode);
            }
        }
    }
}

void MsgSinks::RemoveSink(Hmx::Object *s, Symbol event) {
    MILO_ASSERT(s, 0x10A);
    FOREACH (it, mSinks) {
        if (it->obj == s) {
            if (!event.Null()) {
                MILO_NOTIFY(
                    "%s: removing global to %s for event %s, all other events will be wiped out",
                    PathName(mOwner),
                    s->Name(),
                    event
                );
            }
            it->obj = nullptr;
            if (mExporting == 0) {
                mSinks.erase(it);
            }
            return;
        }
    }
    if (event.Null()) {
        FOREACH (it, mEventSinks) {
            it->Remove(s, mExporting);
        }
    } else {
        FOREACH (it, mEventSinks) {
            if (it->event == event) {
                it->Remove(s, mExporting);
                return;
            }
        }
    }
}

void MsgSinks::RemovePropertySink(Hmx::Object *o1, DataArray *a2) {
    Symbol path = PathToEventName(a2);
    RemoveSink(o1, path);
    if (mPropertySinks) {
        for (int i = 1; i < mPropertySinks->Size(); i += 2) {
            if (path == mPropertySinks->Sym(i)) {
                mPropertySinks->Remove(i);
                mPropertySinks->Remove(i - 1);
                return;
            }
        }
    }
    MILO_NOTIFY_ONCE(
        "Property Sink not in the list! %s -> %s", PathName(mOwner), PathName(o1)
    );
}

void MsgSinks::Export(DataArray *a) {
    mExporting++;
    Symbol curExport = sCurrentExportEvent;
    for (auto it = mSinks.begin(); it != mSinks.end();) {
        if (it->obj) {
            it->Export(a);
            ++it;
        } else if (mExporting == 1) {
            it = mSinks.erase(it);
        } else {
            ++it;
        }
    }
    sCurrentExportEvent = a->Sym(1);
    FOREACH (it, mEventSinks) {
        if (it->event == a->Sym(1)) {
            DataNode n = a->Node(1);
            for (auto sink = it->sinks.begin(); sink != it->sinks.end();) {
                if (sink->obj) {
                    a->Node(1) = sink->handler;
                    sink->Export(a);
                    ++sink;
                } else if (mExporting == 1) {
                    sink = it->sinks.erase(sink);
                } else {
                    ++sink;
                }
            }
            a->Node(1) = n;
            it->sinks.clear();
            break;
        }
    }
    mExporting--;
    sCurrentExportEvent = curExport;
}

#pragma endregion
