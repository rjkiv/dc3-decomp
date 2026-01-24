#include "char/CharTaskMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"

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
