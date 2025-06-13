#include <gtest/gtest.h>
#include <print>

#include "scan.hpp"

TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");
    ASSERT_TRUE(result);
}

TEST(ScanTest, SimpleTest2) {
    auto result = stdx::scan<float>("1.23", "{%f}");
    ASSERT_TRUE(result);
}

TEST(ScanTest, SimpleTest3) {
    auto result = stdx::scan<float>("1.23", "{%s}");
    ASSERT_FALSE(result);
}

TEST(ScanTest, SimpleTest4) {
    auto result = stdx::scan<int>("123", "{%u}");
    ASSERT_FALSE(result);
}

TEST(ScanTest, SimpleTest5) {
    auto result = stdx::scan<int>("123", "{%d}");
    ASSERT_TRUE(result);
}

//TODO: rewrite as test with values -> 5 correct, 5 incorrect, 15 total

//TODO:
//Сompile-time-проверки на корректность шаблонных параметров и результатов во всех подходящих местах.
//consexr / static_assert?

//Код следует принципам SOLID и KISS.


