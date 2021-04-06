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
