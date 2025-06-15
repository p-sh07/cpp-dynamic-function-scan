#pragma once

#include <expected>
#include <charconv>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {
using std::literals::operator ""s;
using std::literals::operator ""sv;

//================ Make std::unexpected error shorthand ==============
auto make_scan_error(std::string message) {
    return std::unexpected( scan_error{std::move(message)} );
}

//================ Token processing ================
static constexpr char FORMAT_ESCAPE_CHAR = '%';

enum class FormatToken : char {
    Int = 'd',
    Uint = 'u',
    String  = 's',
    FloatDbl = 'f',
    Empty = ' ',
};

//More flexible than using char - can use other escape symbol, multi-letter fmt token, etc.
std::expected<FormatToken, scan_error> strview_to_token(std::string_view fmt) {
    if(fmt.empty()) {
        return FormatToken::Empty;
    }
    if(fmt.size() != 2 || fmt[0] != FORMAT_ESCAPE_CHAR) {
        return make_scan_error("Invalid Format token: " + std::string(fmt));
    }
    return static_cast<FormatToken>(fmt[1]);
}

//================ Types restrictions for parsing/scan ================
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


//================ Parsing functions ================
template<typename T>
std::expected<T, scan_error> parse_value(std::string_view input) requires whole_number<T> {
    T value;
    const char* first = input.data();
    const char* last  = input.data() + input.size();

    // https://cppreference.com/w/cpp/utility/from_chars.html
    auto result = std::from_chars(first, last, value);

    if (result.ptr != last) {
        return make_scan_error("failed to parse integer number"s);
    }

    if (result.ec == std::errc::invalid_argument) {
        return make_scan_error("Failed to parse integer number: invalid argument"s);
    }

    if (result.ec == std::errc::result_out_of_range) {
        return make_scan_error("Failed to parse integer number: result out of range"s);
    }

    return value;
}

template<typename T>
std::expected<T, scan_error> parse_value(std::string_view input) requires float_number<T> {
    //TODO: in clang from_chars doesn't work with float - known bug
    try {
        return static_cast<T>(std::stod(std::string{input}));
    } catch (std::exception& ex) {
        return make_scan_error("Failed to parse decimal number: "s + ex.what());
    }
}

template<string_type T>
std::expected<T, scan_error> parse_value(std::string_view input) {
    return T{input};
}

template<typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    //check that not a ref or pointer type - static assert or return std::unexpected is better?
    static_assert(!std::is_reference_v<T> && !std::is_pointer_v<T>);

    if (input.size() == 0) {
        return make_scan_error("Empty input to parse"s);
    }

    auto token = strview_to_token(fmt);
    if(!token) {
        return std::unexpected(token.error());
    }

    switch (token.value()) {
        case FormatToken::Int:
            if (!integer_number<T>) {
                return make_scan_error("Using wrong type with Int format token"s);
            }
        break;

        case FormatToken::Uint:
            if (!natural_number<T>) {
                return make_scan_error("Using wrong type with Unsigned int format token"s);
            }
        break;

        case FormatToken::FloatDbl:
            if (!float_number<T>) {
                return make_scan_error("Using wrong type with Floating pt. format token"s);
            }
        break;

        case FormatToken::String:
            if (!string_type<T>) {
                return make_scan_error("Using wrong type with string format token"s);
            }
        break;

        case FormatToken::Empty:
            if (!valid_type<T>) {
                return make_scan_error("Invalid type used with scan function token"s);
            }
        break;

        default:
            return make_scan_error("Unsupported format token"s);
    }

    return parse_value<T>(input);

}

using parse_source_result = std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>;

template<typename... Ts>
parse_source_result parse_sources(std::string_view input, std::string_view format) {
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
                return make_scan_error("Unformatted text in input and format string are different"s);
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
            return make_scan_error("Unformatted text in input and format string are different"s);
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{input_parts, format_parts};
}

} // namespace stdx::details
