#include <catch2/catch.hpp>

#include "ScheduleData.h"

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


