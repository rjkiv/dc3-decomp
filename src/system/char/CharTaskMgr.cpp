#include "char/CharTaskMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "utl/MakeString.h"

// Force template instantiation for MakeString<int, float, float, float>
template const char *
MakeString<int, float, float, float>(const char *, const int &, const float &, const float &, const float &);

bool CharTaskMgr::sShowGraph = false;
int CharTaskMgr::sNumInstances;
float CharTaskMgr::sGraphPosY;

namespace {
    static DataNode OnToggleCharTaskGraph(DataArray *arr) {
        CharTaskMgr::sShowGraph = !CharTaskMgr::sShowGraph;
        return DataNode(CharTaskMgr::sShowGraph);
    }
}

void CharTaskMgr::Init() {
    DataRegisterFunc("toggle_char_task_graph", OnToggleCharTaskGraph);
}
