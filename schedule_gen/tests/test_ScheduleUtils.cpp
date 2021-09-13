#include "ScheduleUtils.h"

#include <array>
#include <catch2/catch.hpp>


TEST_CASE("Check if two ordered sets intersects", "[algorithms]")
{
    SECTION("Two empty sets do not intersects")
    {
        REQUIRE_FALSE(set_intersects(std::vector<int>{}, std::vector<int>{}));
    }
    SECTION("If one of sets is empty - sets are not intersects")
    {
        constexpr std::array firstSet = {1, 2, 3};
        std::vector<int> secondSet;
        REQUIRE_FALSE(set_intersects(firstSet, secondSet));
        REQUIRE_FALSE(set_intersects(secondSet, firstSet));
    }
    SECTION("set_intersects function works correctly")
    {
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{1, 2, 3}));
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{2, 3}));
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{1, 2}));
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{1}));
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{2}));
        REQUIRE(set_intersects(std::array{1, 2, 3}, std::array{3}));
        REQUIRE(set_intersects(std::array{2, 4, 5, 6, 7, 9}, std::array{8, 9}));

        REQUIRE_FALSE(set_intersects(std::array{1, 3, 5}, std::array{2, 4, 6}));
        REQUIRE_FALSE(set_intersects(std::array{1, 3, 5}, std::array{0}));
        REQUIRE_FALSE(set_intersects(std::array{1, 3, 5}, std::array{9, 10}));
    }
}

TEST_CASE("Check if container contains elements", "[algorithms]")
{
    constexpr std::array arr{4, 2, 3, 6, 1, 8, 10, 43, 22, 21};
    for(auto v : arr)
        REQUIRE(contains(arr, v));

    REQUIRE_FALSE(contains(arr, 0));
    REQUIRE_FALSE(contains(arr, 20));
    REQUIRE_FALSE(contains(arr, 34));
    REQUIRE_FALSE(contains(arr, 134));
}
