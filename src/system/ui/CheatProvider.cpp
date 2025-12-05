#include "ui/CheatProvider.h"
#include "obj/DataUtl.h"
#include "os/Debug.h"
#include "os/System.h"
// #include "ui/UIListLabel.h"
#include "ui/UIListLabel.h"
#include "utl/Cheats.h"
#include "utl/Symbol.h"

CheatProvider *CheatProvider::sInstance;

#pragma region CheatProvider

CheatProvider::CheatProvider() : mFilterIdx(0) {
    SetName("cheat_provider", ObjectDir::Main());
    mFilters.push_back("all");
    DataArray *cfg = SystemConfig("quick_cheats");
    for (int i = 1; i < cfg->Size(); i++) {
        DataArray *arr = cfg->Array(i);
        Symbol cheatTypeSym = arr->Sym(0);
        const char *strtext = 0;
        if (cheatTypeSym == "keyboard")
            strtext = "KEYBOARD CHEATS";
        else if (cheatTypeSym == "right")
            strtext = "RIGHT CHEATS (R1 + R2)";
        else if (cheatTypeSym == "left")
            strtext = "LEFT CHEATS (L1 + L2)";
        mCheats.push_back(Cheat(strtext));
        for (int j = 1; j < arr->Size(); j++) {
            DataArray *arr2 = arr->Array(j);
            DataType nType = arr2->Node(0).Type();
            String theKeyStr;
            if (nType == kDataString || nType == kDataSymbol) {
                theKeyStr = arr2->Str(0);
            } else {
                const char *cheatStrStart = cheatTypeSym == "keyboard" ? "KB_" : "kPad_";
                theKeyStr = DataGetMacroByInt(arr2->Int(0), cheatStrStart).Str()
                    + strlen(cheatStrStart);
            }
            String theConcattedStr;
            int theNodeIdx = 1;
            while (arr2->Type(theNodeIdx) == kDataSymbol) {
                theKeyStr = String(arr2->Sym(theNodeIdx)) + String(" ") + theKeyStr;
                theNodeIdx++;
            }
            theConcattedStr = arr2->Str(theNodeIdx);
            mCheats.push_back(Cheat(theKeyStr, theConcattedStr, arr2));
            static Symbol filters("filters");
            DataArray *filterArr = arr2->FindArray(filters, false);
            if (filterArr) {
                for (int k = 1; k < filterArr->Size(); k++) {
                    Symbol curFilt = filterArr->Sym(k);
                    if (std::find(mFilters.begin(), mFilters.end(), curFilt)
                        == mFilters.end()) {
                        mFilters.push_back(curFilt);
                    }
                }
            }
        }
        if (i < cfg->Size() - 1) {
            mCheats.push_back(gNullStr);
        }
    }
    ApplyFilter();
}

void CheatProvider::Init() {
    if (CheatsInitialized()) {
        MILO_ASSERT(!sInstance, 0x60);
        sInstance = new CheatProvider();
    }
}

void CheatProvider::Terminate() {
    MILO_ASSERT(sInstance, 0x66);
    RELEASE(sInstance);
}

void CheatProvider::InitData(RndDir *) { ApplyFilter(); }

int CheatProvider::NumData() const { return mFilterCheats.size(); }

bool CheatProvider::IsActive(int i) const { return !mFilterCheats[i].mKey.empty(); }

void CheatProvider::Text(int i, int j, UIListLabel *listlabel, UILabel *label) const {
    const Cheat &cheat = mFilterCheats[j];

    if (listlabel->Matches("key")) {
        label->SetEditText(cheat.mKey.c_str());
    } else if (listlabel->Matches("description")) {
        label->SetEditText(cheat.mDesc.c_str());
    } else if (listlabel->Matches("value")) {
        static Symbol value_bool("value_bool");
        static Symbol value("value");
        if (!cheat.mScript)
            label->SetTextToken(gNullStr);
        else {
            DataArray *bool_arr = cheat.mScript->FindArray(value_bool, false);
            if (bool_arr) {
                label->SetEditText(bool_arr->Int(1) != 0 ? "ON" : "OFF");
            } else {
                DataArray *value_arr = cheat.mScript->FindArray(value, false);
                if (value_arr) {
                    const DataNode &n = value_arr->Node(1).Evaluate();
                    if (n.Type() == kDataSymbol || n.Type() == kDataString) {
                        label->SetEditText(n.Str());
                    } else if (n.Type() == kDataInt) {
                        label->SetInt(n.Int(), false);
                    } else if (n.Type() == kDataFloat) {
                        label->SetFloat("%f", n.Float());
                    } else
                        label->SetEditText("?");
                } else
                    label->SetEditText(gNullStr);
            }
        }
    }
}

BEGIN_HANDLERS(CheatProvider)
    HANDLE_ACTION(
        invoke,
        CallQuickCheat(mFilterCheats[_msg->Int(2)].mScript, _msg->Obj<LocalUser>(3))
    )
    HANDLE_ACTION(next_filter, ApplyFilter())
    HANDLE_EXPR(filter, mFilters[mFilterIdx])
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion CheatProvider
#pragma region CheatProvider::Cheat

CheatProvider::Cheat::Cheat(const char *desc) : mKey(), mDesc(desc), mScript(0) {}

CheatProvider::Cheat::Cheat(String &key, String &desc, DataArray *script)
    : mKey(key), mDesc(desc), mScript(script) {}

#pragma endregion CheatProvider::Cheat
