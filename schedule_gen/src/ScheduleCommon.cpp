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
