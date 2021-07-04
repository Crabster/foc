#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <stdexcept>
#include <functional>
#include <variant>

#include "util.hpp"

namespace foc {

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

struct ID {
    std::string name;

    bool operator==(const ID& other) const;
    bool operator!=(const ID& other) const;
};

struct TypeExpr;

struct Expr {
    std::shared_ptr<Expr> primary_expr;
    std::shared_ptr<Expr> secondary_expr;

    std::optional<Operator> op;
    std::shared_ptr<std::vector<Expr>> fun_args;
    bool deref_array;
    bool deref_tuple;

    std::shared_ptr<TypeExpr> type_expr;
    bool minus;
    std::optional<ID> id;
};

/*
struct Expr {
    variant<BinOperation, DerefArr, DerefTuple, FunCall, ID, TypeExpr>
    bool minus;
}
*/

struct PtrExpr {
    // &ref_expr
    // *deref_expr
    // neither -> &$
    std::shared_ptr<Expr> ref_expr;
    std::shared_ptr<Expr> deref_expr;
};

struct OptExpr {
    // ?opt_expr
    // !nopt_expr
    // neither -> ?$
    std::optional<Expr> opt_expr;
    std::optional<Expr> nopt_expr;
};

struct TupleExpr {
    std::vector<Expr> exprs;
};

struct ArrayExpr {
    std::vector<Expr> exprs;
};

struct TypeExpr {
    std::variant<int, char, std::string, bool, PtrExpr, OptExpr, TupleExpr, ArrayExpr> expr;
};

struct VarDecl;
struct Assign;
struct Flow;

struct FunBodyPart {
    // imo variant
    std::shared_ptr<VarDecl> decl;
    std::shared_ptr<Assign> assign;
    std::shared_ptr<Flow> flow;
};

struct FunBody {
    std::vector<FunBodyPart> parts;
};

struct Type {
    enum Primitive {
        INT,
        CHAR,
        BOOL,
    };

    using Opt   = std::optional<Type>;
    using Ptr   = std::shared_ptr<Type>;
    using Tuple = std::vector<Type>;
    using Array = std::pair<Type, int>;
    using Fun   = std::pair<Type, Type>;

    std::dynamic_variant<std::monostate, Primitive, Ptr, Opt, Tuple, Array, Fun> var;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;

    bool is_full_type() const;
    bool empty() const;

    std::string to_string() const {
        return "TODO";
    }
};

struct VarDecl {
    // type ids = expr;
    std::optional<Type> type;
    std::optional<std::vector<ID>> ids;
    std::optional<Expr> expr;
};

struct Assign {
    // assign_expr = expr
    Expr assign_expr;
    Expr expr;
};

struct IfCond {
    // if (expr) body
    Expr expr;
    FunBody body;
};

struct Cond {
    std::vector<IfCond> if_conds;
    std::optional<FunBody> else_body;
};

struct Loop {
    // while (expr) body
    Expr expr;
    FunBody body;
};

struct Flow {
    // We need to remember what we are returning!
    enum Control {
        CONTINUE,
        BREAK,
        RETURN,
    };

    std::variant<Cond, Loop, Control> var;
    // Control -> std::pair<Control, std::opt<Expr>>
};


struct FunArg {
    Type type;
    ID id;
};

struct FunDecl {
    Type ret_type;
    ID id;
    std::vector<FunArg> args;
    FunBody body;
};

struct Program {
    std::vector<FunDecl> decls;
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
