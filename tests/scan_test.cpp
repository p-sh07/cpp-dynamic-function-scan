#include <gtest/gtest.h>

#include <print>

#include "scan.hpp"
#include "types.hpp"


namespace {
using std::literals::operator""sv;

using stdx::scan;
using stdx::details::scan_result;

using ::testing::TestWithParam;
using ::testing::Values;

//====== Correct tests:
//Simple test, no extra text
TEST(ScanTest, test_correct_1) {
    int8_t expected{42};
    auto result = scan<int>("42", "{}");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(std::get<0>(result->values()), expected);
}

//Empty fmt, short test, int / float
TEST(ScanTest, test_correct_2) {
    std::tuple<int, float> expected{1, 3.14};
    auto result = scan<int, float>("hello 1 this is 3.14 scan", "hello {} this is {} scan");

    ASSERT_TRUE(result);
    ASSERT_EQ(result->values(), expected);
}

//Empty fmt, long test, int8_t / int32_t / uint16_t / uint64_t / double / std::string
TEST(ScanTest, test_correct_3) {
    std::tuple<int, float> expected{1, 3.14};
    auto result = scan<int, float>("hello 1 this is 3.14 scan", "hello {} this is {} scan");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}

//With correct fmt tokens, short test, int16_t / uint8_t
TEST(ScanTest, test_correct_4) {
    std::tuple<int, float> expected{1, 3.14};
    auto result = scan<int, float>("hello 1 this is 3.14 scan", "hello {} this is {} scan");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}

//With correct fmt tokens, long test, uint32_t / int64_t /std::strig / std::string_view
TEST(ScanTest, test_correct_5) {
    std::tuple<int, float> expected{1, 3.14};
    auto result = scan<int, float>("hello 1 this is 3.14 scan", "hello {} this is {} scan");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->values(), expected);
}


//====== Error tests:
// Mismatched fmt tokens
TEST(ScanTest, test_incorrect_1) {
    auto result = scan<int, float>("hello 1 this is 3.14 scan", "hello {%f} this is {%d} scan");
    std::cerr << result.error().message << std::endl;
    ASSERT_FALSE(result.has_value());
}

//Invalid fmt tokens
TEST(ScanTest, test_incorrect_2) {
    auto result = scan<double, uint64_t>("hello 1 this is 3.14 scan", "hello {%a} this is {$s} scan");
    std::cerr << result.error().message << std::endl;
    ASSERT_FALSE(result.has_value());
}

//Invalid input data
TEST(ScanTest, test_incorrect_3) {
    auto result = scan<float, int32_t>("hello there, this is 3.14 scan", "hello {%f} this is {%d} scan");
    std::cerr << result.error().message << std::endl;
    ASSERT_FALSE(result.has_value());
}

//Invalid input data - int overflow
TEST(ScanTest, test_incorrect_4) {
    auto result = scan<float, int8_t>("hello 1.1 this is 129 scan", "hello {%f} this is {%d} scan");
    std::cerr << result.error().message << std::endl;
    ASSERT_FALSE(result.has_value());
}

//Invalid input data - wrong fmt placement in text
TEST(ScanTest, test_incorrect_5) {
    auto result = scan<float, uint32_t>("hello 1.1 this is 2147483647 scan", "hello this {%f} is scan {%u}");
    std::cerr << result.error().message << std::endl;
    ASSERT_FALSE(result.has_value());
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
//     ASSERT_EQ(cg_.EncryptFile(src_, encrypted_, password_));
//     EXPECT_NO_THROW(cg_.DecryptFile(encrypted_, decrypted_, password_));
//
//     EXPECT_EQ(GetSrcStr(), GetDecryptedStr());
// }
//
// INSTANTIATE_TEST_SUITE_P(
//
// );
} //namespace
