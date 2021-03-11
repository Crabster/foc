#include <antlr4-runtime.h>
#include "CodeVisitor.hpp"

namespace foc {

antlrcpp::Any  
CodeVisitor::visitProgram(FocParser::ProgramContext *ctx) {
    if (ctx->decls()) {
        visitDecls(ctx->decls());
    }
    return 0;
}

antlrcpp::Any CodeVisitor::visitDecls(FocParser::DeclsContext *ctx) {
    if (ctx->varDecl()) {
        visitVarDecl(ctx->varDecl());
        visitDecls(ctx->decls());
    } else if (ctx->funDecl()) {
        visitFunDecl(ctx->funDecl());
        visitDecls(ctx->decls());
    }
    return 0;
}

antlrcpp::Any CodeVisitor::visitVarDecl(FocParser::VarDeclContext *ctx) {
    if (ctx->Equal()) {
    } else {
    }
    return 0;
}

antlrcpp::Any CodeVisitor::visitFunDecl(FocParser::FunDeclContext *ctx) {
    return 0;
}

} 
