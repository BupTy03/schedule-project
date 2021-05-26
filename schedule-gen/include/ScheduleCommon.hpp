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

std::size_t LessonToScheduleDay(std::size_t lesson);
WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
std::vector<std::size_t> MakeIndexesRange(std::size_t n);
constexpr bool IsLateScheduleLessonInSaturday(std::size_t l)
{
    static_assert(MAX_LESSONS_COUNT == 84, "re-fill lateSaturdayLessonsTable");
    assert(l < MAX_LESSONS_COUNT);

    constexpr bool lateSaturdayLessonsTable[MAX_LESSONS_COUNT] = {
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

    return lateSaturdayLessonsTable[l];
}

struct ClassroomAddress
{
    ClassroomAddress() = default;
    explicit ClassroomAddress(std::size_t building, std::size_t classroom)
        : Building(building)
        , Classroom(classroom)
    {}

    static ClassroomAddress NoClassroom();
    static ClassroomAddress Any();

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

std::vector<ClassroomAddress> GenerateClassrooms(std::size_t n);


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

    [[nodiscard]] constexpr bool operator*() const { assert(mask_ <= 0b00111111); return data_ & mask_; }

    constexpr WeekDaysIterator& operator++() { mask_ <<= 1; return *this; }
    constexpr WeekDaysIterator operator++(int) { auto result = *this; ++(*this); return result; }

    [[nodiscard]] constexpr bool operator==(WeekDaysIterator other) const
    {
        assert(data_ == other.data_);
        return mask_ == other.mask_;
    }
    [[nodiscard]] constexpr bool operator!=(WeekDaysIterator other) const { return (*this == other); }

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
    static WeekDays fullWeek();
    static WeekDays emptyWeek();

    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;

    void insert(WeekDay d);
    void erase(WeekDay d);
    [[nodiscard]] bool contains(WeekDay d) const;

    friend bool operator==(WeekDays lhs, WeekDays rhs) { return lhs.days_ == rhs.days_; }
    friend bool operator!=(WeekDays lhs, WeekDays rhs) { return !(lhs == rhs); }

private:
    explicit WeekDays(std::uint8_t days);

private:
    std::uint8_t days_;
};

struct LessonAddress
{
    LessonAddress() = default;
    explicit LessonAddress(std::size_t Group, std::size_t Lesson)
            : Group(Group)
            , Lesson(Lesson)
    { }

    [[nodiscard]] friend bool operator<(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return (lhs.Group < rhs.Group) || (lhs.Group == rhs.Group && lhs.Lesson < rhs.Lesson);
    }

    [[nodiscard]] friend bool operator==(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group == rhs.Group && lhs.Lesson == rhs.Lesson;
    }

    [[nodiscard]] friend bool operator!=(const LessonAddress& lhs, const LessonAddress& rhs) { return !(lhs == rhs); }

    std::size_t Group = 0;
    std::size_t Lesson = 0;
};

struct LessonsMatrixItemAddress
{
    explicit LessonsMatrixItemAddress(std::size_t group,
                                      std::size_t professor,
                                      std::size_t lesson,
                                      std::size_t classroom,
                                      std::size_t subject);

    [[nodiscard]] friend bool operator==(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Group == rhs.Group &&
               lhs.Professor == rhs.Professor &&
               lhs.Lesson == rhs.Lesson &&
               lhs.Classroom == rhs.Classroom &&
               lhs.Subject == rhs.Subject;
    }

    [[nodiscard]] friend bool operator!=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool operator<(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return
               (lhs.Group < rhs.Group) ||
               (lhs.Group == rhs.Group && lhs.Professor < rhs.Professor) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson < rhs.Lesson) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom < rhs.Classroom) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom == rhs.Classroom && lhs.Subject < rhs.Subject);
    }

    [[nodiscard]] friend bool operator>(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend bool operator<=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs > rhs);
    }
    [[nodiscard]] friend bool operator>=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs < rhs);
    }

    std::size_t Group;
    std::size_t Professor;
    std::size_t Lesson;
    std::size_t Classroom;
    std::size_t Subject;
};
