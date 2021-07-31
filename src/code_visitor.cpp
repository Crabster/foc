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
    if (ctx->funArgs()) {
        decl.args = visitFunArgs(ctx->funArgs()).as<std::vector<FunArg>>();
    }
    decl.body = visitFunBody(ctx->funBody()).as<FunBody>();
    return decl;
}

antlrcpp::Any CodeVisitor::visitFunArgs(FocParser::FunArgsContext *ctx) {
    std::vector<FunArg> fun_args;
    if (ctx->funArgs()) {
        fun_args = visitFunArgs(ctx->funArgs()).as<std::vector<FunArg>>();
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

    FunBodyPart part;
    if (ctx->varDecl()) {
        part.var = std::move(visitVarDecl(ctx->varDecl()).as<VarDecl>());
    } else if (ctx->assignment()) {
        part.var = std::move(visitAssignment(ctx->assignment()).as<Assign>());
    } else if (ctx->flow()) {
        part.var = std::move(visitFlow(ctx->flow()).as<Flow>());
    } else if (ctx->expr()) {
        part.var = std::move(visitExpr(ctx->expr()).as<Expr>());
    }
    body.parts.push_back(part);
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
    if (ctx->ID()) {
        decl.ids = std::vector<ID>{ ID{ .name = ctx->ID()->getText() } };
    }
    return decl;
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
        flow.var = visitCond(ctx->cond()).as<Cond>();
    } else if (ctx->loop()) {
        flow.var = visitLoop(ctx->loop()).as<Loop>();
    } else if (ctx->CONTINUE()) {
        flow.var = std::make_pair(Flow::ControlTypes::CONTINUE, std::nullopt);
    } else if (ctx->BREAK()) {
        flow.var = std::make_pair(Flow::ControlTypes::BREAK, std::nullopt);
    } else {
        flow.var = std::make_pair(Flow::ControlTypes::RETURN, visitExpr(ctx->expr()).as<Expr>());
    }
    return flow;
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
    std::copy(elif_conds.begin(), elif_conds.end(), std::back_inserter(cond.if_conds));
    std::cout << cond.if_conds.size() << std::endl;
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
        elif_conds = visitElifConds(ctx->elifConds()).as<std::vector<IfCond>>();

        IfCond if_cond;
        if_cond.expr = visitExpr(ctx->expr()).as<Expr>();
        if_cond.body = visitFunBody(ctx->funBody()).as<FunBody>();
        elif_conds.push_back(if_cond);
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
    std::shared_ptr<Expr> first_expr;
    std::shared_ptr<Expr> second_expr;
    if (ctx->expr().size() >= 1) {
        first_expr = std::make_shared<Expr>(visitExpr(ctx->expr()[0]).as<Expr>());
    }
    if (ctx->expr().size() == 2) {
        second_expr = std::make_shared<Expr>(visitExpr(ctx->expr()[1]).as<Expr>());
    }

    if (ctx->operator_()) {
         expr.var = BinOperation{
            .left_expr = first_expr,
            .right_expr = second_expr,
            .op = visitOperator_(ctx->operator_()).as<BinOperation::Operator>(),
        };
    } else if (ctx->UNIT_TYPE()) {
        expr.var = FunCall{
            .fun = first_expr,
        };
    } else if (ctx->listExprs()) {
        expr.var = FunCall{
            .fun = first_expr,
            .fun_args = std::make_shared<std::vector<Expr>>(
                    visitListExprs(ctx->listExprs()).as<std::vector<Expr>>()),
        };
    } else if (ctx->OpenSquare()) {
        expr.var = DerefArray{
            .array_expr = first_expr,
            .deref_expr = second_expr,
        };
    } else if (ctx->OpenSharp()) {
        expr.var = DerefTuple{
            .tuple_expr = first_expr,
            .deref_expr = second_expr,
        };
    } else if (ctx->typeExpr()) {
        expr.var = std::move(visitTypeExpr(ctx->typeExpr()).as<TypeExpr>());
    } else if (ctx->Minus()) {
        expr = *first_expr;
        expr.minus ^= true;
    } else if (ctx->ID()) {
        expr.var = ID{ .name = ctx->ID()->getText() };
    } else {
        expr = *first_expr;
    }
    return expr;
}

