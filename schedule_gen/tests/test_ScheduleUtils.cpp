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

TEST_CASE("Constructing bit vector", "[bit_vector]")
{
    BitVector vec(4);
    REQUIRE_FALSE(vec.get_bit(0));
    REQUIRE_FALSE(vec.get_bit(1));
    REQUIRE_FALSE(vec.get_bit(2));
    REQUIRE_FALSE(vec.get_bit(3));
}

TEST_CASE("Indexing bit vector", "[bit_vector]")
{
    BitVector vec(100);
    for(std::size_t i = 0; i < 100; ++i)
        REQUIRE_FALSE(vec.get_bit(i));

    vec.set_bit(70, true);
    for(std::size_t i = 0; i < 100; ++i)
    {
        if(i == 70)
            REQUIRE(vec.get_bit(i));
        else
            REQUIRE_FALSE(vec.get_bit(i));
    }

    vec.set_bit(72, true);
    for(std::size_t i = 0; i < 100; ++i)
    {
        if(i == 70 || i == 72)
            REQUIRE(vec.get_bit(i));
        else
            REQUIRE_FALSE(vec.get_bit(i));
    }
}
