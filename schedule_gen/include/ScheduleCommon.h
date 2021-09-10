#pragma once
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>


constexpr std::size_t SCHEDULE_DAYS_COUNT = 12;
constexpr std::size_t MAX_LESSONS_PER_DAY = 7;
constexpr std::size_t DAYS_IN_SCHEDULE_WEEK = 6;
constexpr std::size_t DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr std::size_t MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE;

constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();
constexpr std::size_t NO_BUILDING = std::numeric_limits<std::size_t>::max();
constexpr std::size_t NOT_EVALUATED = std::numeric_limits<std::size_t>::max();


const std::vector<std::size_t>& AllLessons();


struct ClassroomAddress
{
    static ClassroomAddress NoClassroom() { return ClassroomAddress{.Building = std::numeric_limits<std::size_t>::max(),
                                                                    .Classroom = std::numeric_limits<std::size_t>::max()}; }

    static ClassroomAddress Any() { return ClassroomAddress{.Building = 0, .Classroom = 0}; }

    friend bool operator==(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return lhs.Building == rhs.Building && lhs.Classroom == rhs.Classroom;
    }
    friend bool operator!=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs == rhs); }

    friend bool operator<(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return (lhs.Building < rhs.Building) || (lhs.Building == rhs.Building && lhs.Classroom < rhs.Classroom);
    }
    friend bool operator>(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return rhs < lhs; }
    friend bool operator<=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(rhs < lhs); }
    friend bool operator>=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs < rhs); }

    std::size_t Building = 0;
    std::size_t Classroom = 0;
};
