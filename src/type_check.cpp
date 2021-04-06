#include "type_check.hpp"

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

Type get_expr_type(const Expr& expr, std::shared_ptr<IDContext> context) {
    // TODO
    Type result;
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

bool type_check(const VarDecl& decl, std::shared_ptr<IDContext> context) {
    if (!decl.expr) {
            if (!decl.type) {
                throw std::logic_error("Error in parser, using `_ x;`");
            }
            return add_vec_context(decl.ids, *decl.type, context);
    }
    Type expr_type = get_expr_type(*decl.expr, context);
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

bool type_check(const FunBody& body, std::shared_ptr<IDContext> context) {
    bool res = true;
    for (const auto& part : body.parts) {
        if (part.decl) {
            res &= type_check(*part.decl, context);
        }
        if (part.assign) {
            //TODO
            continue;
        }
        if (part.flow) {
            //TODO
            continue;
        }
    }
    return false;
}

bool type_check(const FunDecl& fun_decl, std::shared_ptr<IDContext> par_context) {
    auto loc_context = std::make_shared<IDContext>();
    loc_context->parent_context = par_context;
    for (const auto& fun_arg : fun_decl.args) {
        loc_context->add_context(fun_arg.id, fun_arg.type);
    }
    return type_check(fun_decl.body, loc_context);
}

bool type_check(const Program& prog) {
    auto glob_context = std::make_shared<IDContext>();
    bool succ = true;

    for (const auto& fun_decl : prog.decls) {
        glob_context->add_context(fun_decl.id, create_fun_type(fun_decl));
    }

    for (const auto& fun_decl : prog.decls) {
        succ &= type_check(fun_decl, glob_context);
    }

    return true;
}


}