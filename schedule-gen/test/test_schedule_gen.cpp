#include <catch2/catch.hpp>
#include "ScheduleCommon.hpp"
#include <array>


TEST_CASE("TestScheduleDayNumberToWeekDay", "[utils]")
{
    const std::array<WeekDay, 12> scheduleDays = {
            WeekDay::Monday,
            WeekDay::Tuesday,
            WeekDay::Wednesday,
            WeekDay::Thursday,
            WeekDay::Friday,
            WeekDay::Saturday,
            WeekDay::Monday,
            WeekDay::Tuesday,
            WeekDay::Wednesday,
            WeekDay::Thursday,
            WeekDay::Friday,
            WeekDay::Saturday
    };

    for(std::size_t d = 0; d < scheduleDays.size(); ++d)
        REQUIRE(ScheduleDayNumberToWeekDay(d) == scheduleDays.at(d));
}


TEST_CASE("TestWeekDays", "[WeekDays]")
{
    WeekDays days;
    REQUIRE(days.Contains(WeekDay::Monday));
    REQUIRE(days.Contains(WeekDay::Tuesday));
    REQUIRE(days.Contains(WeekDay::Wednesday));
    REQUIRE(days.Contains(WeekDay::Thursday));
    REQUIRE(days.Contains(WeekDay::Friday));
    REQUIRE(days.Contains(WeekDay::Saturday));

    for(auto d : days)
        REQUIRE(d);

    days.Remove(WeekDay::Monday);
    REQUIRE_FALSE(days.Contains(WeekDay::Monday));

    days.Remove(WeekDay::Tuesday);
    REQUIRE_FALSE(days.Contains(WeekDay::Tuesday));

    days.Remove(WeekDay::Wednesday);
    REQUIRE_FALSE(days.Contains(WeekDay::Wednesday));

    days.Remove(WeekDay::Thursday);
    REQUIRE_FALSE(days.Contains(WeekDay::Thursday));

    days.Remove(WeekDay::Friday);
    REQUIRE_FALSE(days.Contains(WeekDay::Friday));

    // удалить все дни в неделе = заполнить все дни
    days.Remove(WeekDay::Saturday);
    for(auto d : days)
        REQUIRE(d);
}

TEST_CASE("Test.SortedSet.Construct", "[SortedSet]")
{
    std::array<int, 20> arr = {4, 6, 2, 9, 1, 3, 5, 7, 8, 0,
                               4, 2, 6, 1, 3, 0, 7, 5, 2, 6};
    std::array<int, 10> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SortedSet<int> sortedSet(arr.begin(), arr.end());
    REQUIRE(sortedSet.size() == expected.size());
    REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
}

TEST_CASE("Test.SortedSet.Contains", "[SortedSet]")
{
    SortedSet<int> emptySet;
    for(auto e : {1, 2, 3, 6})
        REQUIRE_FALSE(emptySet.Contains(e));

    std::array<int, 10> arr = {4, 6, 2, 9, 0, 6, 1, 0, 5, 1};
    SortedSet<int> sortedSet(arr.begin(), arr.end());
    for(std::size_t i = 0; i < 100; ++i)
    {
        if(std::find(arr.begin(), arr.end(), i) == arr.end())
            REQUIRE_FALSE(sortedSet.Contains(i));
        else
            REQUIRE(sortedSet.Contains(i));
    }
}

TEST_CASE("Test.SortedSet.Add", "[SortedSet]")
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
        sortedSet.Add(toAdd);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}

TEST_CASE("Test.SortedSet.Remove", "[SortedSet]")
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
        sortedSet.Remove(toRemove);
        REQUIRE(sortedSet.size() == expected.size());
        REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
    }
}
