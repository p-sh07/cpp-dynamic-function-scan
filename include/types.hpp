#pragma once

namespace stdx::details {

struct scan_error {
    std::string message;
};

template <typename... Ts>
struct scan_result {

    template<typename... Us>
    scan_result(scan_result<Us...> && other) noexcept
    requires(std::is_same_v<scan_result, scan_result<Us...>>) : values(std::forward<Us...>(other.values)) {}

    scan_result(scan_result&& other) noexcept : values(std::move(other.values)) {}

    template<typename... Args>
    scan_result(Args&&... args) noexcept : values(std::forward<Args>(args)...) {}

    scan_result(Ts&&... args) noexcept : values(std::move(args)...) {}

    std::tuple<Ts...> values;
};
} // namespace stdx::details