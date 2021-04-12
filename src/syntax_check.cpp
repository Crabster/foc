#include "syntax_check.hpp"
#include <iostream>

namespace foc {

Type create_fun_type(const FunDecl& fun_decl) {
    std::vector<Type> vec_args_types;
    for (const auto& arg : fun_decl.args) {
        vec_args_types.emplace_back(arg.type);
    }
    Type args_type;
    args_type.tuple_type = std::make_shared<std::vector<Type>>(std::move(vec_args_types));

    Type func_type;
    func_type.fun_type = std::make_shared<std::pair<Type, Type>>(args_type, fun_decl.ret_type);
    return func_type;
}

std::optional<Type> make_bool() {
    auto res = std::make_optional<Type>();
    res->prim_type = Type::Primitive::BOOL;
    return res;
}

std::optional<Type> get_expr_type(const Operation& expr, std::shared_ptr<IDContext> context) {
    auto l_type = get_expr_type(expr.expr_left, context);
    auto r_type = get_expr_type(expr.expr_right, context);
    if (!l_type || !r_type) {
        return {};
    }
    if (l_type != r_type) {
        std::cerr << "Types of left and right expression don't match" << std::endl;
        return {};
    }
    switch (expr.op) {
    case Operation::Operator::PLUS:
    case Operation::Operator::MINUS:
    case Operation::Operator::STAR:
    case Operation::Operator::SLASH:
        if (l_type->prim_type && *l_type->prim_type == Type::Primitive::INT) {
            return l_type;
        }
        std::cerr << "TypeError: Using int operators on non-int types" << std::endl;
        return {};
    case Operation::Operator::IS_EQUAL:
    case Operation::Operator::NOT_EQUAL:
        return make_bool();
    case Operation::Operator::AND:
    case Operation::Operator::OR:
        if (l_type->prim_type && *l_type->prim_type == Type::Primitive::BOOL) {
            return l_type;
        }
        std::cerr << "TypeError: Using logical operators on non-bool types" << std::endl;
        return {};
    case Operation::Operator::LESS:
    case Operation::Operator::GREATER:
    case Operation::Operator::LEQ:
    case Operation::Operator::GEQ:
        if (l_type->prim_type && *l_type->prim_type == Type::Primitive::INT) {
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
    if (!fun_type->fun_type) {
        std::cerr << "TypeError: Trying to use () on non-function type" << std::endl;
        return {};
    }
    if (!fun_type->fun_type->first.tuple_type) {
        throw std::logic_error("Bug in parser, arguments should always be encapsed in tuple");
    }
    const auto& fun_args_type = *fun_type->fun_type->first.tuple_type;
    if (fun_args_type.size() != expr.args.size()) {
        std::cerr << "TypeError: Number of arguments doesn't match with function type" << std::endl;
        return {};
    }
    for (unsigned i = 0; i < fun_args_type.size(); ++i) {
        auto arg_type = get_expr_type(expr.args[i], context);
        if (!arg_type) {
            return {};
        }
        if (fun_args_type[i] != *arg_type) {
            std::cerr << "TypeError: Type of argument " << i << " dosn't match the function type" << std::endl;
            return {};
        }
    }
    return fun_type->fun_type->second;
}

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

std::optional<Type> get_expr_type(const TypeExpr& expr, std::shared_ptr<IDContext> context) {
    Type res;
    if (expr.int_expr) {
        res.prim_type = Type::Primitive::INT;
        return res;
    }
    if (expr.char_expr) {
        res.prim_type = Type::Primitive::CHAR;
        return res;
    }
    if (expr.str_expr) {
        Type sub_res;
        sub_res.prim_type = Type::Primitive::CHAR;
        res.array_type = std::make_shared<std::pair<Type, int>>(sub_res, expr.str_expr->size());
        return res;
    }
    if (expr.bool_expr) {
        res.prim_type = Type::Primitive::BOOL;
        return res;
    }
    if (expr.ptr_expr) {
        if (expr.ptr_expr->ref_expr) {
            auto sub_res = get_expr_type(*expr.ptr_expr->ref_expr, context);
            if (!sub_res) {
                return {};
            }
            res.ptr_type = std::make_shared<Type>(*sub_res);
            return res;
        }
        if (expr.ptr_expr->deref_expr) {
            auto sub_res = get_expr_type(*expr.ptr_expr->deref_expr, context);
            if (!sub_res) {
                return {};
            }
            if (!sub_res->ptr_type) {
                std::cerr << "TypeError: Dereferencing non-pointer type!" << std::endl;
                return {};
            }
            return *sub_res->ptr_type;
        }
        res.ptr_type = std::make_shared<Type>();
        return res;
    }
    if (expr.opt_expr) {
        if (expr.opt_expr->opt_expr) {
            auto sub_res = get_expr_type(*expr.opt_expr->opt_expr, context);
            if (!sub_res) {
                return {};
            }
            res.opt_type = std::make_shared<Type>(*sub_res);
            return res;
        }
        if (expr.opt_expr->nopt_expr) {
            auto sub_res = get_expr_type(*expr.opt_expr->nopt_expr, context);
            if (!sub_res) {
                return {};
            }
            if (!sub_res->opt_type) {
                std::cerr << "TypeError: De-maybeing non-maybe type!" << std::endl;
                return {};
            }
            return *sub_res->opt_type;
        }
        res.opt_type = std::make_shared<Type>();
    }
    if (expr.tuple_expr) {
        std::vector<Type> vres;
        for (const auto& texpr : expr.tuple_expr->exprs) {
            auto sub_res = get_expr_type(texpr, context);
            if (!sub_res) {
                return {};
            }
            vres.emplace_back(*sub_res);
        }
        res.tuple_type = std::make_shared<std::vector<Type>>(std::move(vres));
        return res;
    }
    if (expr.array_expr) {
        if (expr.array_expr->exprs.size() == 0) {
            std::cerr << "ArrayError: We do not allow arrays of size 0" << std::endl;
            return {};
        }
        auto sub_type = get_expr_type(expr.array_expr->exprs[0], context);
        if (!sub_type) {
            return {};
        }
        for (const auto& aexpr : expr.array_expr->exprs) {
            auto sub_res = get_expr_type(aexpr, context);
            if (!sub_res) {
                return {};
            }
            if (*sub_type != *sub_res) {
                std::cerr << "TypeError: Types in array do not match" << std::endl;
                return {};
            }
        }
        res.array_type = std::make_shared<std::pair<Type, int>>(*sub_type, expr.array_expr->exprs.size());
        return res;
    }

    throw std::logic_error("Bug in parser, empty typeExpr");
    return {};
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
    if (expr.operation) {
        result = get_expr_type(*expr.operation, context);
    } else if (expr.fun_call) {
        result = get_expr_type(*expr.fun_call, context);
    } else if (expr.assign_expr) {
        result = get_expr_type(*expr.assign_expr, context);
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
        if (result->prim_type.has_value() && (*result->prim_type == Type::Primitive::INT
            || *result->prim_type == Type::Primitive::BOOL)) {
            return result;
        }
        std::cerr << "TypeError: using 'minus' to non-INT non-BOOL expression" << std::endl;
        return {};
    }
    return result;
}

bool add_vec_context(const std::optional<std::vector<ID>>& ids, const Type& curr_type, std::shared_ptr<IDContext> context) {
    if (!ids || ids->size() == 0) {
        if ((curr_type.array_type && curr_type.array_type->second == 0) ||
            (curr_type.tuple_type && curr_type.tuple_type->size() == 0)) {

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
    if (flow.cond) {
        return syntax_check(*flow.cond, context, in_cycle);
    }
    if (flow.loop) {
        return syntax_check(*flow.loop, context, in_cycle);
    }
    if (flow.control) {
        return syntax_check(*flow.control, context, in_cycle);
    }
    throw std::logic_error("Bug in parser, empty 'Flow'!");
    return false;
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