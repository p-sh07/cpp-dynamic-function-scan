#pragma once

namespace stdx::details {

struct scan_error {
    std::string message;
};

template <typename... Ts>
struct scan_result {
    scan_result() = default;

    template<typename... Args>
    scan_result(Args&&... args) : data(std::make_tuple(args...)) {}

    std::tuple<Ts...> data;

    const std::tuple<Ts...>& values() const {
        return data;
    };

    std::tuple<Ts...>& values() {
        return data;
    };

};

} // namespace stdx::details