#include <gtest/gtest.h>
#include <limits>
#include <print>

#include "scan.hpp"
#include "types.hpp"


namespace {
using std::literals::operator""s;
using std::literals::operator""sv;

using stdx::scan;
using stdx::details::scan_result;

using ::testing::TestWithParam;
using ::testing::Values;

//====== Correct tests:
//Simple test, no extra text
TEST(ScanTest, test_correct_1) {
    int8_t expected{42};
    auto result = scan<int>("42"sv, "{}"sv);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::get<0>(result->values()), expected);
}

//Empty fmt, short test, double / float
TEST(ScanTest, test_correct_2) {
    std::tuple<double, float> expected{-0.0019380412341, 3.1483834};
    auto result = scan<double, float>("hello -0.0019380412341 this is 3.1483834 scan"sv, "hello {} this is {} scan"sv);

    ASSERT_TRUE(result);
    ASSERT_DOUBLE_EQ(std::get<double>(result->values()), std::get<double>(expected));
    ASSERT_FLOAT_EQ(std::get<float>(result->values()), std::get<float>(expected));
}

//Empty fmt, long test, int32_t / int8_t / uint16_t / std::string_view / std::string
TEST(ScanTest, test_correct_3) {
    std::tuple<int32_t, int8_t, uint16_t, const std::string_view, std::string> expected{2147483600, -128, 65530, "some"sv, "each"s};
    auto result = scan<int32_t, int8_t, uint16_t, const std::string_view, std::string>(
        "if I could take 2147483600 apples and make them into -128 pies i could "
        "feed 65530 people. They would get some pie each"sv,
        "if I could take {} apples and make them into {} pies i could "
        "feed {} people. They would get {} pie {}"sv);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}

//With correct fmt tokens, short test, int16_t / uint8_t / uint64_t
TEST(ScanTest, test_correct_4) {
    auto a = std::numeric_limits<int16_t>::min();
    auto b = std::numeric_limits<uint8_t>::max();
    auto c = std::numeric_limits<uint64_t>::max();

    std::tuple expected{a, b, c};
    auto result = scan<int16_t, uint8_t, uint64_t>(
        "just: -32768 some; 255 text@ 18446744073709551615 with// numbers+"sv,
        "just: {%d} some; {%u} text@ {%u} with// numbers+"sv);

    std::cerr << result.error().message << std::endl;
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}

//With correct fmt tokens, const, long test, uint32_t / const int64_t /std::strig / std::string_view
TEST(ScanTest, test_correct_5) {
    std::tuple<uint32_t, int64_t, std::string, const std::string_view> expected{2147483600, -128423423, "a lot of negative"s, "truly a lot"sv};
    auto result = scan<uint32_t, const int64_t, std::string, const std::string_view>(
        "if I could take 2147483600 apples and make them into -128423423 pies i could "
        "feed a lot of negative people. They would get truly a lot of pie each"sv,
        "if I could take {} apples and make them into {} pies i could "
        "feed {} people. They would get {} of pie each"sv);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}


//====== Error tests:
// Mismatched fmt tokens
TEST(ScanTest, test_incorrect_1) {
    auto result = scan<int, float>("hello 1 this is 3.14 scan"sv, "hello {%f} this is {%d} scan"sv);
    ASSERT_FALSE(result.has_value());
    std::cerr << result.error().message << std::endl;
}

//Invalid fmt tokens
TEST(ScanTest, test_incorrect_2) {
    auto result = scan<double, uint64_t>("hello 1 this is 3.14 scan"sv, "hello {%a} this is {$s} scan"sv);
    ASSERT_FALSE(result.has_value());
    std::cerr << result.error().message << std::endl;
}

//Invalid input data
TEST(ScanTest, test_incorrect_3) {
    auto result = scan<float, int32_t>("hello there, this is 3.14 scan"sv, "hello {%f} this is {%d} scan"sv);
    ASSERT_FALSE(result.has_value());
    std::cerr << result.error().message << std::endl;
}

//Invalid input data - int overflow
TEST(ScanTest, test_incorrect_4) {
    auto result = scan<float, int8_t>("hello 1.1 this is 129 scan"sv, "hello {%f} this is {%d} scan"sv);
    ASSERT_FALSE(result.has_value());
    std::cerr << result.error().message << std::endl;
}

//Invalid input data - wrong fmt placement in text
TEST(ScanTest, test_incorrect_5) {
    auto result = scan<float, uint32_t>("hello 1.1 this is 2147483647 scan"sv, "hello this {%f} is scan {%u}"sv);
    ASSERT_FALSE(result.has_value());
    std::cerr << result.error().message << std::endl;
}

//==== Compile-time checks:
//Unsupported types -> doesn't compile with "error: no matching function for call to 'parse_value' "
// TEST(ScanTest, test_incorrect_3) {
//     auto result = scan<char, bool>("hello c this is 1 scan", "hello {} this is {} scan");
//     ASSERT_FALSE(result.has_value());
// }

//Ref types -> doesn't compile with "error: static assertion failed due to requirement '!is_reference_v<int &>' "
// TEST(ScanTest, test_incorrect_4) {
//     auto result = scan<int&, float&>("hello 1 this is 3.14 scan", "hello {} this is {} scan");
//     ASSERT_FALSE(result.has_value());
// }

//Ptr types -> doesn't compile with "error: static assertion failed due to requirement '!std::is_pointer_v<int *>' "
// TEST(ScanTest, test_incorrect_5) {
//     auto result = scan<int*, float*>("hello 1 this is 3.14 scan", "hello {} this is {} scan");
//     ASSERT_FALSE(result.has_value());
// }


//======== Test suite P:
//Test various inputs for no error:
// class TestNoError : public TestWithParam<std::string_view> {
//
// };
//
// //Test various inputs for error return (std::unexpected):
// class TestError : public TestWithParam<std::string_view> {
//
// };

//TODO: can test various inputs for absence/presence of error:
//
// TEST_P(TestCtx, EncryptDecrypt) {
//     auto input = GetParam().first;
//     auto format = GetParam().second;
//      ............
// }
//
// INSTANTIATE_TEST_SUITE_P(
//     //params list
//      ............
// );
} //namespace
