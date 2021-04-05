#pragma once

#include "syntax_tree.hpp"
#include <unordered_map>

namespace foc {

struct IDContext {
    std::shared_ptr<IDContext> parent_context;
    std::unordered_map<ID, Type> type_decls;

    std::optional<Type> find_type(const ID& id) const;
    bool is_declared(const ID& id) const;
    void add_context(const ID& id, const Type& type);
    bool add_strict_context(const ID& id, const Type& type);
    bool add_contexts(const std::vector<ID>& id, const Type& type);
};

}