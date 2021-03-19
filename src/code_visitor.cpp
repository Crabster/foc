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
        decl = visitFunDecl(ctx->funDecl());
        decls = visitDecls(ctx->decls()).as<std::vector<FunDecl>>();
        decls.push_back(decl);
    }
    return decls;
}

antlrcpp::Any CodeVisitor::visitFunDecl(FocParser::FunDeclContext *ctx) {
    FunDecl decl;
    decl.ret_type = visitType(ctx->type());
    decl.id = { .name = ctx->ID()->getText() };
    decl.args = visitFunArgs(ctx->funArgs()).as<std::vector<FunArg>>();
    decl.body = visitFunBody(ctx->funBody());
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
    fun_arg.type = visitType(ctx->type());
    fun_arg.id = { .name = ctx->ID()->getText() };
    fun_args.push_back(fun_arg);
    return fun_args;

}

antlrcpp::Any CodeVisitor::visitFunBody(FocParser::FunBodyContext *ctx) {
    FunBody body;
    if (!ctx->funBody()) {
        return body;
    }
    body = visitFunBody(ctx->funBody());

    if (ctx->varDecl()) {
        FunBodyPart part;
        part.decl = visitVarDecl(ctx->varDecl());
        body.parts.push_back(part);
    } else if (ctx->assignment()) {
        FunBodyPart part;
        part.assign = visitAssignment(ctx->assignment());;
        body.parts.push_back(part);
    } else if (ctx->flow()) {
        FunBodyPart part;
        part.flow = visitFlow(ctx->flow());
        body.parts.push_back(part);
    }
    return body;
}

antlrcpp::Any CodeVisitor::visitVarDecl(FocParser::VarDeclContext *ctx) {
    VarDecl decl;
    if (ctx->Equal()) {
        decl.expr = visitExpr(ctx->expr());
    }
    if (ctx->type()) {
        decl.type = visitType(ctx->type());
    }
    if (ctx->OpenSquare() || ctx->OpenSharp()) {
        decl.ids = visitListIDs(ctx->listIDs()).as<std::vector<ID>>();
    }
    return std::make_shared<VarDecl>(decl);
}

antlrcpp::Any CodeVisitor::visitAssignment(FocParser::AssignmentContext *ctx) {
    Assign assign;
    if (ctx->assignExpr()) {
        assign.assign_expr = visitAssignExpr(ctx->assignExpr());
    } else {
        assign.assign_expr.expr.id = { .name = ctx->ID()->getText() };
    }
    assign.expr = visitExpr(ctx->expr());
    return std::make_shared<Assign>(assign);
}