antlrcpp::Any CodeVisitor::visitTypeExpr(FocParser::TypeExprContext *ctx) {
    TypeExpr expr;
    if (ctx->INT()) {
        expr.expr = std::stoi(ctx->INT()->getText());
    } else if (ctx->CHAR()) {
        expr.expr = ctx->CHAR()->getText()[0];
    } else if (ctx->STRING()) {
        expr.expr = ctx->STRING()->getText();
    } else if (ctx->bool_()) {
        expr.expr = visitBool_(ctx->bool_()).as<bool>();
    } else if (ctx->ptrExpr()) {
        expr.expr = visitPtrExpr(ctx->ptrExpr()).as<PtrExpr>();
    } else if (ctx->optExpr()) {
        expr.expr = visitOptExpr(ctx->optExpr()).as<OptExpr>();
    } else if (ctx->tupleExpr()) {
        expr.expr = visitTupleExpr(ctx->tupleExpr()).as<TupleExpr>();
    } else {
        expr.expr = visitArrayExpr(ctx->arrayExpr()).as<ArrayExpr>();
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
    if (ctx->listExprs()) {
        exprs = visitListExprs(ctx->listExprs()).as<std::vector<Expr>>();
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
    if (ctx->Plus())     return BinOperation::Operator::PLUS;
    if (ctx->Minus())    return BinOperation::Operator::MINUS;
    if (ctx->Star())     return BinOperation::Operator::STAR;
    if (ctx->Slash())    return BinOperation::Operator::SLASH;
    if (ctx->IsEqual())  return BinOperation::Operator::IS_EQUAL;
    if (ctx->NotEqual()) return BinOperation::Operator::NOT_EQUAL;
    if (ctx->And())      return BinOperation::Operator::AND;
    if (ctx->Or())       return BinOperation::Operator::OR;
    if (ctx->less())     return BinOperation::Operator::LESS;
    if (ctx->greater())  return BinOperation::Operator::GREATER;
    if (ctx->Leq())      return BinOperation::Operator::LEQ;
    if (ctx->Geq())      return BinOperation::Operator::GEQ;
}

antlrcpp::Any CodeVisitor::visitBool_(FocParser::Bool_Context *ctx) {
    return bool(ctx->TRUE());
}

antlrcpp::Any CodeVisitor::visitType(FocParser::TypeContext *ctx) {
    Type type;
    if (ctx->UNIT_TYPE()) {
        type.var = Type::Primitive::UNIT;
    } else if (ctx->INT_TYPE()) {
        type.var = Type::Primitive::INT;
    } else if (ctx->CHAR_TYPE()) {
        type.var = Type::Primitive::CHAR;
    } else if (ctx->BOOL_TYPE()) {
        type.var = Type::Primitive::BOOL;
    } else if (ctx->Star()) {
        type.var = std::make_shared<Type>(std::move(visitType(ctx->type()).as<Type>()));
    } else if (ctx->QuestionMark()) {
        type.var = std::optional(std::move(visitType(ctx->type()).as<Type>()));
    } else if (ctx->Arrow()) {
        Type::Tuple args_types;
        if (ctx->typeList()) {
            args_types = std::move(visitTypeList(ctx->typeList()).as<std::vector<Type>>());
        }
        type.var = std::make_pair(args_types, visitType(ctx->type()).as<Type>());
    } else if (ctx->OpenSharp()) {
        if (ctx->typeList()) {
            type.var = std::move(visitTypeList(ctx->typeList()).as<std::vector<Type>>());
        } else {
            type.var = std::vector<Type>();
        }
    } else {
        type.var = std::make_pair(visitType(ctx->type()).as<Type>(), std::stoi(ctx->INT()->getText()));
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
