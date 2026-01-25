#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "rndobj/Text.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "ui/UILabelDir.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class UILabel : public RndText, public UIComponent, public TextHolder {
public:
    struct LabelStyle {
        LabelStyle(Hmx::Object *);
        ~LabelStyle();

        ObjPtr<UIColor> mColorOverride; // 0x0
        ObjPtr<UILabelDir> unk14; // 0x14
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
    virtual void Highlight();
    // TextHolder
    virtual void SetTextToken(Symbol);
    virtual void SetInt(int, bool);

    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x26);
    NEW_OBJ(UILabel)

    static void Init();
    static void Terminate();
    static bool sRequireFixedLength;

    void SetFloat(char const *, float);
    void SetDateTime(DateTime const &, Symbol);
    void SetIcon(char);
    void SetTokenFmt(const DataArray *);
    RndText::Style &Style(int);
    void SetPrelocalizedString(String &);
    void SetSubtitle(const DataArray *);
    void SetTimeHMS(int, bool);
    bool CheckValid(bool);
    void SetEditText(char const *);

    char const *GetDefaultText() const;
    void CenterWithLabel(UILabel *, bool, float);
    LabelStyle &LStyle(int) const;

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

    template <class T1, class T2, class T3, class T4>
    void SetTokenFmt(Symbol s, T1 t1, T2 t2, T3 t3, T4 t4) {
        SetTokenFmt(DataArrayPtr(s, t1, t2, t3, t4));
    }

protected:
    virtual void OldResourcePreload(BinStream &);
    virtual void SetDisplayText(const char *, bool);

    UILabel();
    void SetTokenFmtImp(Symbol, DataArray const *, DataArray const *, int, bool);
    DataNode OnSetPrelocalizedString(DataArray const *);
    DataNode OnSetTokenFmt(DataArray const *);
    DataNode OnSetInt(DataArray const *);
    DataNode OnSetTimeHMS(DataArray const *);
    bool AllowEditText() const;
    void LabelUpdate(bool);
    DataNode OnSetHeightFromText(DataArray *);
    void SetFontMat(char const *, int);
    char const *GetFontMat(int);
    void RefreshFontMat(int);

    static bool sDeferUpdate;
    static bool sDebugHighlight;

    Symbol mTextToken; // 0x114
    String unk118; // 0x118
    char unk120;
    bool unk121;
    bool unk122;
    ObjVector<LabelStyle> unk124; // 0x124
};

bool PropSync(UILabel::LabelStyle &, DataNode &, DataArray *, int, PropOp);
