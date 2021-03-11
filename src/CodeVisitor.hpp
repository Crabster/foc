#include <antlr4-runtime.h>
#include "FocLexer.h"
#include "FocParser.h"
#include "FocBaseVisitor.h"

namespace foc {

class CodeVisitor : FocBaseVisitor {
public:
    antlrcpp::Any visitProgram(FocParser::ProgramContext *ctx);
    antlrcpp::Any visitDecls(FocParser::DeclsContext *ctx);
    antlrcpp::Any visitVarDecl(FocParser::VarDeclContext *ctx);
    antlrcpp::Any visitFunDecl(FocParser::FunDeclContext *ctx);
};

} 
