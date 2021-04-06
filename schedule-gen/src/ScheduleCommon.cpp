#include "ScheduleCommon.hpp"

#include <algorithm>


const std::size_t SCHEDULE_DAYS_COUNT = 12;


WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum)
{
    return static_cast<WeekDay>(dayNum % 6);
}

typename std::array<bool, 6>::const_iterator WeekDays::begin() const
{
    if(Empty())
        days_.fill(true);

    return days_.begin();
}

typename std::array<bool, 6>::const_iterator WeekDays::end() const
{
    if(Empty())
        days_.fill(true);

    return days_.end();
}

std::size_t WeekDays::size() const { return days_.size(); }

void WeekDays::Add(WeekDay d) { days_.at(static_cast<std::size_t>(d)) = true; }

void WeekDays::Remove(WeekDay d) { days_.at(static_cast<std::size_t>(d)) = false; }

bool WeekDays::Contains(WeekDay d) const
{
    return days_.at(static_cast<std::size_t>(d)) || Empty();
}

bool WeekDays::Empty() const
{
    return std::none_of(days_.begin(), days_.end(), [](auto f){return f;});
}
