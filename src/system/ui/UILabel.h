#pragma once
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Text.h"
#include "ui/UIComponent.h"
#include "utl/MemMgr.h"

class UILabel : public RndText, public UIComponent, public TextHolder {
public:
    struct LabelStyle {
        LabelStyle(Hmx::Object *);
        int unk0;
        int unk4;
        int unk8;
        int unkc;
        int unk10;
        int unk14;
        int unk18;
        int unk1c;
        int unk20;
        int unk24;
        int unk28;
    };
    // Hmx::Object
    virtual ~UILabel() {}
    OBJ_CLASSNAME(UILabel)
    OBJ_SET_TYPE(UILabel)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndText
    virtual Symbol TextToken();
    virtual void SetCreditsText(DataArray *, class UIListSlot *) {
        MILO_ASSERT(false, 0x50);
    }
    // UIComponent
    virtual void Poll();
    virtual void OldResourcePreload(BinStream &);
    virtual void Highlight();
    // TextHolder
    virtual void SetTextToken(Symbol);
    virtual void SetInt(int, bool);

    OBJ_MEM_OVERLOAD(0x26);

    void SetIcon(char);
    void SetTokenFmt(const DataArray *);
    RndText::Style &Style(int);
    void SetPrelocalizedString(String &);

    template <class T1>
    void SetTokenFmt(Symbol s, T1 t1) {
        SetTokenFmt(DataArrayPtr(s, t1));
    }

    template <class T1, class T2>
    void SetTokenFmt(Symbol s, T1 t1, T2 t2) {
        SetTokenFmt(DataArrayPtr(s, t1, t2));
    }

    template <class T1, class T2, class T3>
    void SetTokenFmt(Symbol s, T1 t1, T2 t2, T3 t3) {
        SetTokenFmt(DataArrayPtr(s, t1, t2, t3));
    }

protected:
    UILabel();

    virtual void SetDisplayText(const char *, bool);

    Symbol unk114; // 0x114
    String unk118; // 0x118
    bool unk120;
    bool unk121;
    bool unk122;
    ObjVector<LabelStyle> unk124; // 0x124
};
