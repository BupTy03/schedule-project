#pragma once

#include "ScheduleCommon.hpp"
#include <cstdint>
#include <vector>
#include <optional>
#include <stdexcept>


class ScheduleData;

struct ScheduleItem
{
    explicit ScheduleItem(std::size_t subject, std::size_t professor, std::size_t classroom);

    std::size_t Subject;
    std::size_t Professor;
    std::size_t Classroom;
};

class ScheduleResult
{
public:
    using Lesson = std::optional<ScheduleItem>;
    using Day = std::vector<Lesson>;
    using Group = std::vector<Day>;

    explicit ScheduleResult(std::vector<Group> groups);
    [[nodiscard]] bool Empty() const;
    [[nodiscard]] Lesson At(std::size_t group, std::size_t day, std::size_t lesson) const;

private:
    std::vector<Group> groups_;
};

struct ExceedingComplexityItem
{
    std::pair<std::size_t, std::size_t> GroupAndDay = {0,0};
    std::size_t TotalComplexity = 0;
};

struct ExceedingComplexityLess
{
    bool operator()(const ExceedingComplexityItem& lhs, const ExceedingComplexityItem& rhs) const
    {
        return lhs.GroupAndDay < rhs.GroupAndDay;
    }

    bool operator()(const ExceedingComplexityItem& lhs, const std::pair<std::size_t, std::size_t>& rhs) const
    {
        return lhs.GroupAndDay < rhs;
    }

    bool operator()(const std::pair<std::size_t, std::size_t>& lhs, const ExceedingComplexityItem& rhs) const
    {
        return lhs < rhs.GroupAndDay;
    }
};

struct LessonAddress
{
    explicit LessonAddress(std::size_t Group, std::size_t Day, std::size_t Lesson)
        : Group(Group), Day(Day), Lesson(Lesson)
    {
    }

    friend bool operator<(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group < rhs.Group || (!(lhs.Group < rhs.Group) && lhs.Day < rhs.Day) ||
                (!(lhs.Group < rhs.Group) && !(lhs.Day < rhs.Day) && lhs.Lesson < rhs.Lesson);
    }

    friend bool operator==(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group == rhs.Group && lhs.Day == rhs.Day && lhs.Lesson == rhs.Lesson;
    }

    std::size_t Group = 0;
    std::size_t Day = 0;
    std::size_t Lesson = 0;
};

struct OverlappedClassroom
{
    explicit OverlappedClassroom(std::size_t Classroom, SortedSet<LessonAddress> Lessons)
        : Classroom(Classroom)
        , Lessons(std::move(Lessons))
    {}

    std::size_t Classroom = 0;
    SortedSet<LessonAddress> Lessons;
};

void Print(const OverlappedClassroom& overlappedClassroom);

struct OverlappedProfessor
{
    explicit OverlappedProfessor(std::size_t Professor, SortedSet<LessonAddress> Lessons)
        : Professor(Professor)
        , Lessons(std::move(Lessons))
    {}

    std::size_t Professor = 0;
    SortedSet<LessonAddress> Lessons;
};

struct ViolatedSubjectRequest
{
    explicit ViolatedSubjectRequest(std::size_t Subject, SortedSet<LessonAddress> Lessons)
        : Subject(Subject)
        , Lessons(std::move(Lessons))
    {}

    std::size_t Subject = 0;
    SortedSet<LessonAddress> Lessons;
};

struct SubjectWithAddress
{
    explicit SubjectWithAddress(std::size_t Subject, LessonAddress Address)
        : Subject(Subject)
        , Address(Address)
    {}

    friend bool operator<(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject < rhs.Subject;
    }

    friend bool operator==(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject == rhs.Subject;
    }

    friend bool operator!=(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject != rhs.Subject;
    }

    std::size_t Subject;
    LessonAddress Address;
};


std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data, const ScheduleResult& result);
std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data, const ScheduleResult& result);
