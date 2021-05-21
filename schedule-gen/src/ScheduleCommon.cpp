#include "ScheduleCommon.hpp"

#include <numeric>

#undef min
#undef max


std::size_t LessonToScheduleDay(std::size_t lesson)
{
    return lesson / MAX_LESSONS_PER_DAY;
}

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum)
{
    return static_cast<WeekDay>(dayNum % DAYS_IN_SCHEDULE_WEEK);
}

std::vector<std::size_t> MakeIndexesRange(std::size_t n)
{
    std::vector<std::size_t> result(n);
    std::iota(result.begin(), result.end(), std::size_t{0});
    return result;
}

std::vector<ClassroomAddress> GenerateClassrooms(std::size_t n)
{
    std::vector<ClassroomAddress> result;
    result.reserve(n);
    for(std::size_t classroom = 0; classroom < n; ++classroom)
        result.emplace_back(0, classroom);

    return result;
}


ClassroomAddress ClassroomAddress::NoClassroom()
{
    return ClassroomAddress(std::numeric_limits<std::size_t>::max(),
                            std::numeric_limits<std::size_t>::max());
}


WeekDays::WeekDays()
    : days_(FULL_WEEK)
{
}

WeekDays::WeekDays(std::initializer_list<WeekDay> lst)
    : days_(0)
{
    for(auto wd : lst)
        insert(wd);
}

WeekDays::WeekDays(std::uint8_t days)
    : days_(days)
{
}

WeekDays WeekDays::fullWeek() { return WeekDays(FULL_WEEK); }
WeekDays WeekDays::emptyWeek() { return WeekDays(0); }

WeekDays::iterator WeekDays::begin() const { return WeekDaysIterator(days_, WeekDaysIterator::BEGIN_MASK); }
WeekDays::iterator WeekDays::end() const { return WeekDaysIterator(days_, WeekDaysIterator::END_MASK); }

std::size_t WeekDays::size() const { return 6; }
bool WeekDays::empty() const { return days_ == 0; }

void WeekDays::insert(WeekDay d) { days_ |= (1 << static_cast<std::uint8_t>(d)); }
void WeekDays::erase(WeekDay d) { days_ &= ~(1 << static_cast<std::uint8_t>(d)); }
bool WeekDays::contains(WeekDay d) const { return (days_ & (1 << static_cast<std::uint8_t>(d))); }


std::size_t CalculatePadding(std::size_t baseAddress, std::size_t alignment)
{
    if(baseAddress == 0 || alignment == 0)
        return 0;

    if(alignment > baseAddress)
        return alignment - baseAddress;

    return baseAddress % alignment;
}

LessonsMatrixItemAddress::LessonsMatrixItemAddress(std::size_t group,
                                                   std::size_t professor,
                                                   std::size_t lesson,
                                                   std::size_t classroom,
                                                   std::size_t subject)
        : Group(group)
        , Professor(professor)
        , Lesson(lesson)
        , Classroom(classroom)
        , Subject(subject)
{
}
