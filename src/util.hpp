#include <variant>
#include <memory>

namespace std {

template <class... Types>
class dynamic_variant {
public:
    template <class T>
    constexpr dynamic_variant<Types...>& operator=(T&& t) noexcept {
        m_var = std::make_shared<T>(t);
        return *this;
    }

    template <class Visitor>
    void visit(Visitor&& vis) const {
        std::visit([&](std::shared_ptr<auto> arg) { return vis(*arg); }, m_var);
    }


    bool operator==(const dynamic_variant& other) const {
        return m_var == other.m_var;
    }

    std::variant<std::shared_ptr<Types>...> m_var;
};


template <class T, class... Types>
constexpr T& get(dynamic_variant<Types...>& v) {
    return *std::get<std::shared_ptr<T>>(v.m_var);
}

template <class T, class... Types>
constexpr const T& get(const dynamic_variant<Types...>& v) {
    return *std::get<std::shared_ptr<T>>(v.m_var);
}

/*
template <class R, class Visitor, class... Types>
constexpr R visit(Visitor&& vis, dynamic_variant<Types...>& v) {
    return std::visit([&](auto&& arg) { return vis(*arg); }, v.m_var);
}
*/

template <class T, class... Types>
constexpr bool holds_alternative(const dynamic_variant<Types...>& v) noexcept {
    return std::holds_alternative<std::shared_ptr<T>>(v.m_var);
}

}
