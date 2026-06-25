#pragma once

#include "graphic/Types.hpp"
#include "util/Overloaded.hpp"
#include "event/AllEvent.hpp"
#include <type_traits>
#include <variant>

namespace event {

template<typename T> struct is_variant : std::false_type {};
template<typename... Ts> struct is_variant<std::variant<Ts...>> : std::true_type  {};
template<typename T> inline constexpr bool is_variant_v = is_variant<T>::value;

template<typename V, typename... Handlers>
    requires is_variant_v<V>
void on(const V& ev, Handlers&&... handlers)
{
    auto vis = overloaded{handlers..., [](const auto&){}};
    std::visit([&](const auto& inner) {
        using T = std::decay_t<decltype(inner)>;
        if constexpr (is_variant_v<T>)
            on(inner, handlers...);
        else
            vis(inner);
    }, ev);
}

} // namespace event
