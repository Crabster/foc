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

    std::string to_string() const {
        return name;
    }
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
};

struct DerefArray {
    std::shared_ptr<Expr> array_expr;
    std::shared_ptr<Expr> deref_expr;
};

struct DerefTuple {
    std::shared_ptr<Expr> tuple_expr;
    std::shared_ptr<Expr> deref_expr;
};

struct FunCall {
    std::shared_ptr<Expr> fun;
    std::shared_ptr<std::vector<Expr>> fun_args;
};

struct TypeExpr;

struct Expr {
    std::dynamic_variant<std::monostate, BinOperation, DerefArray, DerefTuple, FunCall, ID, TypeExpr> var;
    bool minus;

    std::string to_string() const {
        return "TODO_E";
    }
};

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
inline std::string d_to_string(const VarDecl& vd);
struct Assign;
inline std::string d_to_string(const Assign& ass);
struct Flow;
inline std::string d_to_string(const Flow& flow, const std::string& lb);

struct FunBodyPart {
    std::dynamic_variant<VarDecl, Assign, Flow> var;

    std::string to_string(const std::string& line_beg) const {
        if (std::holds_alternative<VarDecl>(var)) {
            return d_to_string(std::get<VarDecl>(var));
        } else if (std::holds_alternative<Assign>(var)) {
            return d_to_string(std::get<Assign>(var));
        } else if (std::holds_alternative<Flow>(var)) {
            return d_to_string(std::get<Flow>(var), line_beg);
        } else {
            throw std::logic_error("Error in FunBodyPart::to_string -- holds_alternative didnt catch");
        }
    }
};

struct FunBody {
    std::vector<FunBodyPart> parts;

    std::string to_string(const std::string& line_beg) const {
        std::string res{};
        for (const auto& part : parts) {
            res += line_beg;
            res += part.to_string(line_beg);
            res += "\n";
        }
        return res;
    }
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
        return "TODO_T";
    }
};

struct VarDecl {
    // type ids = expr;
    std::optional<Type> type;
    std::optional<std::vector<ID>> ids;
    std::optional<Expr> expr;

    std::string to_string() const {
        std::string res{};
        std::cout << "Still fine!" << std::endl;
        if (type) {
            std::cout << "We good?" << std::endl;
            res += type->to_string();
            std::cout << "Def wont see this" << std::endl;
        } else {
            std::cout << "Weeee" << std::endl;
            res += "_";
        }
        res += " ";

        if (ids) {
            std::string delim = "";
            for (const auto& id : *ids) {
                res += delim;
                res += id.to_string();
                delim = ", ";
            }
        } else {
            res += "_";
        }

        if (expr) {
            res += " = ";
            res += expr->to_string();
        }

        res += ";";
        return res;
    }
};

inline std::string d_to_string(const VarDecl& vd) {
    return vd.to_string();
}

struct Assign {
    // assign_expr = expr
    Expr assign_expr;
    Expr expr;

    std::string to_string() const {
        std::string res{};
        res += assign_expr.to_string();
        res += " = ";
        res += expr.to_string();
        res += ";";
        return res;
    }
};

inline std::string d_to_string(const Assign& ass) {
    return ass.to_string();
}


struct IfCond {
    // if (expr) body
    Expr expr;
    FunBody body;

    std::string to_string(const std::string& line_beg) const {
        std::string res{};
        res += line_beg + "IF (";
        res += expr.to_string();
        res += ") {\n";
        std::string new_lb = "  " + line_beg;
        res += body.to_string(new_lb);
        res += line_beg + "}\n";
        return res;
    }
};

struct Cond {
    std::vector<IfCond> if_conds;
    std::optional<FunBody> else_body;

    std::string to_string(const std::string& line_beg) const {
        std::string res{};
        std::string new_lb = "  " + line_beg;
        std::string delim = line_beg;
        for (const auto& if_c : if_conds) {
            res += delim;
            res +=  if_c.to_string(new_lb);
            delim = line_beg + "ELSE ";
        }
        if (else_body) {
            res += line_beg + "ELSE {\n";
            res += else_body->to_string(new_lb);
        }
        return res;
    }
};

struct Loop {
    // while (expr) body
    Expr expr;
    FunBody body;

    std::string to_string(const std::string& line_beg) const {
        std::string res{};
        res += line_beg + "WHILE (";
        res += expr.to_string();
        res += ") {\n";
        std::string new_lb = line_beg + "  ";
        res += body.to_string(new_lb);
        res += "}\n";
        return res;
    }
};

struct Flow {
    enum ControlTypes {
        CONTINUE,
        BREAK,
        RETURN,
    };

    using Control = std::pair<ControlTypes, std::optional<Expr>>;

    std::variant<Cond, Loop, Control> var;

    std::string to_string(const std::string& line_beg) const {
        if (std::holds_alternative<Control>(var)) {
            const auto& ctrl = std::get<Control>(var);
            std::string ret_value{};
            switch (ctrl.first)
            {
            case ControlTypes::CONTINUE:
                return "CONTINUE;";
            case ControlTypes::BREAK:
                return "BREAK;";
            case ControlTypes::RETURN:
                ret_value = ctrl.second ? " " + ctrl.second->to_string() : "";
                return "RETURN" + ret_value + ";";
            default:
                throw std::logic_error("Flow::to_string -- ctrl types non-exhaustive");
            }
        } else if (std::holds_alternative<Cond>(var)) {
            return std::get<Cond>(var).to_string(line_beg);
        } else if (std::holds_alternative<Loop>(var)) {
            return std::get<Loop>(var).to_string(line_beg);
        } else {
            throw std::logic_error("Flow::to_string -- variant smth smth");
        }

        return "";
    }
};

inline std::string d_to_string(const Flow& flow, const std::string& lb) {
    return flow.to_string(lb);
}


struct FunArg {
    Type type;
    ID id;

    std::string to_string() const {
        std::string res{};
        res += type.to_string();
        res += " ";
        res += id.to_string();
        return res;
    }
};

struct FunDecl {
    Type ret_type;
    ID id;
    std::vector<FunArg> args;
    FunBody body;

    std::string to_string() const {
        std::string res{};
        res += ret_type.to_string();
        res += " ";
        res += id.to_string();
        res += "(";
        std::string del = "";
        for (const auto& arg : args) {
            res += del;
            res += arg.to_string();
            del = ", ";
        }
        res += ") {";
        std::string line_beg = "  ";
        res += body.to_string(line_beg);
        res += "}\n";
        return res;
    }
};

struct Program {
    std::vector<FunDecl> decls;

    std::string to_string() const {
        std::string res{};
        for (const auto& decl : decls) {
            res += decl.to_string();
        }
        return res;
    }
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
