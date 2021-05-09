#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <stdexcept>
#include <functional>

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

struct Type {
    enum Primitive {
        INT,
        CHAR,
        BOOL,
    };

    std::optional<Primitive> prim_type;
    std::shared_ptr<Type> ptr_type;
    std::shared_ptr<Type> opt_type;
    std::shared_ptr<std::vector<Type>> tuple_type;
    std::shared_ptr<std::pair<Type, int>> array_type;
    std::shared_ptr<std::pair<Type, Type>> fun_type;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;

    bool is_full_type() const;
    bool empty() const;

    std::string to_string() const {
        return "TODO";
    }
};

struct ID {
    std::string name;

    bool operator==(const ID& other) const;
    bool operator!=(const ID& other) const;
};

struct Operation;
struct FunCall;
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

struct PtrExpr {
    std::shared_ptr<Expr> ref_expr;
    std::shared_ptr<Expr> deref_expr;
};

struct OptExpr {
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
    std::optional<int> int_expr;
    std::optional<char> char_expr;
    std::optional<std::string> str_expr;
    std::optional<bool> bool_expr;
    std::optional<PtrExpr> ptr_expr;
    std::optional<OptExpr> opt_expr;
    std::optional<TupleExpr> tuple_expr;
    std::optional<ArrayExpr> array_expr;
};

struct FunCall {
    Expr expr;
    std::vector<Expr> args;
};

struct AssignExpr {
    Expr expr;
    std::optional<Expr> idx_expr;
};

struct VarDecl;
struct Assign;
struct Flow;

struct FunBodyPart {
    std::shared_ptr<VarDecl> decl;
    std::shared_ptr<Assign> assign;
    std::shared_ptr<Flow> flow;
};

struct FunBody {
    std::vector<FunBodyPart> parts;
};

struct VarDecl {
    std::optional<Type> type;
    std::optional<std::vector<ID>> ids;
    std::optional<Expr> expr;
};

struct Assign {
    Expr assign_expr;
    Expr expr;
};

struct IfCond {
    Expr expr;
    FunBody body;
};

struct Cond {
    std::vector<IfCond> if_conds;
    std::optional<FunBody> else_body;
};

struct Loop {
    Expr expr;
    FunBody body;
};

struct Flow {
    enum Control {
        CONTINUE,
        BREAK,
        RETURN,
    };

    std::optional<Cond> cond;
    std::optional<Loop> loop;
    std::optional<Control> control;
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
