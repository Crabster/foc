#include "syntax_tree.hpp"

namespace foc {

bool Type::operator==(const Type& other) const {
    return this->var == other.var;
    /*
    if (prim_type.has_value()) {
        return other.prim_type.has_value() &&
            *prim_type == *other.prim_type;
    }
    if (ptr_type) {
        return other.ptr_type && (*ptr_type == *other.ptr_type
            || ptr_type->empty() || other.ptr_type->empty());
    }
    if (opt_type) {
        return other.opt_type && (*opt_type == *other.opt_type
            || opt_type->empty() || other.opt_type->empty());
    }
    if (tuple_type) {
        if (!other.tuple_type || tuple_type->size() != other.tuple_type->size()) {
            return false;
        }
        for (unsigned i = 0; i < tuple_type->size(); ++i) {
            if (tuple_type->at(i) != other.tuple_type->at(i)) {
                return false;
            }
        }
        return true;
    }
    if (array_type) {
        return other.array_type &&
            array_type->first == other.array_type->first &&
            array_type->second == other.array_type->second;
    }
    if (fun_type) {
        return other.fun_type &&
            fun_type->first == other.fun_type->first &&
            fun_type->second == other.fun_type->second;
    }

    throw std::logic_error("Empty type!");
    */
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

}
