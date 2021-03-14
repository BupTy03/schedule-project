#include "ScheduleData.h"


const std::size_t SCHEDULE_DAYS_COUNT = 12;


WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum)
{
    return static_cast<WeekDay>(dayNum % 6);
}
