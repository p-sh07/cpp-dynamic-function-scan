#pragma once

namespace stdx::details {

struct scan_error {
    std::string message;
};

template <typename... Ts>
struct scan_result {
    scan_result() = default;

    template<typename... Args>
    scan_result(Args&&... args) : values(std::make_tuple(args...)) {}

    std::tuple<Ts...> values;
};

} // namespace stdx::details