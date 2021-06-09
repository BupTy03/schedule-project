#include "ScheduleUtils.hpp"
#include <array>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>


TEST_CASE("Test.SortedSet.construct", "[SortedSet]")
{
    std::array arr = {4, 6, 2, 9, 1, 3, 5, 7, 8, 0,
                               4, 2, 6, 1, 3, 0, 7, 5, 2, 6};
    std::array expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SortedSet<int> sortedSet(arr.begin(), arr.end());
    REQUIRE(sortedSet.size() == expected.size());
    REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
}

TEST_CASE("Test.SortedSet.contains", "[SortedSet]")
{
    SortedSet<int> emptySet;
    for(auto e : {1, 2, 3, 6})
        REQUIRE_FALSE(emptySet.contains(e));

    std::array arr = {4, 6, 2, 9, 0, 6, 1, 0, 5, 1};
    SortedSet<int> sortedSet(arr.begin(), arr.end());
    for(std::size_t i = 0; i < 100; ++i)
    {
        if(std::find(arr.begin(), arr.end(), i) == arr.end())
            REQUIRE_FALSE(sortedSet.contains(i));
        else
            REQUIRE(sortedSet.contains(i));
    }
}

TEST_CASE("Test.SortedSet.insert", "[SortedSet]")
{
    const std::pair<int, std::vector<int>> testCases[] = {
        {0, {0}},
        {9, { 0, 9 }},
        {4, { 0, 4, 9 }},
        {-1, { -1, 0, 4, 9 }},
        {2, { -1, 0, 2, 4, 9 }},
        {7, {-1, 0, 2, 4, 7, 9}},
        {0, {-1, 0, 2, 4, 7, 9}},
        {4, {-1, 0, 2, 4, 7, 9}}
    };

    SortedSet<int> sortedSet;
    for(auto[toAdd, expected] : testCases)
    {
        sortedSet.insert(toAdd);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}

TEST_CASE("Test.SortedSet.erase", "[SortedSet]")
{
    const std::pair<int, std::vector<int>> testCases[] = {
        {4, {-1, 0, 2, 7, 9}},
        {0, {-1, 2, 7, 9}},
        {7, {-1, 2, 9}},
        {2, {-1, 9}},
        {-1, {9}},
        {4, {9}},
        {9, {}},
        {0, {}},
    };

    const auto& initialList = testCases[0].second;
    SortedSet<int> sortedSet(initialList.begin(), initialList.end());
    for(auto[toRemove, expected] : testCases)
    {
        sortedSet.erase(toRemove);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}


TEST_CASE("Test.SortedMap.construct", "[SortedMap]")
{
    SortedMap<std::string, int> words = {
        {"pen",      1},
        {"apple", 3},
        {"penapple", 0},
        {"applepen", 2}
    };

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
        {"apple", 3},
        {"applepen", 2},
        {"pen",      1},
        {"penapple", 0}
    });
}

TEST_CASE("Test.SortedMap.operator[]", "[SortedMap]")
{
    SortedMap<std::string, int> words = {
        {"pen",      1},
        {"apple", 3},
        {"penapple", 0},
        {"applepen", 2}
    };

    // inserting elements
    words["pen"] = 2;
    words["apple"] = 0;
    words["penapple"] = 3;
    words["applepen"] = 1;

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
        {"apple", 0},
        {"applepen", 1},
        {"pen",      2},
        {"penapple", 3}
    });

    // changing existing elements
    words["pen"] = 1;
    words["apple"] = 3;
    words["penapple"] = 0;
    words["applepen"] = 2;

    REQUIRE(words.elems() == std::vector<std::pair<std::string, int>>{
        {"apple", 3},
        {"applepen", 2},
        {"pen",      1},
        {"penapple", 0}
    });
}


TEST_CASE("Check if two ordered sets intersects", "[algorithms]")
{
    SECTION("Two empty sets do not intersects")
    {
        REQUIRE_FALSE(set_intersects(std::vector<int>{}, std::vector<int>{}));
    }
    SECTION("If one of sets is empty - sets are not intersects")
    {
        std::vector<int> firstSet = {1, 2, 3};
        std::vector<int> secondSet;
        REQUIRE_FALSE(set_intersects(firstSet, secondSet));
        REQUIRE_FALSE(set_intersects(secondSet, firstSet));
    }
    SECTION("set_intersects function works correctly")
    {
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{1, 2, 3}));
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{2, 3}));
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{1, 2}));
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{1}));
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{2}));
        REQUIRE(set_intersects(std::vector{1, 2, 3}, std::vector{3}));
        REQUIRE(set_intersects(std::vector{2, 4, 5, 6, 7, 9}, std::vector{8, 9}));

        REQUIRE_FALSE(set_intersects(std::vector{1, 3, 5}, std::vector{2, 4, 6}));
        REQUIRE_FALSE(set_intersects(std::vector{1, 3, 5}, std::vector{0}));
        REQUIRE_FALSE(set_intersects(std::vector{1, 3, 5}, std::vector{9, 10}));
    }
}

template<typename InputIter1, typename InputIter2>
static bool simple_set_intersects(InputIter1 first1, InputIter1 last1,
                                  InputIter2 first2, InputIter2 last2)
{
    for(auto i = first1; i != last1; ++i)
    {
        for(auto j = first2; j != last2; ++j)
        {
            if(*i == *j)
                return true;
        }
    }

    return false;
}

template<class Range1, class Range2>
bool simple_set_intersects(const Range1& r1, const Range2& r2)
{
    return simple_set_intersects(std::begin(r1), std::end(r1),
                                 std::begin(r2), std::end(r2));
}

TEST_CASE("Benchmark: Determining intersection of two sets", "[benchmark]")
{
    const std::vector<int> lhs = {0, 2, 4, 6, 7, 8, 9, 10, 12, 14, 16, 18, 19, 20, 22};
    const std::vector<int> rhs = {1, 3, 5, 7, 11, 13, 15, 17, 19, 21, 23};

    BENCHMARK("Using simple brute force")
    {
        return simple_set_intersects(lhs, rhs);
    };

    BENCHMARK("Using sorted data advantages with std::set_intersection")
    {
        std::vector<int> intersected;
        std::set_intersection(lhs.begin(), lhs.end(),
                              rhs.begin(), rhs.end(),
                              std::back_inserter(intersected));

        return !intersected.empty();
    };

    BENCHMARK("Using sorted data advantages with set_intersects")
    {
        return set_intersects(lhs, rhs);
    };
}
