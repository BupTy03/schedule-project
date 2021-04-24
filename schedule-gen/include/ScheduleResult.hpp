#pragma once
#include "ScheduleCommon.hpp"

#include <map>
#include <cstdint>
#include <vector>
#include <optional>
#include <stdexcept>


class ScheduleData;

struct ScheduleItem
{
    explicit ScheduleItem(const LessonAddress& address,
                          std::size_t subject,
                          std::size_t professor,
                          std::size_t classroom);

    LessonAddress Address;
    std::size_t Subject;
    std::size_t Professor;
    std::size_t Classroom;
};

struct ScheduleItemLess
{
    bool operator()(const ScheduleItem& lhs, const ScheduleItem& rhs) const
    {
        return lhs.Address < rhs.Address;
    }

    bool operator()(const ScheduleItem& lhs, const LessonAddress& rhsAddress) const
    {
        return lhs.Address < rhsAddress;
    }

    bool operator()(const LessonAddress& lhsAddress, const ScheduleItem& rhs) const
    {
        return lhsAddress < rhs.Address;
    }
};

class ScheduleResult
{
public:
    explicit ScheduleResult(std::vector<ScheduleItem> items);
    [[nodiscard]] bool Empty() const;
    [[nodiscard]] const std::vector<ScheduleItem>& Items() const;
    [[nodiscard]] const ScheduleItem* At(const LessonAddress& address) const;

private:
    std::vector<ScheduleItem> items_;
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

    bool operator()(const ExceedingComplexityItem& lhs, const std::pair<std::size_t, std::size_t>& rhsGroupAndDay) const
    {
        return lhs.GroupAndDay < rhsGroupAndDay;
    }

    bool operator()(const std::pair<std::size_t, std::size_t>& lhsGroupAndDay, const ExceedingComplexityItem& rhs) const
    {
        return lhsGroupAndDay < rhs.GroupAndDay;
    }
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
    explicit ViolatedSubjectRequest(std::size_t Subject)
        : Subject(Subject)
        , Lessons()
    {}

    explicit ViolatedSubjectRequest(std::size_t Subject, SortedSet<LessonAddress> Lessons)
        : Subject(Subject)
        , Lessons(std::move(Lessons))
    {}

    std::size_t Subject = 0;
    SortedSet<LessonAddress> Lessons;
};

struct ViolatedSubjectRequestLess
{
    bool operator()(const ViolatedSubjectRequest& lhs, const ViolatedSubjectRequest& rhs) const
    {
        return lhs.Subject < rhs.Subject;
    }

    bool operator()(const ViolatedSubjectRequest& lhs, std::size_t rhsSubject) const
    {
        return lhs.Subject < rhsSubject;
    }

    bool operator()(std::size_t lhsSubject, const ViolatedSubjectRequest& rhs) const
    {
        return lhsSubject < rhs.Subject;
    }
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
std::vector<ViolatedSubjectRequest> FindViolatedSubjectRequests(const ScheduleData& data, const ScheduleResult& result);
