#pragma once

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    scan_result() = default;

    //TODO: does make_tuple forward args?
    template<typename... Args>
    scan_result(Args&&... args) : data(std::make_tuple(args...)) {}

    std::tuple<Ts...> data;

    //TODO: is this all that is needed?
    const std::tuple<Ts...>& values() const {
        return data;
    };

    std::tuple<Ts...>& values() {
        return data;
    };

};

} // namespace stdx::details