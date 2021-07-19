#pragma once

#include <antlr4-runtime.h>
#include "FocLexer.h"
#include "FocParser.h"
#include "FocBaseVisitor.h"

#include "syntax_tree.hpp"

namespace foc {

class CodeVisitor : FocBaseVisitor {
public:
    antlrcpp::Any visitProgram(FocParser::ProgramContext *ctx);
    antlrcpp::Any visitDecls(FocParser::DeclsContext *ctx);
    antlrcpp::Any visitFunDecl(FocParser::FunDeclContext *ctx);
    antlrcpp::Any visitFunArgs(FocParser::FunArgsContext *ctx);
    antlrcpp::Any visitFunBody(FocParser::FunBodyContext *ctx);
    antlrcpp::Any visitVarDecl(FocParser::VarDeclContext *ctx);
    antlrcpp::Any visitAssignment(FocParser::AssignmentContext *ctx);
    antlrcpp::Any visitFlow(FocParser::FlowContext *ctx);
    antlrcpp::Any visitLoop(FocParser::LoopContext *ctx);
    antlrcpp::Any visitCond(FocParser::CondContext *ctx);
    antlrcpp::Any visitIfCond(FocParser::IfCondContext *ctx);
    antlrcpp::Any visitElifConds(FocParser::ElifCondsContext *ctx);
    antlrcpp::Any visitElseCond(FocParser::ElseCondContext *ctx);

    antlrcpp::Any visitExpr(FocParser::ExprContext *ctx);
    antlrcpp::Any visitTypeExpr(FocParser::TypeExprContext *ctx);
    antlrcpp::Any visitPtrExpr(FocParser::PtrExprContext *ctx);
    antlrcpp::Any visitOptExpr(FocParser::OptExprContext *ctx);
    antlrcpp::Any visitTupleExpr(FocParser::TupleExprContext *ctx);
    antlrcpp::Any visitArrayExpr(FocParser::ArrayExprContext *ctx);
    antlrcpp::Any visitListExprs(FocParser::ListExprsContext *ctx);
    antlrcpp::Any visitListExpr(FocParser::ListExprContext *ctx);
    antlrcpp::Any visitListIDs(FocParser::ListIDsContext *ctx);
    antlrcpp::Any visitListID(FocParser::ListIDContext *ctx);
    antlrcpp::Any visitOperator_(FocParser::Operator_Context *ctx);
    antlrcpp::Any visitBool_(FocParser::Bool_Context *ctx);

    antlrcpp::Any visitType(FocParser::TypeContext *ctx);
    antlrcpp::Any visitTypeList(FocParser::TypeListContext *ctx);
};

}
