#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>


constexpr std::size_t MIN_COMPLEXITY = 1;
constexpr std::size_t MAX_COMPLEXITY = 4;

constexpr std::size_t SCHEDULE_DAYS_COUNT = 12;
constexpr std::size_t MAX_LESSONS_PER_DAY = 7;
constexpr std::size_t DAYS_IN_SCHEDULE_WEEK = 6;
constexpr std::size_t DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr std::size_t MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE;

constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();
constexpr std::size_t NO_BUILDING = std::numeric_limits<std::size_t>::max();
constexpr std::size_t NOT_EVALUATED = std::numeric_limits<std::size_t>::max();


struct ClassroomAddress
{
    static ClassroomAddress NoClassroom()
    {
        return ClassroomAddress{.Building = std::numeric_limits<std::size_t>::max(),
                                .Classroom = std::numeric_limits<std::size_t>::max()};
    }

    static ClassroomAddress Any() { return ClassroomAddress{.Building = 0, .Classroom = 0}; }

    friend bool operator==(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return lhs.Building == rhs.Building && lhs.Classroom == rhs.Classroom;
    }
    friend bool operator!=(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return (lhs.Building < rhs.Building)
               || (lhs.Building == rhs.Building && lhs.Classroom < rhs.Classroom);
    }
    friend bool operator>(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return rhs < lhs;
    }
    friend bool operator<=(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return !(rhs < lhs);
    }
    friend bool operator>=(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return !(lhs < rhs);
    }

    std::size_t Building = 0;
    std::size_t Classroom = 0;
};


const std::vector<std::size_t>& AllLessons();
std::vector<std::size_t> LessonsSortedByOrderInDay(std::vector<std::size_t> lessons);
bool LessonsAreInSameDay(std::size_t firstLesson, std::size_t secondLesson);
