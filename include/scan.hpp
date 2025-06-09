#pragma once

#include "parse.hpp"
#include "types.hpp"

namespace stdx {

/**
 * Like scanf: https://en.cppreference.com/w/c/io/fscanf
 */

//TODO:
/**
* 4. Pеализуйте главную шаблонную функцию scan:
*    - Функция в качестве шаблонных параметров принимает набор типов, в которые
*    нужно конвертировать исходные данные. В качестве параметров функция принимает
*    форматирующую строку и строку с исходными данными.
*    - scan должна использовать функцию parse_sources, чтобы получить
*    разбиение строк, агрегировать результаты работы parse_value_with_format
*    в объект типа scan_result и возвращать его наружу.
*/

// замените болванку функции scan на рабочую версию
template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {
    //TODO: check if types in Ts are allowed types? Check they are not & or *
    // ****
    // ****

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
}

} // namespace stdx

/**
* 5. Внедрите повсеместную обработку ошибок с помощью стандартного класса std::expected.
* В случае некорректного ввода функции должны возвращать ошибку (unexpected result), в случае корректного ввода — набор преобразованных значений. Исключения использовать запрещено.
*
* Финализируйте работу:
*   - Протестируйте функцию scan для разных входных данных.
*   - Убедитесь, что выходные данные действительно соответствуют ожидаемым значениям.
*   - Убедитесь в правильной обработке пограничных случаев и ошибочных сценариев.
*
* Ключевые функциональные требования:
*   - Функция scan должна сканировать строку с исходными данными на основе форматирующей строки
*   и конвертировать в набор объектов переданных типов.
*   - Форматирующая строка должна поддерживать следующие converison specifiers:
*   - d — в исходной строке на месте плейсхолдера находится целое число;
*   - s — в исходной строке на месте плейсхолдера находится строка;
*   - u — в исходной строке на месте плейсхолдера находится натуральное число;
*   - f — в исходной строке на месте плейсхолдера находится число с плавающей точкой.
*   - Функция scan должна поддерживать сканирование данных из исходной строки в следующий набор типов:
*   int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double,
*   std::string_view и std::string, а также в cv-квалифицированные версии этих типов.
*   Использование других типов, в том числе ссылочных версий типов, нужно запретить.
*   - Функция scan должна возвращать либо корректный результат, либо объект ошибки с сообщением о том,
*   что пошло не так.
*/