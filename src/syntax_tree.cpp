#include "syntax_tree.hpp"

namespace foc {

bool Type::operator==(const Type& other) const {
    return this->var == other.var;
}

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
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
        return fun_type.first.is_full_type() &&
            fun_type.second.is_full_type();
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
    return "TODO_E";
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

}
