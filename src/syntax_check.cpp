#include "syntax_check.hpp"
#include <iostream>

namespace foc {

Type create_fun_type(const FunDecl& fun_decl) {
    std::vector<Type> args_types;
    for (const auto& arg : fun_decl.args) {
        args_types.emplace_back(arg.type);
    }
    Type ret_type = fun_decl.ret_type;

    Type func_type;
    func_type.var = std::make_pair<Type::Tuple, Type>(std::move(args_types), std::move(ret_type));
    return func_type;
}

std::optional<Type> make_bool() {
    auto res = std::make_optional<Type>();
    res->var = Type::Primitive::BOOL;
    return res;
}

std::optional<Type> get_texpr_type(const int& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::INT;
    return res;
}

std::optional<Type> get_texpr_type(const char& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::CHAR;
    return res;
}

std::optional<Type> get_texpr_type(const std::string& expr, std::shared_ptr<IDContext> context) {
    Type res;
    Type sub_res;
    sub_res.var = Type::Primitive::CHAR;
    res.var = std::make_pair<Type, int>(std::move(sub_res), expr.size());
    return res;
}

std::optional<Type> get_texpr_type(const bool& expr, std::shared_ptr<IDContext> context) {
    Type res;
    res.var = Type::Primitive::BOOL;
    return res;
}

std::optional<Type> get_texpr_type(const PtrExpr& expr, std::shared_ptr<IDContext> context) {
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

std::optional<Type> get_texpr_type(const OptExpr& expr, std::shared_ptr<IDContext> context) {
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

std::optional<Type> get_texpr_type(const TupleExpr& expr, std::shared_ptr<IDContext> context) {
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

std::optional<Type> get_texpr_type(const ArrayExpr& expr, std::shared_ptr<IDContext> context) {
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

std::optional<Type> get_op_type(const BinOperation::Operator& op, const Type& l_type, const Type& r_type) {
    if (l_type != r_type) {
        std::cerr << "Types of left and right expression don't match" << std::endl;
        return {};
    }
    switch (op) {
    case BinOperation::Operator::PLUS:
    case BinOperation::Operator::MINUS:
    case BinOperation::Operator::STAR:
    case BinOperation::Operator::SLASH:
        if (std::holds_alternative<Type::Primitive>(l_type.var)
                && std::get<Type::Primitive>(l_type.var) == Type::Primitive::INT) {
            return l_type;
        }
        std::cerr << "TypeError: Using int operators on non-int types" << std::endl;
        return {};
    case BinOperation::Operator::IS_EQUAL:
    case BinOperation::Operator::NOT_EQUAL:
        // Maybe forbid smthng like f = g? But we'll see
        // when we will do assembler :shrug:
        return make_bool();
    case BinOperation::Operator::AND:
    case BinOperation::Operator::OR:
        if (std::holds_alternative<Type::Primitive>(l_type.var)
                && std::get<Type::Primitive>(l_type.var) == Type::Primitive::BOOL) {
            return l_type;
        }
        std::cerr << "TypeError: Using logical operators on non-bool types" << std::endl;
        return {};
    case BinOperation::Operator::LESS:
    case BinOperation::Operator::GREATER:
    case BinOperation::Operator::LEQ:
    case BinOperation::Operator::GEQ:
        if (std::holds_alternative<Type::Primitive>(l_type.var)
                && std::get<Type::Primitive>(l_type.var) == Type::Primitive::INT) {
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

std::optional<int> apply_op(BinOperation::Operator op, int l, int r) {
    switch (op) {
    case BinOperation::Operator::PLUS:
        return { l+r };
    case BinOperation::Operator::MINUS:
        return { l-r };
    case BinOperation::Operator::STAR:
        return { l*r };
    case BinOperation::Operator::SLASH:
        if (r == 0) {
            std::cerr << "Error: Div by 0" << std::endl;
            return {};
        }
        return { l/r };
    case BinOperation::Operator::IS_EQUAL:
    case BinOperation::Operator::NOT_EQUAL:
    case BinOperation::Operator::AND:
    case BinOperation::Operator::OR:
    case BinOperation::Operator::LESS:
    case BinOperation::Operator::GREATER:
    case BinOperation::Operator::LEQ:
    case BinOperation::Operator::GEQ:
        return {};
    default:
        throw std::logic_error("Bug in parser, unknown operand");
        break;
    }
    return {};
}

std::optional<int> get_valid_index(const Expr& expr) {
    int minus = expr.minus ? -1 : 1;
    if (std::holds_alternative<BinOperation>(expr.var)) {
        const auto& binop = std::get<BinOperation>(expr.var);
        if (!binop.left_expr || !binop.right_expr) {
            throw std::logic_error("Wtf empty binop?!?");
        }
        auto l_res = get_valid_index(*binop.left_expr);
        auto r_res = get_valid_index(*binop.right_expr);
        if (!l_res || !r_res) {
            return {};
        }
        auto res = apply_op(binop.op, *l_res, *r_res);
        if (!res) {
            return {};
        }
        return { *res * minus };
    } else if (std::holds_alternative<TypeExpr>(expr.var)) {
        const auto& texpr = std::get<TypeExpr>(expr.var);
        if (!std::holds_alternative<int>(texpr.expr)) {
            return {};
        }
        return { std::get<int>(texpr.expr) * minus };
    } else {
        return {};
    }
}

bool fun_args_matching(const Type::Fun& fun, const std::shared_ptr<std::vector<Expr>>& args, std::shared_ptr<IDContext> context) {
    const auto& fun_args = fun.first;
    if (!args) {
        if (fun_args.size() == 0) {
            return true;
        }
        std::cerr << "Error: Function expects params (or compiler broke and gives empty sp" << std::endl;
        return false;
    }
    const auto& real_args = *args;
    if (fun_args.size() != real_args.size()) {
        std::cerr << "Error: Non-mathcing number of params" << std::endl;
        return false;
    }
    bool res = true;
    for (unsigned i = 0; i < fun_args.size(); ++i) {
        auto r_type = get_expr_type(real_args[i], context);
        if (!r_type) {
            res = false;
            continue;
        }
        res &= fun_args[i] == *r_type;
    }
    return res;
}

std::optional<Type> get_expr_type(const Expr& expr, std::shared_ptr<IDContext> context) {
    std::optional<Type> result{};

    if (std::holds_alternative<BinOperation>(expr.var)) {
        const auto& op_expr = std::get<BinOperation>(expr.var);
        if (!op_expr.left_expr || !op_expr.right_expr) {
            throw std::logic_error("Wtf man, empty binopexpr???");
        }
        auto ltype = get_expr_type(*op_expr.left_expr, context);
        auto rtype = get_expr_type(*op_expr.right_expr, context);
        if (!ltype || !rtype) {
            return {};
        }
        result = get_op_type(op_expr.op, *ltype, *rtype);

    } else if (std::holds_alternative<DerefArray>(expr.var)) {
        const auto& arr_expr = std::get<DerefArray>(expr.var).array_expr;
        const auto& deref_expr = std::get<DerefArray>(expr.var).deref_expr;
        if (!arr_expr || !deref_expr) {
            throw std::logic_error("Wtf man, empty derefArray???");
        }
        auto ltype = get_expr_type(*arr_expr, context);
        auto rtype = get_expr_type(*deref_expr, context);
        if (!ltype || !rtype) {
            return {};
        }
        if (!std::holds_alternative<Type::Array>(ltype->var)) {
            std::cerr << "TypeError: Trying to dereference non-array as array" << std::endl;
            return {};
        }
        if (!std::holds_alternative<Type::Primitive>(rtype->var)
                || std::get<Type::Primitive>(rtype->var) != Type::Primitive::INT) {
            std::cerr << "TypeError: Trying to index with a non-int" << std::endl;
            return {};
        }
        result = std::get<Type::Array>(ltype->var).first;

    } else if (std::holds_alternative<DerefTuple>(expr.var)) {
        const auto& tuple_expr = std::get<DerefTuple>(expr.var).tuple_expr;
        const auto& deref_expr = std::get<DerefTuple>(expr.var).deref_expr;
        if (!tuple_expr || deref_expr) {
            throw std::logic_error("Wtf man, empty derefTuple???");
        }
        auto ltype = get_expr_type(*tuple_expr, context);
        if (!ltype) {
            return {};
        }
        if (!std::holds_alternative<Type::Tuple>(ltype->var)) {
            std::cerr << "TypeError: Trying to dereference non-tuple as tuple" << std::endl;
            return {};
        }
        auto opt_index = get_valid_index(*deref_expr);
        if (!opt_index) {
            std::cerr << "Error: Using tuple bad stupid non-compile-time index" << std::endl;
            return {};
        }
        if (*opt_index < 0) {
            std::cerr << "Error: Indexing tuple with negative index" << std::endl;
            return {};
        }
        const auto& types = std::get<Type::Tuple>(ltype->var);
        if (types.size() <= *opt_index) {
            std::cerr << "Error: Indexing tuple out of range" << std::endl;
        }
        result = types[*opt_index];

    } else if (std::holds_alternative<FunCall>(expr.var)) {
        const auto& fun = std::get<FunCall>(expr.var).fun;
        const auto& args = std::get<FunCall>(expr.var).fun_args;
        if (!fun) {
            throw std::logic_error("Wtf empty fun fun?!?");
        }
        auto f_type = get_expr_type(*fun, context);
        if (!f_type) {
            return {};
        }
        if (!std::holds_alternative<Type::Fun>(f_type->var)) {
            std::cerr << "TypeError: Trying to invoke a non-fun expr" << std::endl;
            return {};
        }
        if (!fun_args_matching(std::get<Type::Fun>(f_type->var), args, context)) {
            return {};
        }
        result = std::get<Type::Fun>(f_type->var).second;

    } else if (std::holds_alternative<ID>(expr.var)) {
        if (!context->is_declared(std::get<ID>(expr.var))) {
            std::cerr << "Error: Use of undeclared id: " << std::get<ID>(expr.var).name << std::endl;
            return {};
        }
        result = context->find_type(std::get<ID>(expr.var));

    } else if (std::holds_alternative<TypeExpr>(expr.var)) {
        auto visit_cb = [&](const auto& arg) {
            return get_texpr_type(arg, context);
        };
        result = std::visit(visit_cb, std::get<TypeExpr>(expr.var).expr);

    } else {
        throw std::logic_error("Idk man, empty expr?");
    }

    if (expr.minus && result) {
        if (!std::holds_alternative<Type::Primitive>(result->var)
            || (std::get<Type::Primitive>(result->var) != Type::Primitive::INT
                && std::get<Type::Primitive>(result->var) != Type::Primitive::BOOL)) {
            std::cerr << "TypeError: using 'minus' to non-INT non-BOOL expression" << std::endl;
            return {};
        }
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

bool is_lvalue(const int& expr) {
    std::cerr << "Error: Ass into int" << std::endl;
    return false;
}

bool is_lvalue(const char& expr) {
    std::cerr << "Error: Ass into char" << std::endl;
    return false;
}

bool is_lvalue(const std::string& expr) {
    std::cerr << "Error: Ass into string" << std::endl;
    return false;
}

bool is_lvalue(const bool& expr) {
    std::cerr << "Error: Ass into bool" << std::endl;
    return false;
}

bool is_lvalue(const PtrExpr& expr) {
    if (expr.ref_expr) {
        return is_lvalue(*expr.ref_expr);
    }
    if (expr.deref_expr) {
        return is_lvalue(*expr.deref_expr);
    }
    std::cerr << "Error: Assigning into &$" << std::endl;
    return false;
}

bool is_lvalue(const OptExpr& expr) {
    if (expr.opt_expr) {
        return is_lvalue(*expr.opt_expr);
    }
    if (expr.nopt_expr) {
        return is_lvalue(*expr.nopt_expr);
    }
    std::cerr << "Error: Assigning into ?$" << std::endl;
    return false;
}

bool is_lvalue(const TupleExpr& expr) {
    bool res = true;
    for (const auto& texpr : expr.exprs) {
        res &= is_lvalue(texpr);
    }
    return res;
}

bool is_lvalue(const ArrayExpr& expr) {
    bool res = true;
    for (const auto& aexpr : expr.exprs) {
        res &= is_lvalue(aexpr);
    }
    return res;
}

bool is_lvalue(const Expr& expr) {
    // We will limit ourselves to only few stuff, that can be on the left
    if (std::holds_alternative<BinOperation>(expr.var)) {
        std::cerr << "Error: trying to assign into bin-op" << std::endl;
        return false;
    } else if (std::holds_alternative<DerefArray>(expr.var)) {
        const auto& arr_expr = std::get<DerefArray>(expr.var).array_expr;
        if (!arr_expr) {
            throw std::logic_error("Wtf man, empty derefArray???");
        }
        return is_lvalue(*arr_expr);
    } else if (std::holds_alternative<DerefTuple>(expr.var)) {
        const auto& tuple_expr = std::get<DerefTuple>(expr.var).tuple_expr;
        if (!tuple_expr) {
            throw std::logic_error("Wtf man, empty derefTuple???");
        }
        return is_lvalue(*tuple_expr);
    } else if (std::holds_alternative<FunCall>(expr.var)) {
        std::cerr << "Error: trying to ass into fun ret" << std::endl;
        return false;
    } else if (std::holds_alternative<ID>(expr.var)) {
        return true;
    } else if (std::holds_alternative<TypeExpr>(expr.var)) {
        auto visit_cb = [&](const auto& arg) {
            return is_lvalue(arg);
        };
        return std::visit(visit_cb, std::get<TypeExpr>(expr.var).expr);
    } else {
        throw std::logic_error("Idk man, empty expr?");
    }
    return true;
}

bool syntax_check(const Assign& ass, std::shared_ptr<IDContext> context) {
    auto opt_ltype = get_expr_type(ass.assign_expr, context);
    auto opt_rtype = get_expr_type(ass.expr, context);
    if (!opt_ltype || !opt_rtype) {
        std::cerr << "Error: Ass bad" << std::endl;
        return false;
    }
    if (*opt_ltype != *opt_rtype) {
        std::cerr << "Error: Ass do not match" << std::endl;
        return false;
    }
    if (!is_lvalue(ass.assign_expr)) {
        std::cerr << "Error: Stupid ass" << std::endl;
        return false;
    }
    return true;
}

bool syntax_check(const IfCond& if_cond, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    bool res = true;
    auto cond_type = get_expr_type(if_cond.expr, context);
    res &= (cond_type.has_value() && *cond_type == make_bool());
    auto loc_context = std::make_shared<IDContext>(true);
    loc_context->parent_context = context;
    res &= syntax_check(if_cond.body, loc_context, in_cycle, ret_type);
    return res;
}

bool syntax_check(const Cond& cond, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    bool res = true;
    for (const auto& ifcond : cond.if_conds) {
        res &= syntax_check(ifcond, context, in_cycle, ret_type);
    }
    if (cond.else_body) {
        auto loc_context = std::make_shared<IDContext>(true);
        loc_context->parent_context = context;
        res &= syntax_check(*cond.else_body, loc_context, in_cycle, ret_type);
    }
    return res;
}

bool syntax_check(const Loop& loop, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    bool res = true;
    auto cond_type = get_expr_type(loop.expr, context);
    res &= (cond_type.has_value() && (*cond_type == make_bool()));
    auto loc_context = std::make_shared<IDContext>(true);
    loc_context->parent_context = context;
    res &= syntax_check(loop.body, loc_context, true, ret_type);
    return res;
}

bool syntax_check(const Flow::Control& ctrl, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    if (ctrl.first == Flow::ControlTypes::CONTINUE) {
        if (!in_cycle) {
            std::cerr << "Warning: Using 'continue' outside loop" << std::endl;
        }
        return in_cycle;
    }
    if (ctrl.first == Flow::ControlTypes::BREAK) {
        if (!in_cycle) {
            std::cerr << "Warning: Using 'break' outside loop" << std::endl;
        }
        return in_cycle;
    }
    if (ctrl.first == Flow::ControlTypes::RETURN) {
        if (!ctrl.second) {
            if (ret_type.empty()) {
                return true;
            }
            std::cerr << "Error: Returning empty expression from non-void function" << std::endl;
            return false;
        }
        auto opt_type = get_expr_type(*ctrl.second, context);
        if (!opt_type || *opt_type != ret_type) {
            std::cerr << "Error: Type of returning expression does not match the return type of function" << std::endl;
            return false;
        }
        return true;
    }

    throw std::logic_error("Bug in parser, empty 'control'");
    return false;
}

bool syntax_check(const Flow& flow, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    auto visit_cb = [&](auto arg){
        return syntax_check(arg, context, in_cycle, ret_type);
    };
    return std::visit(visit_cb, flow.var);
}

bool syntax_check(const FunBody& body, std::shared_ptr<IDContext> context, bool in_cycle, const Type& ret_type) {
    bool res = true;
    for (const auto& part : body.parts) {
        if (std::holds_alternative<VarDecl>(part.var)) {
            res &= syntax_check(std::get<VarDecl>(part.var), context);
            continue;
        } else if (std::holds_alternative<Assign>(part.var)) {
            res &= syntax_check(std::get<Assign>(part.var), context);
            continue;
        } else if (std::holds_alternative<Flow>(part.var)) {
            res &= syntax_check(std::get<Flow>(part.var), context, in_cycle, ret_type);
            continue;
        } else {
            throw std::logic_error("Bug in parser, empty body");
        }
    }
    return res;
}

bool syntax_check(const FunDecl& fun_decl, std::shared_ptr<IDContext> par_context) {
    auto loc_context = std::make_shared<IDContext>(true);
    loc_context->parent_context = par_context;
    for (const auto& fun_arg : fun_decl.args) {
        loc_context->add_context(fun_arg.id, fun_arg.type);
    }
    return syntax_check(fun_decl.body, loc_context, false, fun_decl.ret_type);
}

bool syntax_check(const Program& prog) {
    auto glob_context = std::make_shared<IDContext>(true);
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
