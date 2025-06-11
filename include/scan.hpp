#pragma once

#include "parse.hpp"
#include "types.hpp"

namespace stdx {

/**
 * Like scanf: https://en.cppreference.com/w/c/io/fscanf
 */
template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {

    const auto parse_results = details::parse_sources<Ts...>(input, format);

    if(!parse_results) {
        return std::unexpected<details::scan_error>(parse_results.error());
    }

    details::scan_result<Ts...> result;

    auto add_value_to_result = [&result](auto value, size_t& index) {
        std::tuple_cat(result.values(), std::make_tuple(value));
        ++index;
    };

    size_t idx = 0;
    auto parsed_inputs = parse_results->first;
    auto format_tokens = parse_results->second;

    add_value_to_result(details::parse_value_with_format<Ts>(parsed_inputs[idx], format_tokens[idx])..., idx);
    return result;
}

} // namespace stdx