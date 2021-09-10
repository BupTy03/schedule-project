#include "ScheduleCommon.h"

#include <numeric>


std::vector<std::size_t> AllLessons()
{
    std::vector<std::size_t> result(MAX_LESSONS_COUNT);
    std::iota(result.begin(), result.end(), 0);
    return result;
}

std::size_t LessonToScheduleDay(std::size_t lesson) { return lesson / MAX_LESSONS_PER_DAY; }
WeekDay DayToWeekDay(std::size_t d) { return static_cast<WeekDay>(d % DAYS_IN_SCHEDULE_WEEK); }
WeekDay LessonToWeekDay(std::size_t lesson) { return DayToWeekDay(LessonToScheduleDay(lesson)); }
std::size_t FirstWeekFirstLessonIn(WeekDay d) { return static_cast<std::uint8_t>(d) * MAX_LESSONS_PER_DAY; }
std::size_t SecondWeekFirstLessonIn(WeekDay d) { return (static_cast<std::uint8_t>(d) + DAYS_IN_SCHEDULE_WEEK) * MAX_LESSONS_PER_DAY; }

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
