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
constexpr std::size_t MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE_WEEK * 2;


enum class WeekDay : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

struct ClassroomAddress
{
    ClassroomAddress() = default;
    ClassroomAddress(std::size_t building, std::size_t classroom)
        : Building(building)
        , Classroom(classroom)
    {}

    static ClassroomAddress NoClassroom() { return ClassroomAddress(std::numeric_limits<std::size_t>::max(),
                                                                    std::numeric_limits<std::size_t>::max()); }

    static ClassroomAddress Any() { return ClassroomAddress(0, 0); }

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

    std::size_t Building;
    std::size_t Classroom;
};


class WeekDays;
class WeekDaysIterator
{
    friend class WeekDays;
    static constexpr std::uint8_t BEGIN_MASK = 0b00000001;
    static constexpr std::uint8_t END_MASK = 0b01000000;
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = bool;
    using reference = bool&;
    using pointer = bool*;

    constexpr WeekDaysIterator() = default;

    constexpr bool operator*() const { assert(mask_ <= 0b00111111); return data_ & mask_; }

    constexpr WeekDaysIterator& operator++() { mask_ <<= 1; return *this; }
    constexpr WeekDaysIterator operator++(int) { auto result = *this; ++(*this); return result; }

    constexpr bool operator==(WeekDaysIterator other) const
    {
        assert(data_ == other.data_);
        return mask_ == other.mask_;
    }
    constexpr bool operator!=(WeekDaysIterator other) const { return (*this == other); }

private:
    explicit WeekDaysIterator(std::uint8_t data, std::uint8_t mask) : mask_(mask), data_(data) {}

private:
    std::uint8_t mask_ = 0;
    std::uint8_t data_ = 0;
};

/**
 * Набор дней недели.
 */
class WeekDays
{
    static constexpr std::uint8_t FULL_WEEK = 0b00111111;
public:
    using iterator = WeekDaysIterator;

    WeekDays();
    WeekDays(std::initializer_list<WeekDay> lst);
    static WeekDays fullWeek() { return WeekDays(FULL_WEEK); }
    static WeekDays emptyWeek() { return WeekDays(0); }

    auto begin() const { return WeekDaysIterator(days_, WeekDaysIterator::BEGIN_MASK); }
    auto end() const { return WeekDaysIterator(days_, WeekDaysIterator::END_MASK); }
    std::size_t size() const { return 6; }

    void insert(WeekDay d);
    void erase(WeekDay d);
    bool contains(WeekDay d) const;

    friend bool operator==(WeekDays lhs, WeekDays rhs) { return lhs.days_ == rhs.days_; }
    friend bool operator!=(WeekDays lhs, WeekDays rhs) { return !(lhs == rhs); }

private:
    explicit WeekDays(std::uint8_t days);

private:
    std::uint8_t days_;
};

std::size_t LessonToScheduleDay(std::size_t lesson);


constexpr bool IsLateScheduleLessonInSaturday(std::size_t l)
{
    constexpr std::array lateSaturdayLessonsTable = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,
    };

    static_assert(lateSaturdayLessonsTable.size() == MAX_LESSONS_COUNT, "re-fill lateSaturdayLessonsTable");
    return lateSaturdayLessonsTable[l];
}
