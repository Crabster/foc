#include "syntax_tree.hpp"

namespace foc {

bool Type::operator==(const Type& other) const {
    return this->var == other.var;
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}


bool Type::is_equivalent(const Type& other) const {
    const auto& o_var = other.var;
    if (std::holds_alternative<std::monostate>(var)) {
        return true;
    }
    if (std::holds_alternative<std::monostate>(o_var)) {
        return true;
    }
    if (std::holds_alternative<Primitive>(var)) {
        return std::holds_alternative<Primitive>(o_var) ;
    }
    if (std::holds_alternative<Ptr>(var)) {
        return std::holds_alternative<Ptr>(o_var)
            && std::get<Ptr>(var)->is_equivalent(*std::get<Ptr>(o_var));
    }
    if (std::holds_alternative<Opt>(var)) {
        return std::holds_alternative<Opt>(o_var)
            && std::get<Opt>(var)->is_equivalent(*std::get<Opt>(o_var));
    }
    if (std::holds_alternative<Tuple>(var)) {
        if (!std::holds_alternative<Tuple>(o_var)) {
            return false;
        }
        const Tuple& tt = std::get<Tuple>(var);
        const Tuple& ot = std::get<Tuple>(o_var);
        if (tt.size() != ot.size()) {
            return false;
        }
        for (unsigned i = 0; i < tt.size(); ++i) {
            if (!tt[i].is_equivalent(ot[i])) {
                return false;
            }
        }
        return true;
    }
    if (std::holds_alternative<Array>(var)) {
        if (!std::holds_alternative<Array>(o_var)) {
            return false;
        }
        if (std::get<Array>(var).second != std::get<Array>(o_var).second) {
            return false;
        }
        return std::get<Array>(var).first.is_equivalent(std::get<Array>(o_var).first);
    }
    if (std::holds_alternative<Fun>(var)) {
        if (!std::holds_alternative<Fun>(o_var)) {
            return false;
        }
        const Fun& ft = std::get<Fun>(var);
        const Fun& ot = std::get<Fun>(o_var);
        if (ft.first.size() !=  ot.first.size()) {
            return false;
        }
        for (unsigned i = 0; i < ft.first.size(); ++i) {
            if (!ft.first[i].is_equivalent(ot.first[i])) {
                return false;
            }
        }
        return ft.second.is_equivalent(ot.second);
    }
    throw std::logic_error("I think I should throw error here");
    return false;
}

bool Type::is_full_type() const {
    if (std::holds_alternative<Primitive>(var)) {
        return true;
    }
    if (std::holds_alternative<Ptr>(var)) {
        return std::get<Ptr>(var)->is_full_type();
    }
    if (std::holds_alternative<Opt>(var)) {
        return std::get<Opt>(var)->is_full_type();
    }
    if (std::holds_alternative<Tuple>(var)) {
        const Tuple& tuple_type = std::get<Tuple>(var);
        for (const auto& sub_type : tuple_type) {
            if (!sub_type.is_full_type()) {
                return false;
            }
        }
        return true;
    }
    if (std::holds_alternative<Array>(var)) {
        return std::get<Array>(var).first.is_full_type();
    }
    if (std::holds_alternative<Fun>(var)) {
        const Fun& fun_type = std::get<Fun>(var);
        for (const auto& sub_type : fun_type.first) {
            if (!sub_type.is_full_type()) {
                return false;
            }
        }
        return fun_type.second.is_full_type();
    }
    return false;
}

bool Type::empty() const {
    return std::holds_alternative<std::monostate>(var);
}

bool ID::operator==(const ID& other) const {
    return name == other.name;
}

bool ID::operator!=(const ID& other) const {
    return !(*this == other);
}

std::string add_indent(const std::string& str, int space_count = 4) {
    std::string res = str;
    res.insert(0, space_count, ' ');
    size_t pos = res.find("\n");
    while (res.find("\n", pos + 1) != std::string::npos) {
        res.insert(pos + 1, space_count, ' ');
        pos = res.find("\n", pos + 1);
    }
    return res;
}

std::string ID::to_string() const {
    return name;
}

std::string Expr::to_string() const {
    std::string m = "";
    if (minus) {
        m = "-";
    }
    if (std::holds_alternative<std::monostate>(var)) {
        return m + "MONO_EXPR";
    } else if (std::holds_alternative<BinOperation>(var)) {
        return m + std::get<BinOperation>(var).to_string();
    } else if (std::holds_alternative<DerefArray>(var)) {
        return m + std::get<DerefArray>(var).to_string();
    } else if (std::holds_alternative<DerefTuple>(var)) {
        return m + std::get<DerefTuple>(var).to_string();
    } else if (std::holds_alternative<FunCall>(var)) {
        return m + std::get<FunCall>(var).to_string();
    } else if (std::holds_alternative<ID>(var)) {
        return m + std::get<ID>(var).to_string();
    } else if (std::holds_alternative<TypeExpr>(var)) {
        return m + std::get<TypeExpr>(var).to_string();
    } else {
        throw std::logic_error("Expr::to_string -- dyn_var out of range");
    }
    return "TODO_E";
}

std::string op_to_string(BinOperation::Operator o) {
    switch (o)
    {
    case BinOperation::Operator::PLUS:
        return "+";
    case BinOperation::Operator::MINUS:
        return "-";
    case BinOperation::Operator::STAR:
        return "*";
    case BinOperation::Operator::SLASH:
        return "/";
    case BinOperation::Operator::IS_EQUAL:
        return "==";
    case BinOperation::Operator::NOT_EQUAL:
        return "!=";
    case BinOperation::Operator::AND:
        return "&&";
    case BinOperation::Operator::OR:
        return "||";
    case BinOperation::Operator::LESS:
        return "<";
    case BinOperation::Operator::GREATER:
        return ">";
    case BinOperation::Operator::LEQ:
        return "<=";
    case BinOperation::Operator::GEQ:
        return ">=";
    default:
        throw std::logic_error("Operator to_string -- enum out of range");
    }
    return "???";
}

std::string BinOperation::to_string() const {
    std::string l = "X";
    std::string r = "X";
    if (left_expr) {
        l = left_expr->to_string();
    }
    if (right_expr) {
        r = right_expr->to_string();
    }
    return l + op_to_string(op) + r;
}

std::string DerefArray::to_string() const {
    std::string l = "X";
    std::string r = "X";
    if (array_expr) {
        l = array_expr->to_string();
    }
    if (deref_expr) {
        r = deref_expr->to_string();
    }
    return l + "[" + r + "]";
}

std::string DerefTuple::to_string() const {
    std::string l = "X";
    std::string r = "X";
    if (tuple_expr) {
        l = tuple_expr->to_string();
    }
    if (deref_expr) {
        r = deref_expr->to_string();
    }
    return l + "<" + r + ">";
}

std::string FunCall::to_string() const {
    std::string l = "X";
    std::string r = "X";
    if (fun) {
        l = fun->to_string();
    }
    if (fun_args) {
        r = "";
        std::string delim = "";
        for (const auto& e : *fun_args) {
            r += delim;
            r += e.to_string();
            delim = ",";
        }
    }
    return l + "(" + r + ")";
}

std::string PtrExpr::to_string() const {
    if (!ref_expr && !deref_expr) {
        return "&$";
    }
    if (ref_expr && deref_expr) {
        throw std::logic_error("Ptr expr have both ref and deref");
    }
    if (ref_expr) {
        return "&" + ref_expr->to_string();
    }
    return "*" + deref_expr->to_string();
}

std::string OptExpr::to_string() const {
    if (!opt_expr && !nopt_expr) {
        return "?$";
    }
    if (opt_expr && nopt_expr) {
        throw std::logic_error("Opt expr have both opt and nopt");
    }
    if (opt_expr) {
        return "?" + opt_expr->to_string();
    }
    return "!" + nopt_expr->to_string();
}

std::string TupleExpr::to_string() const {
    std::string res = "<";
    std::string delim = "";
    for (const auto& p : exprs) {
        res += delim;
        res += p.to_string();
        delim = ",";
    }
    return res + ">";
}

std::string ArrayExpr::to_string() const {
    std::string res = "[";
    std::string delim = "";
    for (const auto& p : exprs) {
        res += delim;
        res += p.to_string();
        delim = ",";
    }
    return res + "]";
}

std::string TypeExpr::to_string() const {
    if (std::holds_alternative<int>(expr)) {
        return std::to_string(std::get<int>(expr));
    } else if (std::holds_alternative<char>(expr)) {
        return std::string{std::get<char>(expr)};
    } else if (std::holds_alternative<std::string>(expr)) {
        return std::get<std::string>(expr);
    } else if (std::holds_alternative<bool>(expr)) {
        if (std::get<bool>(expr))
            return "T";
        else
            return "F";
    } else if (std::holds_alternative<PtrExpr>(expr)) {
        return std::get<PtrExpr>(expr).to_string();
    } else if (std::holds_alternative<OptExpr>(expr)) {
        return std::get<OptExpr>(expr).to_string();
    } else if (std::holds_alternative<TupleExpr>(expr)) {
        return std::get<TupleExpr>(expr).to_string();
    } else if (std::holds_alternative<ArrayExpr>(expr)) {
        return std::get<ArrayExpr>(expr).to_string();
    } else {
        throw std::logic_error("TypeExpr:to_string -- variant out of range");
    }
    return "TODO_TE";
}

std::string FunBodyPart::to_string() const {
    if (std::holds_alternative<VarDecl>(var)) {
        return std::get<VarDecl>(var).to_string();
    } else if (std::holds_alternative<Assign>(var)) {
        return std::get<Assign>(var).to_string();
    } else if (std::holds_alternative<Flow>(var)) {
        return std::get<Flow>(var).to_string();
    } else {
        throw std::logic_error("Error in FunBodyPart::to_string -- holds_alternative didnt catch");
    }
}

std::string FunBody::to_string() const {
    std::string res{};
    for (const auto& part : parts) {
        res += part.to_string();
    }
    return res;
}

std::string Type::to_string() const {
    if (std::holds_alternative<std::monostate>(var)) {
        return "M";
    } else if (std::holds_alternative<Primitive>(var)) {
        const auto& p = std::get<Primitive>(var);
        switch (p)
        {
        case Primitive::UNIT:
            return "U";
        case Primitive::INT:
            return "I";
        case Primitive::CHAR:
            return "C";
        case Primitive::BOOL:
            return "B";
        default:
            throw std::logic_error("Type::to_string -- primitive enum out of range");
        }
    } else if (std::holds_alternative<Ptr>(var)) {
        const auto& p = std::get<Ptr>(var);
        if (!p) {
            return "*$";
        }
        return "*" + p->to_string();
    } else if (std::holds_alternative<Opt>(var)) {
        const auto& p = std::get<Opt>(var);
        if (!p) {
            return "?$";
        }
        return "?" + p->to_string();
    } else if (std::holds_alternative<Tuple>(var)) {
        const auto& p = std::get<Tuple>(var);
        std::string res = "<";
        std::string delim = "";
        for (const auto& t : p) {
            res += delim;
            res += t.to_string();
            delim = ",";
        }
        res += ">";
        return res;
    } else if (std::holds_alternative<Array>(var)) {
        const auto& p = std::get<Array>(var);
        return "[" + p.first.to_string() + "," + std::to_string(p.second) + "]";
    } else if (std::holds_alternative<Fun>(var)) {
        auto p = std::get<Fun>(var);
        Type cheat;
        cheat.var = Type::Tuple(p.first);
        return "(" + cheat.to_string() + "->" + p.second.to_string() + ")";
    } else {
        throw std::logic_error("Type::to_string -- dyn_var out of range");
    }
    return "TODO_T";
}

std::string VarDecl::to_string() const {
    std::string res{};
    if (type) {
        res += type->to_string();
    } else {
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

    res += ";\n";
    return res;
}

std::string Assign::to_string() const {
    std::string res{};
    res += assign_expr.to_string();
    res += " = ";
    res += expr.to_string();
    res += ";\n";
    return res;
}

std::string IfCond::to_string() const {
    std::string res{};
    res += "IF (" + expr.to_string() + ") {\n";
    res += add_indent(body.to_string());
    res += "}\n";
    return res;
}

std::string Cond::to_string() const {
    std::string res{};
    std::string delim{};
    for (const auto& if_c : if_conds) {
        res += delim;
        res +=  if_c.to_string();
        delim = "EL";
    }
    if (else_body) {
        res += "ELSE {\n" + add_indent(else_body->to_string()) + "}\n";
    }
    return res;
}

std::string Loop::to_string() const {
    std::string res{};
    res += "WHILE (" + expr.to_string() + ") {\n";
    res += add_indent(body.to_string());
    res += "}\n";
    return res;
}

std::string Flow::to_string() const {
    if (std::holds_alternative<Control>(var)) {
        const auto& ctrl = std::get<Control>(var);
        std::string ret_value{};
        switch (ctrl.first)
        {
        case ControlTypes::CONTINUE:
            return "CONTINUE;\n";
        case ControlTypes::BREAK:
            return "BREAK;\n";
        case ControlTypes::RETURN:
            ret_value = ctrl.second ? " " + ctrl.second->to_string() : "";
            return "RETURN" + ret_value + ";\n";
        default:
            throw std::logic_error("Flow::to_string -- ctrl types non-exhaustive");
        }
    } else if (std::holds_alternative<Cond>(var)) {
        return std::get<Cond>(var).to_string();
    } else if (std::holds_alternative<Loop>(var)) {
        return std::get<Loop>(var).to_string();
    } else {
        throw std::logic_error("Flow::to_string -- variant smth smth");
    }
}

std::string FunArg::to_string() const {
    std::string res{};
    res += type.to_string();
    res += " ";
    res += id.to_string();
    return res;
}

std::string FunDecl::to_string() const {
    std::string res = ret_type.to_string() + " " + id.to_string() + "(";
    std::string del = "";
    for (const auto& arg : args) {
        res += del;
        res += arg.to_string();
        del = ", ";
    }
    res += ") {\n" + add_indent(body.to_string()) + "}\n";
    return res;
}

std::string Program::to_string() const {
    std::string res{};
    for (const auto& decl : decls) {
        res += decl.to_string() + "\n";
    }
    return res;
}

size_t Type::byte_size() const {
    if (std::holds_alternative<Type::Array>(var)) {
        const Type::Array& array = std::get<Type::Array>(var);
        return array.first.byte_size() * array.second;
    } else if (std::holds_alternative<Type::Tuple>(var)) {
        const Type::Tuple& tuple = std::get<Type::Tuple>(var);

        size_t tuple_size = 0;
        for (int i = 0; i < tuple.size(); ++i) {
            tuple_size += tuple[i].byte_size();
        }
        return tuple_size;
    } else {
        return 8;
    }
}

}
