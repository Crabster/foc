#include "syntax_check.hpp"
#include <iostream>

namespace foc {

Type create_fun_type(const FunDecl& fun_decl) {
    std::vector<Type> vec_args_types;
    for (const auto& arg : fun_decl.args) {
        vec_args_types.emplace_back(arg.type);
    }
    Type args_type;
    args_type.var = std::move(vec_args_types);
    Type ret_type = fun_decl.ret_type;

    Type func_type;
    func_type.var = std::make_pair<Type, Type>(std::move(args_type), std::move(ret_type));
    return func_type;
}

std::optional<Type> make_bool() {
    auto res = std::make_optional<Type>();
    res->var = Type::Primitive::BOOL;
    return res;
}

std::optional<Type> get_operator_type(const Operator& op, const Expr& expr_left,
        const Expr& expr_right, std::shared_ptr<IDContext> context) {
    auto l_type = get_expr_type(expr_left, context);
    auto r_type = get_expr_type(expr_right, context);
    if (!l_type || !r_type) {
        return {};
    }
    if (l_type != r_type) {
        std::cerr << "Types of left and right expression don't match" << std::endl;
        return {};
    }
    switch (op) {
    case Operator::PLUS:
    case Operator::MINUS:
    case Operator::STAR:
    case Operator::SLASH:
        if (std::holds_alternative<Type::Primitive>(l_type->var)
                && std::get<Type::Primitive>(l_type->var) == Type::Primitive::INT) {
            return l_type;
        }
        std::cerr << "TypeError: Using int operators on non-int types" << std::endl;
        return {};
    case Operator::IS_EQUAL:
    case Operator::NOT_EQUAL:
        return make_bool();
    case Operator::AND:
    case Operator::OR:
        if (std::holds_alternative<Type::Primitive>(l_type->var)
                && std::get<Type::Primitive>(l_type->var) == Type::Primitive::BOOL) {
            return l_type;
        }
        std::cerr << "TypeError: Using logical operators on non-bool types" << std::endl;
        return {};
    case Operator::LESS:
    case Operator::GREATER:
    case Operator::LEQ:
    case Operator::GEQ:
        if (std::holds_alternative<Type::Primitive>(l_type->var)
                && std::get<Type::Primitive>(l_type->var) == Type::Primitive::INT) {
            return make_bool();
        }
        std::cerr << "TypeError: Using int comparators on non-int types" << std::endl;
        return {};
    default:
        throw std::logic_error("Bug in parser, unknown operand");
        break;
    }
    return {};
}

std::optional<Type> get_expr_type(const FunCall& expr, std::shared_ptr<IDContext> context) {
    auto fun_type = get_expr_type(expr.expr, context);
    if (!fun_type) {
        return {};
    }
    if (!std::holds_alternative<Type::Fun>(fun_type->var)) {
        std::cerr << "TypeError: Trying to use () on non-function type" << std::endl;
        return {};
    }

    const Type& fun_args_type = std::get<Type::Fun>(fun_type->var).first;
    if (!std::holds_alternative<Type::Tuple>(fun_args_type.var)) {
        throw std::logic_error("Bug in parser, arguments should always be encapsed in tuple");
    }

    const std::vector<Type>& fun_args_types = std::get<Type::Tuple>(fun_args_type.var);
    if (fun_args_types.size() != expr.args.size()) {
        std::cerr << "TypeError: Number of arguments doesn't match with function type" << std::endl;
        return {};
    }
    for (unsigned i = 0; i < fun_args_types.size(); ++i) {
        auto arg_type = get_expr_type(expr.args[i], context);
        if (!arg_type) {
            return {};
        }
        if (fun_args_types[i] != *arg_type) {
            std::cerr << "TypeError: Type of argument " << i << " dosn't match the function type" << std::endl;
            return {};
        }
    }
    const Type& fun_ret_type = std::get<Type::Fun>(fun_type->var).second;
    return fun_ret_type;
}

/*
std::optional<Type> get_expr_type(const AssignExpr& ass_expr, std::shared_ptr<IDContext> context) {
    auto l_type = get_expr_type(ass_expr.expr, context);
    if (!l_type) {
        return {};
    }
    if (!ass_expr.idx_expr) {
        return l_type;
    }
    // stále nechápu tu sémantiku ass expr, taaaakže toto zkontrolovat
    auto idx_type = get_expr_type(*ass_expr.idx_expr, context);
    if (!idx_type) {
        return {};
    }
    if (!idx_type->prim_type || *idx_type->prim_type != Type::Primitive::INT) {
        std::cerr << "TypeError: Dereferencing with non-int type" << std::endl;
        return {};
    }
    if (l_type->tuple_type) {
        if (ass_expr.idx_expr->type_expr && ass_expr.idx_expr->type_expr->int_expr) {
            const auto& tt = *l_type->tuple_type;
            const auto& tid = *ass_expr.idx_expr->type_expr->int_expr;
            if (tt.size() <= tid) {
                std::cerr << "TupleError: Trying to access tuple element out of range" << std::endl;
                return {};
            }
            return tt[tid];
        }
        std::cerr << "TupleError: It is possible to access tuple element only via constant" << std::endl;
        return {};
    }
    if (l_type->array_type) {
        const auto& tt = *l_type->array_type;
        if (ass_expr.idx_expr->type_expr && ass_expr.idx_expr->type_expr->int_expr) {
            const auto& tid = *ass_expr.idx_expr->type_expr->int_expr;
            if (tt.second <= tid) {
                std::cerr << "ArrayError: Accesing element of out bound with constant" << std::endl;
                return {};
            }
        }
        return tt.first;
    }
    std::cerr << "TypeError: IDK MAN, ASS EXPR?!?!? Co tím krab myslel???" << std::endl;
    return {};
}
*/

std::optional<Type> get_expr_type(const int& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::INT;
    return res;
}

std::optional<Type> get_expr_type(const char& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::CHAR;
    return res;
}

std::optional<Type> get_expr_type(const std::string& expr, std::shared_ptr<IDContext> context) {
    Type res;
    Type sub_res;
    sub_res.var = Type::Primitive::CHAR;
    res.var = std::make_pair<Type, int>(std::move(sub_res), expr.size());
    return res;
}

std::optional<Type> get_expr_type(const bool& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::BOOL;
    return res;
}

std::optional<Type> get_expr_type(const PtrExpr& expr, std::shared_ptr<IDContext> context) {
    Type res;
    if (expr.ref_expr) {
        auto sub_res = get_expr_type(*expr.ref_expr, context);
        if (!sub_res) {
            return {};
        }
        res.var = std::make_shared<Type>(*sub_res);
        return res;
    } else if (expr.deref_expr) {
        auto sub_res = get_expr_type(*expr.deref_expr, context);
        if (!sub_res) {
            return {};
        }
        if (!std::holds_alternative<Type::Ptr>(sub_res->var)) {
            std::cerr << "TypeError: Dereferencing non-pointer type!" << std::endl;
            return {};
        }
        return *std::get<Type::Ptr>(sub_res->var);
    } else {
        res.var = std::make_shared<Type>();
        return res;
    }
}

std::optional<Type> get_expr_type(const OptExpr& expr, std::shared_ptr<IDContext> context) {
    Type res;
    if (expr.opt_expr) {
        auto sub_res = get_expr_type(*expr.opt_expr, context);
        if (!sub_res) {
            return {};
        }
        res.var = std::optional<Type>(*sub_res);
        return res;
    } else if (expr.nopt_expr) {
        auto sub_res = get_expr_type(*expr.nopt_expr, context);
        if (!sub_res) {
            return {};
        }
        if (!std::holds_alternative<Type::Opt>(sub_res->var)) {
            std::cerr << "TypeError: De-maybeing non-maybe type!" << std::endl;
            return {};
        }
        return *std::get<Type::Opt>(sub_res->var);
    } else {
        res.var = std::optional<Type>();
        return res;
    }
}

std::optional<Type> get_expr_type(const TupleExpr& expr, std::shared_ptr<IDContext> context) {
    Type res;
    std::vector<Type> vres;
    for (const auto& texpr : expr.exprs) {
        auto sub_res = get_expr_type(texpr, context);
        if (!sub_res) {
            return {};
        }
        vres.emplace_back(*sub_res);
    }
    res.var = std::move(vres);
    return res;
}

std::optional<Type> get_expr_type(const ArrayExpr& expr, std::shared_ptr<IDContext> context) {
    Type res;
    if (expr.exprs.size() == 0) {
        std::cerr << "ArrayError: We do not allow arrays of size 0" << std::endl;
        return {};
    }
    auto sub_type = get_expr_type(expr.exprs[0], context);
    if (!sub_type) {
        return {};
    }
    for (const auto& aexpr : expr.exprs) {
        auto sub_res = get_expr_type(aexpr, context);
        if (!sub_res) {
            return {};
        }
        if (*sub_type != *sub_res) {
            std::cerr << "TypeError: Types in array do not match" << std::endl;
            return {};
        }
    }
    res.var = std::make_pair<Type, int>(std::move(*sub_type), expr.exprs.size());
    return res;
}

std::optional<Type> get_expr_type(const TypeExpr& expr, std::shared_ptr<IDContext> context) {
    auto visit_cb = [&](const auto& arg){
        return get_expr_type(arg, context);
    };
    return std::visit(visit_cb, expr.expr);
}

std::optional<Type> get_expr_type(const ID& expr, std::shared_ptr<IDContext> context) {
    auto type = context->find_type(expr);
    if (!type) {
        std::cerr << "SomeError: identifier '" << expr.name << "' was not declared or is out of scope." << std::endl;
        return {};
    }
    return type;
}

std::optional<Type> get_expr_type(const Expr& expr, std::shared_ptr<IDContext> context) {
    std::optional<Type> result{};
    if (expr.op && expr.primary_expr && expr.secondary_expr) {
        result = get_operator_type(*expr.op, *expr.primary_expr, *expr.secondary_expr, context);
    } else if (expr.type_expr) {
        result = get_expr_type(*expr.type_expr, context);
    } else if (expr.id) {
        result = get_expr_type(*expr.id, context);
    } else {
        throw std::logic_error("Bug in parser, empty 'expr'");
    }
    if (!result) {
        return result;
    }
    if (expr.minus) {
        if (std::holds_alternative<Type::Primitive>(result->var) &&
                (std::get<Type::Primitive>(result->var) == Type::Primitive::INT ||
                 std::get<Type::Primitive>(result->var) == Type::Primitive::BOOL)) {
            return result;
        }
        std::cerr << "TypeError: using 'minus' to non-INT non-BOOL expression" << std::endl;
        return {};
    }
    return result;
}

bool add_vec_context(const std::optional<std::vector<ID>>& ids, const Type& curr_type, std::shared_ptr<IDContext> context) {
    if (!ids || ids->size() == 0) {
        if (std::holds_alternative<Type::Array>(curr_type.var)
                && std::get<Type::Array>(curr_type.var).second == 0) {
            return true;
        }
        if (std::holds_alternative<Type::Tuple>(curr_type.var)
                && std::get<Type::Tuple>(curr_type.var).size() == 0) {
            return true;
        }
        std::cerr << "TypeError: empty ids in variable declaration" << std::endl;
        return false;
    }
    if (ids->size() == 1) {
        context->add_context(ids->at(0), curr_type);
        return true;
    }
    return context->add_contexts(*ids, curr_type);
}

bool syntax_check(const VarDecl& decl, std::shared_ptr<IDContext> context) {
    if (!decl.expr) {
            if (!decl.type) {
                throw std::logic_error("Error in parser, using `_ x;`");
            }
            return add_vec_context(decl.ids, *decl.type, context);
    }
    auto opt_type = get_expr_type(*decl.expr, context);
    if (!opt_type) {
        return false;
    }
    Type expr_type = *opt_type;
    if (!decl.type) {
        return add_vec_context(decl.ids, expr_type, context);
    }
    auto res = *decl.type == expr_type;
    if (!res) {
        std::cerr << "TypeError: expression does not match declared type." << std::endl;
        return false;
    }

    return add_vec_context(decl.ids, *decl.type, context);
}

bool syntax_check(const Assign& ass, std::shared_ptr<IDContext> context) {
    // I want to change syntax, thus I skip
    // assign -> only left values
    return true;
}

bool syntax_check(const IfCond& if_cond, std::shared_ptr<IDContext> context, bool in_cycle) {
    bool res = true;
    auto cond_type = get_expr_type(if_cond.expr, context);
    res &= (cond_type.has_value() && *cond_type == make_bool());
    auto loc_context = std::make_shared<IDContext>();
    loc_context->parent_context = context;
    res &= syntax_check(if_cond.body, loc_context, in_cycle);
    return res;
}

bool syntax_check(const Cond& cond, std::shared_ptr<IDContext> context, bool in_cycle) {
    bool res = true;
    for (const auto& ifcond : cond.if_conds) {
        res &= syntax_check(ifcond, context, in_cycle);
    }
    if (cond.else_body) {
        auto loc_context = std::make_shared<IDContext>();
        loc_context->parent_context = context;
        res &= syntax_check(*cond.else_body, loc_context, in_cycle);
    }
    return res;
}

bool syntax_check(const Loop& loop, std::shared_ptr<IDContext> context, bool in_cycle) {
    bool res = true;
    auto cond_type = get_expr_type(loop.expr, context);
    res &= (cond_type.has_value() && (*cond_type == make_bool()));
    auto loc_context = std::make_shared<IDContext>();
    loc_context->parent_context = context;
    res &= syntax_check(loop.body, loc_context, true);
    return res;
}

bool syntax_check(const Flow::Control& ctrl, std::shared_ptr<IDContext> context, bool in_cycle) {
    // here should be added type_check of the return expr, meaning we should propagate
    // the return type!
    if (ctrl == Flow::Control::CONTINUE) {
        if (!in_cycle) {
            std::cerr << "Using 'continue' outside loop" << std::endl;
        }
        return in_cycle;
    }
    if (ctrl == Flow::Control::BREAK) {
        if (!in_cycle) {
            std::cerr << "Using 'break' outside loop" << std::endl;
        }
        return in_cycle;
    }
    if (ctrl == Flow::Control::RETURN) {
        return true;
    }

    throw std::logic_error("Bug in parser, empty 'control'");
    return false;
}

bool syntax_check(const Flow& flow, std::shared_ptr<IDContext> context, bool in_cycle) {
    auto visit_cb = [&](auto arg){
        return syntax_check(arg, context, in_cycle);
    };
    return std::visit(visit_cb, flow.var);
}

bool syntax_check(const FunBody& body, std::shared_ptr<IDContext> context, bool in_cycle) {
    bool res = true;
    for (const auto& part : body.parts) {
        if (part.decl) {
            res &= syntax_check(*part.decl, context);
            continue;
        }
        if (part.assign) {
            res &= syntax_check(*part.assign, context);
            continue;
        }
        if (part.flow) {
            res &= syntax_check(*part.flow, context, in_cycle);
            continue;
        }
    }
    return res;
}

bool syntax_check(const FunDecl& fun_decl, std::shared_ptr<IDContext> par_context) {
    auto loc_context = std::make_shared<IDContext>();
    loc_context->parent_context = par_context;
    for (const auto& fun_arg : fun_decl.args) {
        loc_context->add_context(fun_arg.id, fun_arg.type);
    }
    return syntax_check(fun_decl.body, loc_context, false);
}

bool syntax_check(const Program& prog) {
    auto glob_context = std::make_shared<IDContext>();
    bool succ = true;

    for (const auto& fun_decl : prog.decls) {
        glob_context->add_context(fun_decl.id, create_fun_type(fun_decl));
    }

    for (const auto& fun_decl : prog.decls) {
        succ &= syntax_check(fun_decl, glob_context);
    }

    return succ;
}


}
