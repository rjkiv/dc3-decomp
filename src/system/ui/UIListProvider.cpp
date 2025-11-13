#include "ui/UIListProvider.h"
#include "UIListLabel.h"
#include "UIListProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "ui/UIList.h"
#include "ui/UIListMesh.h"
#include "utl/Loader.h"
#include "utl/Locale.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region UIListProvider

BEGIN_CUSTOM_HANDLERS(UIListProvider)
    HANDLE_EXPR(num_data, NumData())
    HANDLE_EXPR(data_index, DataIndex(_msg->Sym(2)))
    HANDLE_EXPR(data_symbol, DataSymbol(_msg->Int(2)))
END_CUSTOM_HANDLERS

RndMat *UIListProvider::Mat(int, int, UIListMesh *mesh) const {
    return mesh->DefaultMat();
}

void UIListProvider::Text(int, int, UIListLabel *listlabel, UILabel *label) const {
    if (TheLoadMgr.EditMode()) {
        label->SetEditText(listlabel->GetDefaultText());
    } else
        label->SetTextToken(gNullStr);
}

void UIListProvider::UpdateExtendedText(int, int, UILabel *label) const {
    if (!TheLoadMgr.EditMode()) {
        MILO_NOTIFY(
            "Trying to update extended text without an override provider method. Label = %s",
            label->Name()
        );
        label->SetTextToken(gNullStr);
    }
}

void UIListProvider::UpdateExtendedMesh(int, int, RndMesh *mesh) const {
    if (!TheLoadMgr.EditMode()) {
        MILO_NOTIFY(
            "Trying to update extended mesh without an override provider method. Mesh = %s",
            mesh->Name()
        );
        mesh->SetMat(0);
    }
}

void UIListProvider::UpdateExtendedCustom(int, int, Hmx::Object *obj) const {
    if (!TheLoadMgr.EditMode())
        MILO_NOTIFY(
            "Trying to update extended custom object without an override provider method. object = %s",
            obj->Name()
        );
}

#pragma endregion UIListProvider
#pragma region DataProvider

void DataProvider::Text(int i, int j, UIListLabel *listlabel, UILabel *label) const {}

float DataProvider::GapSize(int, int i, int, int) const {
    if (mFluidWidth)
        return mWidths[i];
    else
        return 0.0f;
}

void DataProvider::Enable(Symbol sym) {
    stlpmtx_std::list<Symbol>::iterator it =
        std::find(mDisabled.begin(), mDisabled.end(), sym);
    if (it != mDisabled.end())
        mDisabled.erase(it);
}

void DataProvider::UnDim(Symbol sym) {
    stlpmtx_std::list<Symbol>::iterator it =
        std::find(mDimmed.begin(), mDimmed.end(), sym);
    if (it != mDimmed.end())
        mDimmed.erase(it);
}

RndMat *DataProvider::Mat(int i, int j, UIListMesh *mesh) const {
    if (!mList)
        return mesh->DefaultMat();
    static Message msgMat("mat", DataNode(0));
    msgMat[0] = DataNode(j);
    DataNode handled = mList->HandleType(msgMat);
    if (handled.Type() == kDataUnhandled) {
        return mesh->DefaultMat();
    } else {
        ObjectDir *pDir = mList->GetUIListDir();
        MILO_ASSERT(pDir, 0xae);
        RndMat *ret = pDir->Find<RndMat>(handled.Str(), false);
        if (!ret)
            return mesh->DefaultMat();
        return ret;
    }
}

void DataProvider::Disable(Symbol sym) {
    stlpmtx_std::list<Symbol>::iterator it =
        std::find(mDisabled.begin(), mDisabled.end(), sym);
    if (it == mDisabled.end())
        mDisabled.push_back(sym);
}

void DataProvider::Dim(Symbol sym) {
    stlpmtx_std::list<Symbol>::iterator it =
        std::find(mDimmed.begin(), mDimmed.end(), sym);
    if (it == mDimmed.end())
        mDimmed.push_back(sym);
}

void DataProvider::SetData(DataArray *data) {
    MILO_ASSERT(data, 0x6a);
    data->AddRef();
    if (mData) {
        mData->Release();
        mData = 0;
    }
    mData = data;

    if (mFluidWidth) {
        mWidths.clear();
        mWidths.resize(NumData());
    }
    mDisabled.clear();
    mDimmed.clear();
}

#pragma endregion DataProvider
