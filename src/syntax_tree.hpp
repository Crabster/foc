#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <stdexcept>
#include <functional>
#include <variant>
#include <iostream>

#include "util.hpp"

namespace foc {

struct ID {
    std::string name;

    bool operator==(const ID& other) const;
    bool operator!=(const ID& other) const;

    std::string to_string() const;
};

struct Expr;

struct BinOperation {
    enum Operator {
        PLUS,
        MINUS,
        STAR,
        SLASH,
        IS_EQUAL,
        NOT_EQUAL,
        AND,
        OR,
        LESS,
        GREATER,
        LEQ,
        GEQ,
    };

    std::shared_ptr<Expr> left_expr;
    std::shared_ptr<Expr> right_expr;
    Operator op;

    std::string to_string() const;
};

struct DerefArray {
    std::shared_ptr<Expr> array_expr;
    std::shared_ptr<Expr> deref_expr;

    std::string to_string() const;
};

struct DerefTuple {
    std::shared_ptr<Expr> tuple_expr;
    std::shared_ptr<Expr> deref_expr;

    std::string to_string() const;
};

struct FunCall {
    std::shared_ptr<Expr> fun;
    std::shared_ptr<std::vector<Expr>> fun_args;

    std::string to_string() const;
};

struct TypeExpr;

struct Expr {
    std::dynamic_variant<std::monostate, BinOperation, DerefArray, DerefTuple, FunCall, ID, TypeExpr> var;
    bool minus = false;

    std::string to_string() const;
};

struct PtrExpr {
    // &ref_expr
    // *deref_expr
    // neither -> &$
    std::shared_ptr<Expr> ref_expr;
    std::shared_ptr<Expr> deref_expr;

    std::string to_string() const;
};

struct OptExpr {
    // ?opt_expr
    // !nopt_expr
    // neither -> ?$
    std::optional<Expr> opt_expr;
    std::optional<Expr> nopt_expr;

    std::string to_string() const;
};

struct TupleExpr {
    std::vector<Expr> exprs;

    std::string to_string() const;
};

struct ArrayExpr {
    std::vector<Expr> exprs;

    std::string to_string() const;
};

struct TypeExpr {
    std::variant<int, char, std::string, bool, PtrExpr, OptExpr, TupleExpr, ArrayExpr> expr;

    std::string to_string() const;
};

struct VarDecl;
struct Assign;
struct Flow;

struct FunBodyPart {
    std::dynamic_variant<VarDecl, Assign, Flow> var;

    std::string to_string() const;
};

struct FunBody {
    std::vector<FunBodyPart> parts;

    std::string to_string() const;
};

struct Type {
    enum Primitive {
        UNIT,
        INT,
        CHAR,
        BOOL,
    };

    using Opt   = std::optional<Type>;
    using Ptr   = std::shared_ptr<Type>;
    using Tuple = std::vector<Type>;
    using Array = std::pair<Type, int>;
    using Fun   = std::pair<Tuple, Type>;

    std::dynamic_variant<std::monostate, Primitive, Ptr, Opt, Tuple, Array, Fun> var;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;

    bool is_full_type() const;
    bool empty() const;

    std::string to_string() const;

    bool is_equivalent(const Type& other) const;
};

struct VarDecl {
    // type ids = expr;
    std::optional<Type> type;
    std::optional<std::vector<ID>> ids;
    std::optional<Expr> expr;

    std::string to_string() const;
};

struct Assign {
    // assign_expr = expr
    Expr assign_expr;
    Expr expr;

    std::string to_string() const;
};

struct IfCond {
    // if (expr) body
    Expr expr;
    FunBody body;

    std::string to_string() const;
};

struct Cond {
    std::vector<IfCond> if_conds;
    std::optional<FunBody> else_body;

    std::string to_string() const;
};

struct Loop {
    // while (expr) body
    Expr expr;
    FunBody body;

    std::string to_string() const;
};

struct Flow {
    enum ControlTypes {
        CONTINUE,
        BREAK,
        RETURN,
    };

    using Control = std::pair<ControlTypes, std::optional<Expr>>;

    std::variant<Cond, Loop, Control> var;

    std::string to_string() const;
};

struct FunArg {
    Type type;
    ID id;

    std::string to_string() const;
};

struct FunDecl {
    Type ret_type;
    ID id;
    std::vector<FunArg> args;
    FunBody body;

    std::string to_string() const;
};

struct Program {
    std::vector<FunDecl> decls;

    std::string to_string() const;
};

}

namespace std {

template<>
struct hash<foc::ID> {
    inline size_t operator()(const foc::ID& id) const {
        return hash<std::string>{}(id.name);
    }
};

}
