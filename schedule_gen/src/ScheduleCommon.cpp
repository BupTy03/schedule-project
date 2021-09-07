#include "ScheduleCommon.h"

#undef min
#undef max


std::size_t LessonToScheduleDay(std::size_t lesson) { return lesson / MAX_LESSONS_PER_DAY; }
WeekDay ToWeekDay(std::size_t d) { return static_cast<WeekDay>(d % DAYS_IN_SCHEDULE_WEEK); }

WeekDays::WeekDays() : days_(FULL_WEEK) { }

WeekDays::WeekDays(std::initializer_list<WeekDay> lst) : days_(0)
{
    for(auto wd : lst)
        insert(wd);
}

WeekDays::WeekDays(std::uint8_t days) : days_(days) { }

void WeekDays::insert(WeekDay d) { days_ |= (1 << static_cast<std::uint8_t>(d)); }
void WeekDays::erase(WeekDay d) { days_ &= ~(1 << static_cast<std::uint8_t>(d)); }
bool WeekDays::contains(WeekDay d) const { return (days_ & (1 << static_cast<std::uint8_t>(d))); }
