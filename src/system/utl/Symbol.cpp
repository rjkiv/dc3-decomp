#include "Symbol.h"
#include "obj/DataFunc.h"

void Symbol::Terminate() {}

void Symbol::PreInit(int, int) {}

// Symbol::Symbol(const char *) {}

DataNode PrintSymbolTable(DataArray *msg) { return 0; }

void Symbol::Init() {
    PreInit(80000, 560000);
    DataRegisterFunc("print_symbol_table", PrintSymbolTable);
}
