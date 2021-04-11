#include <catch2/catch.hpp>
#include "ScheduleCommon.hpp"
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"
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


TEST_CASE("Test.WeekDays.Contains", "[WeekDays]")
{
    WeekDays days;
    REQUIRE(days.Contains(WeekDay::Monday));
    REQUIRE(days.Contains(WeekDay::Tuesday));
    REQUIRE(days.Contains(WeekDay::Wednesday));
    REQUIRE(days.Contains(WeekDay::Thursday));
    REQUIRE(days.Contains(WeekDay::Friday));
    REQUIRE(days.Contains(WeekDay::Saturday));
}

TEST_CASE("Test.WeekDays.Remove", "[WeekDays]")
{
    auto checkRemove = [](const WeekDays& weekDays, WeekDay removed){
        for(int i = 0; i < 6; ++i)
        {
            const auto wd = static_cast<WeekDay>(i);
            if(removed == wd)
            {
                REQUIRE_FALSE(weekDays.Contains(removed));
            }
            else
            {
                REQUIRE(weekDays.Contains(wd));
            }
        }
    };

    for(int i = 0; i < 6; ++i)
    {
        const auto wd = static_cast<WeekDay>(i);
        WeekDays days;
        days.Remove(wd);
        checkRemove(days, wd);
    }

    // удалить все дни недели невозможно
    // удаление всех дней недели автоматически приводит к заполнению всех дней недели
    WeekDays days;
    for(int i = 0; i < 6; ++i)
        days.Remove(static_cast<WeekDay>(i));

    for(int i = 0; i < 6; ++i)
        days.Contains(static_cast<WeekDay>(i));
}

TEST_CASE("Test.WeekDays.Add", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        const auto added = static_cast<WeekDay>(i);
        WeekDays weekDays;
        for(int j = 0; j < 6; ++j)
        {
            const auto curr = static_cast<WeekDay>(j);
            if(curr == added)
                weekDays.Add(curr);
            else
                weekDays.Remove(curr);
        }

        for(int j = 0; j < 6; ++j)
        {
            const auto wd = static_cast<WeekDay>(j);
            if(added == wd)
                REQUIRE(weekDays.Contains(added));
            else
                REQUIRE_FALSE(weekDays.Contains(wd));
        }
    }
}

TEST_CASE("Test.WeekDays.WeekDaysIterator", "[WeekDays]")
{
    for(int i = 0; i < 6; ++i)
    {
        const auto removed = static_cast<WeekDay>(i);

        WeekDays weekDays;
        weekDays.Remove(removed);

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

TEST_CASE("Test.FindOverlappedClassrooms", "[Validate]")
{
    ScheduleData data(3, 2, 2, 3, std::vector<SubjectRequest>({
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({0})),
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({0})),
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({0})),
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({1})),
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({1})),
        SubjectRequest(0, 5, 4, WeekDays(), SortedSet<std::size_t>({0, 1, 2}), SortedSet<std::size_t>({2}))
    }));

    // ScheduleItem(std::size_t subject, std::size_t professor, std::size_t classroom)
    ScheduleResult result(std::vector<ScheduleResult::Group>({
        ScheduleResult::Group({
            ScheduleResult::Day({
                ScheduleResult::Lesson(ScheduleItem(0, 0, 0)),
                ScheduleResult::Lesson(ScheduleItem(2, 0, 1)),
                ScheduleResult::Lesson(ScheduleItem(1, 0, 1)),
                ScheduleResult::Lesson(),
                ScheduleResult::Lesson(),
                ScheduleResult::Lesson()
            }),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6)
        }),
        ScheduleResult::Group({
            ScheduleResult::Day({
                ScheduleResult::Lesson(ScheduleItem(0, 0, 0)),
                ScheduleResult::Lesson(ScheduleItem(2, 0, 1)),
                ScheduleResult::Lesson(ScheduleItem(1, 0, 2)),
                ScheduleResult::Lesson(),
                ScheduleResult::Lesson(),
                ScheduleResult::Lesson()
            }),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6),
            ScheduleResult::Day(6)
        })
    }));

    const auto overlappedClassrooms = FindOverlappedClassrooms(data, result);
    REQUIRE(overlappedClassrooms.at(0).Classroom == 0);
    REQUIRE(overlappedClassrooms.at(0).Lessons.Contains(LessonAddress(0, 0, 0)));
    REQUIRE(overlappedClassrooms.at(0).Lessons.Contains(LessonAddress(1, 0, 0)));

    REQUIRE(overlappedClassrooms.at(1).Classroom == 1);
    REQUIRE(overlappedClassrooms.at(1).Lessons.Contains(LessonAddress(0, 0, 1)));
    REQUIRE(overlappedClassrooms.at(1).Lessons.Contains(LessonAddress(1, 0, 1)));
}
