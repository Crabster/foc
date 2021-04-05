#include "id_context.hpp"
#include <iostream>

namespace foc {

std::optional<Type> IDContext::find_type(const ID& id) const {
    auto it = type_decls.find(id);
    if (it != type_decls.end()) {
        return it->second;
    }
    if (!parent_context) {
        return {};
    }
    return parent_context->find_type(id);
}

bool IDContext::is_declared(const ID& id) const {
    auto it = type_decls.find(id);
    if (it != type_decls.end()) {
        return true;
    }
    if (!parent_context) {
        return false;
    }
    return parent_context->is_declared(id);
}

void IDContext::add_context(const ID& id, const Type& type) {
    if (is_declared(id)) {
        std::cerr << "Warning: shadowing name `" << id.name << "`." << std::endl;
    }
    type_decls[id] = type;
}

bool IDContext::add_strict_context(const ID& id, const Type& type) {
    bool was_declared = is_declared(id);
    if (was_declared) {
        std::cerr << "ShadowError: name `" << id.name << "` should have type `" <<
        find_type(id)->to_string() << "` as well as `" << type.to_string() << "`." << std::endl;
    }
    type_decls[id] = type;
    return !was_declared;
}

bool IDContext::add_contexts(const std::vector<ID>& ids, const Type& type) {
    if (!type.array_type && !type.tuple_type) {
        std::cerr << "TieError: trying to use tie for non-array and non-tuple typle." << std::endl;
        return false;
    }
    if (type.array_type) {
        if (type.array_type->second != ids.size()) {
            std::cerr << "TieError: try to use tie for wrong sized array" << std::endl;
            return false;
        }
        for (const auto& id : ids) {
            add_context(id, type.array_type->first);
        }
        return true;
    }
    // type.tuple_type
    if (type.tuple_type->size() != ids.size()) {
        std::cerr << "TieError: try to use tie for wrong sized tuple" << std::endl;
        return false;
    }
    for (unsigned i = 0; i < ids.size(); ++i) {
        add_context(ids[i], type.tuple_type->at(i));
    }
    return true;
}

}