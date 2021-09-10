#include "ScheduleCommon.h"
#include <catch2/catch.hpp>

#include <array>


TEST_CASE("Day to week day conversion works", "[common][conversions]")
{
    constexpr std::array<WeekDay, DAYS_IN_SCHEDULE> expected {
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

    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
        REQUIRE(DayToWeekDay(d) == expected.at(d));
}

TEST_CASE("Lesson to week day conversion works", "[common][conversions]")
{
    constexpr std::array<WeekDay, MAX_LESSONS_COUNT> expected {
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,

        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,

        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,

        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,

        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,

        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,

        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,
        WeekDay::Monday,

        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,
        WeekDay::Tuesday,

        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,
        WeekDay::Wednesday,

        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,
        WeekDay::Thursday,

        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,
        WeekDay::Friday,

        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday,
        WeekDay::Saturday
    };

    for(std::size_t lesson = 0; lesson < MAX_LESSONS_COUNT; ++lesson)
        REQUIRE(LessonToWeekDay(lesson) == expected.at(lesson));
}

TEST_CASE("Getting first lesson number in weekday (first week)", "[common][conversions]")
{
    constexpr std::array<std::size_t, DAYS_IN_SCHEDULE_WEEK> expected {
        0,
        7,
        14,
        21,
        28,
        35
    };

    for(std::size_t wd = 0; wd < DAYS_IN_SCHEDULE_WEEK; ++wd)
        REQUIRE(FirstWeekFirstLessonIn(static_cast<WeekDay>(wd)) == expected.at(wd));
}

TEST_CASE("Getting first lesson number in weekday (second week)", "[common][conversions]")
{
    constexpr std::array<std::size_t, DAYS_IN_SCHEDULE_WEEK> expected {
        42,
        49,
        56,
        63,
        70,
        77
    };

    for(std::size_t wd = 0; wd < DAYS_IN_SCHEDULE_WEEK; ++wd)
        REQUIRE(SecondWeekFirstLessonIn(static_cast<WeekDay>(wd)) == expected.at(wd));
}

TEST_CASE("Check if WeekDays contains elements", "[common][weekdays]")
{
    WeekDays days;
    REQUIRE(days.contains(WeekDay::Monday));
    REQUIRE(days.contains(WeekDay::Tuesday));
    REQUIRE(days.contains(WeekDay::Wednesday));
    REQUIRE(days.contains(WeekDay::Thursday));
    REQUIRE(days.contains(WeekDay::Friday));
    REQUIRE(days.contains(WeekDay::Saturday));
}

TEST_CASE("Erase elements from WeekDays", "[common][weekdays]")
{
    auto checkRemove = [](const WeekDays& weekDays, WeekDay removed){
        for(int i = 0; i < weekDays.size(); ++i)
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

    REQUIRE(days == WeekDays::emptyWeek());
}

TEST_CASE("Inserting new elements to WeekDays", "[common][weekdays]")
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

TEST_CASE("Iterating through WeekDays elements", "[common][weekdays]")
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
