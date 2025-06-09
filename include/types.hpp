#pragma once

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

/**
 *
 *  В файле types.hpp определите шаблонный класс scan_result, который будет содержать поле типа
 *  std::tuple для готовых сканированных значений и метод values для удобного доступа
 *  к сканированным значениям.
 *  Там же обратите внимание на класс scan_error, хранящий строку ошибки сканирования.
 */
template <typename... Ts>
struct scan_result {
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

/**
*emplate <typename... Args>
auto make_tuple(Args... args) {
return std::tuple<Args...>(
args...);  // Args... внутри треугольных скобок раскрывает пакет параметров шаблона в параметры класса
// std::tuple, args... раскрывает пакет параметров функции в аргументы конструктора std:tuple
}
*/