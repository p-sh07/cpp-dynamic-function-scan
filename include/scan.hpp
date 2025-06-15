#pragma once

#include "parse.hpp"
#include "types.hpp"

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

        // Collect error messages from all failed arguments //results.has_value() ? "+" :
        std::string combined_message;
        ((combined_message += (results.error().message) + "; "), ...);

        return details::make_scan_error(combined_message);
    }
    return details::scan_result<Values...>{results.value()...};
}

//Like scanf: https://en.cppreference.com/w/c/io/fscanf
template <typename... Ts>
scan_results_or_err<Ts...> scan(std::string_view input, std::string_view format) {
    const auto parse_results = details::parse_sources<Ts...>(input, format);

    if(!parse_results) {
        return std::unexpected(parse_results.error());
    }

    /**
     * This lambda makes an index sequence 'Idxs' 0 -> the number of arguments Ts
     * And performs 'process_value' for all of the parse_results vector contents
     * using NTTP <size_t n> as index for input and fmt vectors, parsing it into the corresponding type in Ts
     */
    return [&]<std::size_t...Idxs>(std::index_sequence<Idxs...>) {
        return check_if_any_error<Ts...>(
            process_value<Ts, Idxs>(parse_results->first, parse_results->second)...);
    }(std::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace stdx