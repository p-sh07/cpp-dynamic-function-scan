#pragma once

#include <expected>
#include <charconv>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {
// template <std::size_t I, typename... Ts>
// struct type_at;
//
// template <std::size_t I, typename T, typename... Ts>
// struct type_at<I, T, Ts...> : type_at<I - 1, Ts...> {};
//
// template <typename T, typename... Ts>
// struct type_at<0, T, Ts...> {
//     using type = T;
// };

// здесь ваш код
//Use Tag Dispatching -> we get type instructions from format, choose type based on tag

struct FormatChars {
    static constexpr char Int = 'd';
    static constexpr char Uint = 'u';
    static constexpr char String = 's';
    static constexpr char FloatDbl = 'f';
};
//
struct IntTag {};      // 'd';
struct UintTag {};     // 'u';
struct StringTag {};   // 's';
struct FloatDblTag {}; // 'f';

// // Traits base class
// template<char FmtTag>
// struct FmtTrait {};
//
// //Traits specializations -> for int/uint use max capacity ?
// template<>
// struct FmtTrait<FormatChars::Int> {
//     using tag = IntTag;
// };
//
// template<>
// struct FmtTrait<FormatChars::Uint> {
//     using tag = UintTag;
// };
//
// template<>
// struct FmtTrait<FormatChars::FloatDbl> {
//     using tag = FloatDblTag;
// };
//
// template<>
// struct FmtTrait<FormatChars::String> {
//     using tag = StringTag;
// };

//Could have used is_integral, but that includes char & bool
template<typename T>
concept integer_number = requires(T const& t){
    std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>
    || std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>;
};

template <integer_number T>
std::expected<T, scan_error> parse_value(std::string_view input, IntTag) {
    T value;
    const char* first = input.data();
    const char* last = input.data() + input.size();

    //https://cppreference.com/w/cpp/utility/from_chars.html
    auto result = std::from_chars(first, last, value);

    if(result.ptr != last) {
        return std::unexpected(scan_error{"failed to parse Int"});
    }

    if (result.ec == std::errc::invalid_argument) {
        return std::unexpected(scan_error{"Failed to parse Int: invalid argument"});
    }

    if (result.ec == std::errc::result_out_of_range) {
        return std::unexpected(scan_error{"Failed to parse Int: result out of range"});
    }

    //TODO: check for int-type overflow? or does from chars already check?
    return value;
}


template<typename T>
concept natural_number = requires(T const& t){
    std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>
    || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>;
};

template <natural_number T>
std::expected<T, scan_error> parse_value(std::string_view input, UintTag) {
    T value;
    const char* first = input.data();
    const char* last = input.data() + input.size();

    //https://cppreference.com/w/cpp/utility/from_chars.html
    auto result = std::from_chars(first, last, value);

    if(result.ptr != last) {
        return std::unexpected(scan_error{"failed to parse Unsigned Int"});
    }

    if (result.ec == std::errc::invalid_argument) {
        return std::unexpected(scan_error{"Failed to parse Unsigned Int: invalid argument"});
    }

    if (result.ec == std::errc::result_out_of_range) {
        return std::unexpected(scan_error{"Failed to parse Unsigned Int: result out of range"});
    }

    //TODO: check for int-type overflow? or does from chars already check?
    return value;
}

template <std::floating_point T>
std::expected<T, scan_error> parse_value(std::string_view input, FloatDblTag) {
    T value;
    const char* first = input.data();
    const char* last = input.data() + input.size();

    auto result = std::from_chars(first, last, value);

    if(result.ptr != last) {
        return std::unexpected(scan_error{"failed to parse Float/Dbl"});
    }

    if (result.ec == std::errc::invalid_argument) {
        return std::unexpected(scan_error{"Failed to parse Float/Dbl: invalid argument"});
    }

    if (result.ec == std::errc::result_out_of_range) {
        return std::unexpected(scan_error{"Failed to parse Float/Dbl: result out of range"});
    }

    return value;
}

template<typename T>
concept string_type = requires(T const& t){
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;
};

template <string_type T>
std::expected<T, scan_error> parse_value(std::string_view input, StringTag) {
    return T{input};
}

template<typename T>
concept valid_type = requires(T const& t)
{
    integer_number<T> || std::is_floating_point_v<T> || string_type<T>;
};

template <valid_type T>
std::expected<T, scan_error> parse_value(std::string_view input, char fmt) {
    switch(fmt) {
        case FormatChars::Int:
            return parse_value<T>(input, IntTag{});

        case FormatChars::Uint:
            return parse_value<T>(input, UintTag{});

        case FormatChars::FloatDbl:
            return parse_value<T>(input, FloatDblTag{});

        case FormatChars::String:
            return parse_value<T>(input, StringTag{});

        default:
            return std::unexpected(details::scan_error{"Invalid format token"});
    }
}

// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    // здесь ваш код
    if(input.size() == 0) {
        return std::unexpected(scan_error{"Empty input to parse"});
    }

    static_assert(fmt.size() != 0);

    //NB: only single char expected for format
    if(fmt.size() != 1) {
        return std::unexpected(scan_error{"Incorrect Format in parse"});
    }

    //TODO: check if format is valid? - probably will not compile if invalid, since used as template type
    return parse_value<T>(input, fmt[0]);

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
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
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
            auto pos = input.find(between);
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
        auto pos = input.find(remaining_format);
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