#include "ScheduleCommon.hpp"
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <catch2/catch.hpp>

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


TEST_CASE("Test.WeekDays.contains", "[WeekDays]")
{
    WeekDays days;
    REQUIRE(days.contains(WeekDay::Monday));
    REQUIRE(days.contains(WeekDay::Tuesday));
    REQUIRE(days.contains(WeekDay::Wednesday));
    REQUIRE(days.contains(WeekDay::Thursday));
    REQUIRE(days.contains(WeekDay::Friday));
    REQUIRE(days.contains(WeekDay::Saturday));
}

TEST_CASE("Test.WeekDays.erase", "[WeekDays]")
{
    auto checkRemove = [](const WeekDays& weekDays, WeekDay removed){
        for(int i = 0; i < 6; ++i)
        {
            const auto wd = static_cast<WeekDay>(i);
            if(removed == wd)
            {
                REQUIRE_FALSE(weekDays.contains(wd));
            }
            else
            {
                REQUIRE(weekDays.contains(wd));
            }
        }
    };

    for(int i = 0; i < 6; ++i)
    {
        const auto wd = static_cast<WeekDay>(i);
        WeekDays days;
        days.erase(wd);
        checkRemove(days, wd);
    }

    WeekDays days;
    for(int i = 0; i < 6; ++i)
        days.erase(static_cast<WeekDay>(i));

    REQUIRE(days.empty());
}

TEST_CASE("Test.WeekDays.insert", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        WeekDays weekDays = WeekDays::emptyWeek();
        weekDays.insert(static_cast<WeekDay>(i));
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE(weekDays.contains(static_cast<WeekDay>(j)));
            else
                REQUIRE_FALSE(weekDays.contains(static_cast<WeekDay>(j)));
        }
    }
}

TEST_CASE("Test.WeekDays.WeekDaysIterator", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        const auto removed = static_cast<WeekDay>(i);

        WeekDays weekDays;
        weekDays.erase(removed);

        auto it = weekDays.begin();
        for(int j = 0; j < 6; ++j)
        {
            if(i == j)
                REQUIRE_FALSE(*it);
            else
                REQUIRE(*it);

            ++it;
        }
        REQUIRE(it == weekDays.end());
    }
}

TEST_CASE("Test.SortedSet.construct", "[SortedSet]")
{
    std::array<int, 20> arr = {4, 6, 2, 9, 1, 3, 5, 7, 8, 0,
                               4, 2, 6, 1, 3, 0, 7, 5, 2, 6};
    std::array<int, 10> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    SortedSet<int> sortedSet(arr.begin(), arr.end());
    REQUIRE(sortedSet.size() == expected.size());
    REQUIRE(std::equal(sortedSet.begin(), sortedSet.end(), expected.begin()));
}

TEST_CASE("Test.SortedSet.contains", "[SortedSet]")
{
    SortedSet<int> emptySet;
    for(auto e : {1, 2, 3, 6})
        REQUIRE_FALSE(emptySet.contains(e));

    std::array<int, 10> arr = {4, 6, 2, 9, 0, 6, 1, 0, 5, 1};
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

TEST_CASE("Test.FindOverlappedClassrooms", "[Validate]")
{
    const std::vector<SubjectRequest> subjectRequests = {
        SubjectRequest(0, 1, 0, {}, {0}, {0, 1, 2}),
        SubjectRequest(1, 1, 1, {}, {3}, {0, 1, 2}),
        SubjectRequest(2, 1, 2, {}, {1}, {0, 1, 2}),
        SubjectRequest(3, 1, 3, {}, {2}, {0, 1, 2}),
        SubjectRequest(4, 1, 4, {}, {1}, {0, 1, 2})
    };

    const ScheduleData scheduleData(6, 6, 5, 3, subjectRequests, {});

    ScheduleResult scheduleResult;
    scheduleResult.insert(ScheduleItem(LessonAddress(0, 0, 0), 0, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(3, 0, 0), 1, 1, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(4, 0, 0), 2, 2, 1));
    scheduleResult.insert(ScheduleItem(LessonAddress(2, 0, 0), 3, 3, 2));
    scheduleResult.insert(ScheduleItem(LessonAddress(1, 0, 0), 4, 4, 1));

    const auto result = FindOverlappedClassrooms(scheduleData, scheduleResult);
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Classroom == 0; }) != result.end());
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Classroom == 1; }) != result.end());
}

TEST_CASE("Test.FindOverlappedProfessors", "[Validate]")
{
    const std::vector<SubjectRequest> subjectRequests = {
            SubjectRequest(0, 1, 0, {}, {0}, {0, 1, 2}),
            SubjectRequest(1, 1, 1, {}, {3}, {0, 1, 2}),
            SubjectRequest(2, 1, 2, {}, {1}, {0, 1, 2}),
            SubjectRequest(3, 1, 3, {}, {2}, {0, 1, 2}),
            SubjectRequest(4, 1, 4, {}, {1}, {0, 1, 2})
    };

    const ScheduleData scheduleData(6, 6, 5, 3, subjectRequests, {});

    ScheduleResult scheduleResult;
    scheduleResult.insert(ScheduleItem(LessonAddress(0, 0, 0), 0, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(3, 0, 0), 1, 0, 0));
    scheduleResult.insert(ScheduleItem(LessonAddress(4, 0, 0), 2, 1, 1));
    scheduleResult.insert(ScheduleItem(LessonAddress(2, 0, 0), 3, 3, 2));
    scheduleResult.insert(ScheduleItem(LessonAddress(1, 0, 0), 4, 1, 1));

    const auto result = FindOverlappedProfessors(scheduleData, scheduleResult);
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Professor == 0; }) != result.end());
    REQUIRE(std::find_if(result.begin(), result.end(), [](auto&& oc){ return oc.Professor == 1; }) != result.end());
}

TEST_CASE("Test.FindViolatedSubjectRequests", "[Validate]")
{
}
