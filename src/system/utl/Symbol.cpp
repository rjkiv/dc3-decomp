#include "Symbol.h"

void Symbol::Terminate() {}

void Symbol::PreInit(int, int) {}

Symbol::Symbol(const char *) {}

void Symbol::Init() { PreInit(80000, 560000); }
