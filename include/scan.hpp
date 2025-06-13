#pragma once

#include "parse.hpp"
#include "types.hpp"

//DEBUG
#include <iostream>

namespace stdx {

template<typename... Ts>
using scan_results_or_err = std::expected<details::scan_result<Ts...>, details::scan_error>;

template<typename T, size_t n>
auto process_value(const auto& parsed_inputs, const auto& format_tokens) {
    return details::parse_value_with_format<T>(parsed_inputs[n], format_tokens[n]);
}

template<typename... Values, typename... ScanResults>
scan_results_or_err<Values...> check_if_any_error(ScanResults... results) {

    // If any of args has no value -> error
    if (!(... && results.has_value())) {

        //TODO: test if needs has_value check or if only goes into this branch for errors
        // Collect error messages from all failed arguments
        std::string combined_message;
        ((combined_message += (results.has_value() ? "+" : results.error().message) + "; "), ...);

        return details::make_scan_error(combined_message);
    }
    return details::scan_result<Values...>{results.value()...};
}

//Process all values and return combined error messages if any occired
template <typename... Ts, size_t... Is>
scan_results_or_err<Ts...> process_all(const auto& parsed_inputs, const auto& format_tokens, std::index_sequence<Is...>) {
    return check_if_any_error<Ts...>(process_value<Ts, Is>(parsed_inputs,format_tokens)...);
}

//Like scanf: https://en.cppreference.com/w/c/io/fscanf
template <typename... Ts>
scan_results_or_err<Ts...> scan(std::string_view input, std::string_view format) {

    const auto parse_results = details::parse_sources<Ts...>(input, format);

    if(!parse_results) {
        return std::unexpected(parse_results.error());
    }

    auto parsed_inputs = parse_results->first;
    auto format_tokens = parse_results->second;

    //TODO: convert this into a lambda
    auto result = process_all<Ts...>(parsed_inputs, format_tokens, std::make_index_sequence<sizeof...(Ts)>{});

    return result;
}

} // namespace stdx