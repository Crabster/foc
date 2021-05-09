#include "code_visitor.hpp"

namespace foc {

antlrcpp::Any
CodeVisitor::visitProgram(FocParser::ProgramContext *ctx) {
    Program program;
    if (ctx->decls()) {
        program.decls = visitDecls(ctx->decls()).as<std::vector<FunDecl>>();
    }
    return program;
}

antlrcpp::Any CodeVisitor::visitDecls(FocParser::DeclsContext *ctx) {
    std::vector<FunDecl> decls;
    if (ctx->funDecl()) {
        FunDecl decl;
        decl = visitFunDecl(ctx->funDecl()).as<FunDecl>();
        decls = visitDecls(ctx->decls()).as<std::vector<FunDecl>>();
        decls.push_back(decl);
    }
    return decls;
}

antlrcpp::Any CodeVisitor::visitFunDecl(FocParser::FunDeclContext *ctx) {
    FunDecl decl;
    decl.ret_type = visitType(ctx->type()).as<Type>();
    decl.id = { .name = ctx->ID()->getText() };
    decl.args = visitFunArgs(ctx->funArgs()).as<std::vector<FunArg>>();
    decl.body = visitFunBody(ctx->funBody()).as<FunBody>();
    return decl;
}

antlrcpp::Any CodeVisitor::visitFunArgs(FocParser::FunArgsContext *ctx) {
    return visitFunArg(ctx->funArg());
}

antlrcpp::Any CodeVisitor::visitFunArg(FocParser::FunArgContext *ctx) {
    std::vector<FunArg> fun_args;
    if (ctx->funArg()) {
        fun_args = visitFunArg(ctx->funArg()).as<std::vector<FunArg>>();
    }
    FunArg fun_arg;
    fun_arg.type = visitType(ctx->type()).as<Type>();
    fun_arg.id = { .name = ctx->ID()->getText() };
    fun_args.push_back(fun_arg);
    return fun_args;

}

antlrcpp::Any CodeVisitor::visitFunBody(FocParser::FunBodyContext *ctx) {
    FunBody body;
    if (!ctx->funBody()) {
        return body;
    }
    body = visitFunBody(ctx->funBody()).as<FunBody>();

    if (ctx->varDecl()) {
        FunBodyPart part;
        part.decl = visitVarDecl(ctx->varDecl()).as<std::shared_ptr<VarDecl>>();
        body.parts.push_back(part);
    } else if (ctx->assignment()) {
        FunBodyPart part;
        part.assign = std::make_shared<Assign>(visitAssignment(ctx->assignment()).as<Assign>());
        body.parts.push_back(part);
    } else if (ctx->flow()) {
        FunBodyPart part;
        part.flow = visitFlow(ctx->flow()).as<std::shared_ptr<Flow>>();
        body.parts.push_back(part);
    }
    return body;
}

antlrcpp::Any CodeVisitor::visitVarDecl(FocParser::VarDeclContext *ctx) {
    VarDecl decl;
    if (ctx->Equal()) {
        decl.expr = visitExpr(ctx->expr()).as<Expr>();
    }
    if (ctx->type()) {
        decl.type = visitType(ctx->type()).as<Type>();
    }
    if (ctx->OpenSquare() || ctx->OpenSharp()) {
        decl.ids = visitListIDs(ctx->listIDs()).as<std::vector<ID>>();
    }
    return std::make_shared<VarDecl>(decl);
}

antlrcpp::Any CodeVisitor::visitAssignment(FocParser::AssignmentContext *ctx) {
    Assign assign;
    assign.assign_expr = visitExpr(ctx->expr()[0]).as<Expr>();
    assign.expr = visitExpr(ctx->expr()[1]).as<Expr>();
    return assign;
}

antlrcpp::Any CodeVisitor::visitFlow(FocParser::FlowContext *ctx) {
    Flow flow;
    if (ctx->cond()) {
        flow.cond = visitCond(ctx->cond()).as<Cond>();
    } else if (ctx->loop()) {
        flow.loop = visitLoop(ctx->loop()).as<Loop>();
    } else if (ctx->CONTINUE()) {
        flow.control = Flow::Control::CONTINUE;
    } else if (ctx->BREAK()) {
        flow.control = Flow::Control::BREAK;
    } else {
        flow.control = Flow::Control::RETURN;
    }
    return std::make_shared<Flow>(flow);
}

antlrcpp::Any CodeVisitor::visitLoop(FocParser::LoopContext *ctx) {
    Loop loop;
    loop.expr = visitExpr(ctx->expr()).as<Expr>();
    loop.body = visitFunBody(ctx->funBody()).as<FunBody>();
    return loop;
}

antlrcpp::Any CodeVisitor::visitCond(FocParser::CondContext *ctx) {
    IfCond if_cond = visitIfCond(ctx->ifCond()).as<IfCond>();
    std::vector<IfCond> elif_conds = visitElifConds(ctx->elifConds()).as<std::vector<IfCond>>();

    Cond cond;
    cond.if_conds.push_back(if_cond);
    std::copy(cond.if_conds.begin(), cond.if_conds.end(), std::back_inserter(elif_conds));
    cond.else_body = visitElseCond(ctx->elseCond()).as<std::optional<FunBody>>();
    return cond;
}

antlrcpp::Any CodeVisitor::visitIfCond(FocParser::IfCondContext *ctx) {
    IfCond if_cond;
    if_cond.expr = visitExpr(ctx->expr()).as<Expr>();
    if_cond.body = visitFunBody(ctx->funBody()).as<FunBody>();
    return if_cond;
}

antlrcpp::Any CodeVisitor::visitElifConds(FocParser::ElifCondsContext *ctx) {
    std::vector<IfCond> elif_conds;
    if (ctx->expr()) {
        IfCond if_cond;
        if_cond.expr = visitExpr(ctx->expr()).as<Expr>();
        if_cond.body = visitFunBody(ctx->funBody()).as<FunBody>();
        elif_conds.push_back(if_cond);

        std::vector<IfCond> conds = visitElifConds(ctx->elifConds()).as<std::vector<IfCond>>();
        std::copy(elif_conds.begin(), elif_conds.end(), std::back_inserter(conds));
    }
    return elif_conds;
}

antlrcpp::Any CodeVisitor::visitElseCond(FocParser::ElseCondContext *ctx) {
    std::optional<FunBody> body;
    if (ctx->funBody()) {
        body = visitFunBody(ctx->funBody()).as<FunBody>();
    }
    return body;
}

antlrcpp::Any CodeVisitor::visitExpr(FocParser::ExprContext *ctx) {
    Expr expr;
    if (ctx->operator_()) {
        expr.op = visitOperator_(ctx->operator_()).as<Operator>();
    } else if (ctx->listExprs()) {
        expr.fun_args = std::make_shared<std::vector<Expr>>(visitListExprs(ctx->listExprs()).as<std::vector<Expr>>());
    } else if (ctx->OpenSquare()) {
        expr.deref_array = true;
    } else if (ctx->OpenSharp()) {
        expr.deref_tuple = true;
    } else if (ctx->typeExpr()) {
        expr.type_expr = std::make_shared<TypeExpr>(visitTypeExpr(ctx->typeExpr()).as<TypeExpr>());
        return expr;
    } else if (ctx->Minus()) {
        expr.minus = true;
        return expr;
    } else if (ctx->ID()) {
        expr.id = { .name = ctx->ID()->getText() };
        return expr;
    } else {
        return visitExpr(ctx->expr()[0]).as<Expr>();
    }

    expr.primary_expr = std::make_shared<Expr>(visitExpr(ctx->expr()[0]).as<Expr>());
    if (ctx->expr().size() == 2) {
        expr.secondary_expr = std::make_shared<Expr>(visitExpr(ctx->expr()[1]).as<Expr>());
    }

    return expr;
}

antlrcpp::Any CodeVisitor::visitTypeExpr(FocParser::TypeExprContext *ctx) {
    TypeExpr expr;
    if (ctx->INT()) {
        expr.int_expr = std::stoi(ctx->INT()->getText());
    } else if (ctx->CHAR()) {
        expr.char_expr = ctx->CHAR()->getText()[0];
    } else if (ctx->STRING()) {
        expr.str_expr = ctx->STRING()->getText();
    } else if (ctx->bool_()) {
        expr.bool_expr = visitBool_(ctx->bool_()).as<bool>();
    } else if (ctx->ptrExpr()) {
        expr.ptr_expr = visitPtrExpr(ctx->ptrExpr()).as<PtrExpr>();
    } else if (ctx->optExpr()) {
        expr.opt_expr = visitOptExpr(ctx->optExpr()).as<OptExpr>();
    } else if (ctx->tupleExpr()) {
        expr.tuple_expr = visitTupleExpr(ctx->tupleExpr()).as<TupleExpr>();
    } else {
        expr.array_expr = visitArrayExpr(ctx->arrayExpr()).as<ArrayExpr>();
    }
    return expr;
}

antlrcpp::Any CodeVisitor::visitPtrExpr(FocParser::PtrExprContext *ctx) {
    PtrExpr ptr;
    if (ctx->expr()) {
        if (ctx->Ampersand()) {
            ptr.ref_expr = std::make_shared<Expr>(visitExpr(ctx->expr()).as<Expr>());
        } else {
            ptr.deref_expr = std::make_shared<Expr>(visitExpr(ctx->expr()).as<Expr>());
        }
    }
    return ptr;
}

antlrcpp::Any CodeVisitor::visitOptExpr(FocParser::OptExprContext *ctx) {
    OptExpr opt;
    if (ctx->expr()) {
        if (ctx->QuestionMark()) {
            opt.opt_expr = visitExpr(ctx->expr()).as<Expr>();
        } else {
            opt.nopt_expr = visitExpr(ctx->expr()).as<Expr>();
        }
    }
    return opt;
}

antlrcpp::Any CodeVisitor::visitTupleExpr(FocParser::TupleExprContext *ctx) {
    TupleExpr expr;
    expr.exprs = visitListExprs(ctx->listExprs()).as<std::vector<Expr>>();
    return expr;
}

antlrcpp::Any CodeVisitor::visitArrayExpr(FocParser::ArrayExprContext *ctx) {
    ArrayExpr expr;
    expr.exprs = visitListExprs(ctx->listExprs()).as<std::vector<Expr>>();
    return expr;
}

antlrcpp::Any CodeVisitor::visitListExprs(FocParser::ListExprsContext *ctx) {
    std::vector<Expr> exprs;
    if (ctx->listExpr()) {
        exprs = visitListExpr(ctx->listExpr()).as<std::vector<Expr>>();
    }
    return exprs;
}

antlrcpp::Any CodeVisitor::visitListExpr(FocParser::ListExprContext *ctx) {
    std::vector<Expr> exprs;
    if (ctx->listExpr()) {
        exprs = visitListExpr(ctx->listExpr()).as<std::vector<Expr>>();
    }
    exprs.push_back(visitExpr(ctx->expr()).as<Expr>());
    return exprs;
}

antlrcpp::Any CodeVisitor::visitListIDs(FocParser::ListIDsContext *ctx) {
    std::vector<ID> ids;
    if (ctx->listID()) {
        ids = visitListID(ctx->listID()).as<std::vector<ID>>();
    }
    return ids;
}

antlrcpp::Any CodeVisitor::visitListID(FocParser::ListIDContext *ctx) {
    std::vector<ID> ids;
    if (ctx->listID()) {
        ids = visitListID(ctx->listID()).as<std::vector<ID>>();
    }
    ids.push_back({ .name = ctx->ID()->getText() });
    return ids;
}

antlrcpp::Any CodeVisitor::visitOperator_(FocParser::Operator_Context *ctx) {
    if (ctx->Plus())     return Operator::PLUS;
    if (ctx->Minus())    return Operator::MINUS;
    if (ctx->Star())     return Operator::STAR;
    if (ctx->Slash())    return Operator::SLASH;
    if (ctx->IsEqual())  return Operator::IS_EQUAL;
    if (ctx->NotEqual()) return Operator::NOT_EQUAL;
    if (ctx->And())      return Operator::AND;
    if (ctx->Or())       return Operator::OR;
    if (ctx->less())     return Operator::LESS;
    if (ctx->greater())  return Operator::GREATER;
    if (ctx->Leq())      return Operator::LEQ;
    if (ctx->Geq())      return Operator::GEQ;
}

antlrcpp::Any CodeVisitor::visitBool_(FocParser::Bool_Context *ctx) {
    return bool(ctx->TRUE());
}

antlrcpp::Any CodeVisitor::visitType(FocParser::TypeContext *ctx) {
    Type type;
    if (ctx->INT_TYPE()) {
        type.prim_type = Type::Primitive::INT;
    } else if (ctx->CHAR_TYPE()) {
        type.prim_type = Type::Primitive::CHAR;
    } else if (ctx->BOOL_TYPE()) {
        type.prim_type = Type::Primitive::BOOL;
    } else if (ctx->Star()) {
        type.ptr_type = std::make_shared<Type>(std::move(visitType(ctx->type()[0]).as<Type>()));
    } else if (ctx->QuestionMark()) {
        type.opt_type = std::make_shared<Type>(std::move(visitType(ctx->type()[0]).as<Type>()));
    } else if (ctx->OpenSharp()) {
        if (ctx->typeList()) {
            type.tuple_type = std::make_shared<std::vector<Type>>(visitTypeList(ctx->typeList()).as<std::vector<Type>>());
        } else {
            type.tuple_type = std::make_shared<std::vector<Type>>();
        }
    } else if (ctx->OpenSquare()) {
        type.array_type = std::make_shared<std::pair<Type, int>>(
            std::make_pair(visitType(ctx->type()[0]).as<Type>(), std::stoi(ctx->INT()->getText()))
        );
    } else {
        type.fun_type = std::make_shared<std::pair<Type, Type>>(
            std::make_pair(visitType(ctx->type()[0]).as<Type>(), visitType(ctx->type()[1]).as<Type>())
        );
    }
    return type;
}

antlrcpp::Any CodeVisitor::visitTypeList(FocParser::TypeListContext *ctx) {
    std::vector<Type> types;
    if (ctx->typeList()) {
        types = visitTypeList(ctx->typeList()).as<std::vector<Type>>();
    }
    types.push_back(visitType(ctx->type()).as<Type>());
    return types;
}

}
