#ifndef _DECAF_PRINTER_H_
#define _DECAF_PRINTER_H_
#include "antlr4-runtime.h"
#include "error.h"
#include "Pos.h"
#include <string>

std::string reportErrorText(CompileErrors err,
                            const std::vector<std::string> &texts);
std::string reportErrorText(const Pos &pos, CompileErrors err,
                            const std::vector<std::string> &texts);

#endif