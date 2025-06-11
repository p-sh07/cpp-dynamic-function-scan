#pragma once

#include <expected>
#include <charconv>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {
struct FormatChars {
    static constexpr char Int      = 'd';
    static constexpr char Uint     = 'u';
    static constexpr char String   = 's';
    static constexpr char FloatDbl = 'f';
};

//Could have used is_integral, but that includes char & bool
template<typename T>
concept integer_number = std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>
                         || std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>;

template<typename T>
concept natural_number = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>
                         || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>;
template<typename T>
concept float_number = std::is_floating_point_v<T>;

template<typename T>
concept whole_number = integer_number<T> || natural_number<T>;

template<typename T>
concept number = whole_number<T> || std::is_floating_point_v<T>;

template<typename T>
concept string_type = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

template<typename T>
concept valid_type = number<T> || string_type<T>;

template<typename T>
std::expected<T, scan_error> parse_value(std::string_view input) requires whole_number<T> {
    T value;
    const char* first = input.data();
    const char* last  = input.data() + input.size();

    // https://cppreference.com/w/cpp/utility/from_chars.html
    auto result = std::from_chars(first, last, value);

    if (result.ptr != last) {
        return std::unexpected(scan_error{"failed to parse Int"});
    }

    if (result.ec == std::errc::invalid_argument) {
        return std::unexpected(scan_error{"Failed to parse Int: invalid argument"});
    }

    if (result.ec == std::errc::result_out_of_range) {
        return std::unexpected(scan_error{"Failed to parse Int: result out of range"});
    }

    return value;
}

template<typename T>
std::expected<T, scan_error> parse_value(std::string_view input) requires float_number<T> {
    //TODO: in clang from_chars doesn't work with float - known bug
    std::string float_str{input};
    return static_cast<T>(std::stod(float_str));
}

template<string_type T>
std::expected<T, scan_error> parse_value(std::string_view input) {
    return T{input};
}

template<valid_type T>
std::expected<T, scan_error> parse_value(std::string_view input, char fmt) {
    if (fmt == FormatChars::Int && !integer_number<T>) {
        return std::unexpected(scan_error{"Using wrong type with Int format"});
    }
    if (fmt == FormatChars::Uint && !natural_number<T>) {
        return std::unexpected(scan_error{"Using wrong type with Unsigned int format"});
    }
    if (fmt == FormatChars::FloatDbl && !float_number<T>) {
        return std::unexpected(scan_error{"Using wrong type with Floating pt. format"});
    }

    return parse_value<T> (input);
}

// Функция для парсинга значения с учетом спецификатора формата
template<typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    // здесь ваш код
    if (input.size() == 0) {
        return std::unexpected(scan_error{"Empty input to parse"});
    }

    //NB: only single char expected for format
    if (fmt.size() > 1) {
        return std::unexpected(scan_error{"Incorrect Format in parse"});
    }

    return fmt.empty()
               ? parse_value<T>(input, ' ')
               : parse_value<T>(input, fmt[0]);

}

/**
 * 2. В файле parse.hpp изучите реализацию функции parse_sources, которая возвращает пару массивов
 * подстрок форматирующей и исходной строки. Элементы массивов с одинаковыми индексами соответствуют
 * плейсхолдеру в форматирующей строке и релевантной ему подстроке в строке с исходными данными.
 * В parse_sources используется шаблонный класс std::expected. Перед его использованием прочтите
 * документацию: https://en.cppreference.com/w/cpp/utility/expected.html
 *
 */

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template<typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts; // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos                 = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos                          = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{input_parts, format_parts};
}

} // namespace stdx::details
