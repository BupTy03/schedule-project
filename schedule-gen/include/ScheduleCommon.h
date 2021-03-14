#pragma once
#include <cstdint>
#include <cstddef>


extern const std::size_t SCHEDULE_DAYS_COUNT;

enum class WeekDay : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