antlrcpp::Any CodeVisitor::visitFlow(FocParser::FlowContext *ctx) {
    Flow flow;
    if (ctx->cond()) {
        flow.cond = visitCond(ctx->cond());
    } else if (ctx->loop()) {
        flow.loop = visitLoop(ctx->loop());
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
    loop.expr = visitExpr(ctx->expr());
    loop.body = visitFunBody(ctx->funBody());
    return loop;
}

antlrcpp::Any CodeVisitor::visitCond(FocParser::CondContext *ctx) {
    IfCond if_cond = visitIfCond(ctx->ifCond());
    std::vector<IfCond> elif_conds = visitElifConds(ctx->elifConds());

    Cond cond;
    cond.if_conds.push_back(if_cond);
    std::copy(cond.if_conds.begin(), cond.if_conds.end(), std::back_inserter(elif_conds));
    cond.else_body = visitElseCond(ctx->elseCond());
    return cond;
}

antlrcpp::Any CodeVisitor::visitIfCond(FocParser::IfCondContext *ctx) {
    IfCond if_cond;
    if_cond.expr = visitExpr(ctx->expr());
    if_cond.body = visitFunBody(ctx->funBody());
    return if_cond;
}

antlrcpp::Any CodeVisitor::visitElifConds(FocParser::ElifCondsContext *ctx) {
    std::vector<IfCond> elif_conds;
    if (ctx->expr()) {
        IfCond if_cond;
        if_cond.expr = visitExpr(ctx->expr());
        if_cond.body = visitFunBody(ctx->funBody());
        elif_conds.push_back(if_cond);

        std::vector<IfCond> conds = visitElifConds(ctx->elifConds());
        std::copy(elif_conds.begin(), elif_conds.end(), std::back_inserter(conds));
    }
    return elif_conds;
}

antlrcpp::Any CodeVisitor::visitElseCond(FocParser::ElseCondContext *ctx) {
    std::optional<FunBody> body;
    if (ctx->funBody()) {
        body = visitFunBody(ctx->funBody());
    }
    return body;
}

antlrcpp::Any CodeVisitor::visitExpr(FocParser::ExprContext *ctx) {
    Expr expr;
    if (ctx->operator_()) {
        Operation operation;
        operation.op = visitOperator_(ctx->operator_());
        operation.expr_left = visitExpr_(ctx->expr_());
        operation.expr_right = visitExpr(ctx->expr());
        expr.operation = std::make_shared<Operation>(operation);
    } else if (ctx->funCall()) {
        expr.fun_call = visitFunCall(ctx->funCall());
    } else if (ctx->assignExpr()) {
        expr.assign_expr = std::make_shared<AssignExpr>(visitAssignExpr(ctx->assignExpr()));
    } else {
        expr = visitExpr_(ctx->expr_());
    }
    return expr;
}

antlrcpp::Any CodeVisitor::visitExpr_(FocParser::Expr_Context *ctx) {
    Expr expr;
    if (ctx->typeExpr()) {
        expr.type_expr = std::make_shared<TypeExpr>(visitTypeExpr(ctx->typeExpr()));
    } else if (ctx->Minus()) {
        expr = visitExpr(ctx->expr());
        expr.minus = true;
    } else if (ctx->ID()) {
        expr.id = { .name = ctx->ID()->getText() };
    } else {
        expr = visitExpr(ctx->expr());
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
        expr.bool_expr = visitBool_(ctx->bool_());
    } else if (ctx->ptrExpr()) {
        expr.ptr_expr = visitPtrExpr(ctx->ptrExpr());
    } else if (ctx->optExpr()) {
        expr.opt_expr = visitOptExpr(ctx->optExpr());
    } else if (ctx->tupleExpr()) {
        expr.tuple_expr = visitTupleExpr(ctx->tupleExpr());
    } else {
        expr.array_expr = visitArrayExpr(ctx->arrayExpr());
    }
    return expr;
}

antlrcpp::Any CodeVisitor::visitPtrExpr(FocParser::PtrExprContext *ctx) {
    PtrExpr ptr;
    if (ctx->expr()) {
        if (ctx->Ampersand()) {
            ptr.ref_expr = std::make_shared<Expr>(visitExpr(ctx->expr()));
        } else {
            ptr.deref_expr = std::make_shared<Expr>(visitExpr(ctx->expr()));
        }
    }
    return ptr;
}

antlrcpp::Any CodeVisitor::visitOptExpr(FocParser::OptExprContext *ctx) {
    OptExpr opt;
    if (ctx->expr()) {
        if (ctx->QuestionMark()) {
            opt.opt_expr = visitExpr(ctx->expr());
        } else {
            opt.nopt_expr = visitExpr(ctx->expr());
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
    exprs.push_back(visitExpr(ctx->expr()));
    return exprs;
}

antlrcpp::Any CodeVisitor::visitFunCall(FocParser::FunCallContext *ctx) {
    FunCall fun_call;
    fun_call.args = visitListExprs(ctx->listExprs()).as<std::vector<Expr>>();
    return std::make_shared<FunCall>(fun_call);
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
    if (ctx->Plus())     return Operation::Operator::PLUS;
    if (ctx->Minus())    return Operation::Operator::MINUS;
    if (ctx->Star())     return Operation::Operator::STAR;
    if (ctx->Slash())    return Operation::Operator::SLASH;
    if (ctx->IsEqual())  return Operation::Operator::IS_EQUAL;
    if (ctx->NotEqual()) return Operation::Operator::NOT_EQUAL;
    if (ctx->And())      return Operation::Operator::AND;
    if (ctx->Or())       return Operation::Operator::OR;
    if (ctx->less())   return Operation::Operator::LESS;
    if (ctx->greater())  return Operation::Operator::GREATER;
    if (ctx->Leq())      return Operation::Operator::LEQ;
    if (ctx->Geq())      return Operation::Operator::GEQ;
}

antlrcpp::Any CodeVisitor::visitBool_(FocParser::Bool_Context *ctx) {
    return bool(ctx->TRUE());
}

antlrcpp::Any CodeVisitor::visitAssignExpr(FocParser::AssignExprContext *ctx) {
    AssignExpr assign_expr;
    assign_expr.expr = visitExpr_(ctx->expr_());
    assign_expr.idx_expr = visitExpr(ctx->expr());
    return assign_expr;
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
    types.push_back(visitType(ctx->type()));
    return types;
}

}
