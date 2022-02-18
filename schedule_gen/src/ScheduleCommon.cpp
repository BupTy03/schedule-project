#include "ScheduleCommon.h"

#include <numeric>


const std::vector<std::size_t>& AllLessons()
{
    static const auto allLessons = []
    {
        std::vector<std::size_t> result(MAX_LESSONS_COUNT);
        std::iota(result.begin(), result.end(), 0);
        return result;
    }();

    return allLessons;
}

std::vector<std::size_t> LessonsSortedByOrderInDay(std::vector<std::size_t> lessons)
{
    std::ranges::sort(lessons, [](std::size_t lhs, std::size_t rhs)
      { return lhs % MAX_LESSONS_PER_DAY < rhs % MAX_LESSONS_PER_DAY; });

    return lessons;
}

bool LessonsAreInSameDay(std::size_t firstLesson, std::size_t secondLesson)
{
    return firstLesson / MAX_LESSONS_PER_DAY == secondLesson / MAX_LESSONS_PER_DAY;
}
