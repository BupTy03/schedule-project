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
    explicit ScheduleItem(std::size_t lessonAddress,
                          std::size_t subjectRequest,
                          std::size_t subjectRequestID,
                          std::size_t classroom);

    std::size_t Address;
    std::size_t SubjectRequest;
    std::size_t SubjectRequestID;
    std::size_t Classroom;
};

struct ScheduleItemLess
{
    [[nodiscard]] bool operator()(const ScheduleItem& lhs, const ScheduleItem& rhs) const
    {
        return lhs.Address < rhs.Address;
    }
    [[nodiscard]] bool operator()(const ScheduleItem& lhs, std::size_t rhsLessonAddress) const
    {
        return lhs.Address < rhsLessonAddress;
    }
    [[nodiscard]] bool operator()(std::size_t lhsLessonAddress, const ScheduleItem& rhs) const
    {
        return lhsLessonAddress < rhs.Address;
    }
};

class ScheduleResult
{
public:
    ScheduleResult() = default;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<ScheduleItem>& items() const;

    std::vector<ScheduleItem>::iterator insert(const ScheduleItem& item);
    [[nodiscard]] const ScheduleItem* at(std::size_t lessonAddress) const;

private:
    std::vector<ScheduleItem> items_;
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
    [[nodiscard]] bool operator()(const ViolatedSubjectRequest& lhs, const ViolatedSubjectRequest& rhs) const
    {
        return lhs.Subject < rhs.Subject;
    }

    [[nodiscard]] bool operator()(const ViolatedSubjectRequest& lhs, std::size_t rhsSubject) const
    {
        return lhs.Subject < rhsSubject;
    }

    [[nodiscard]] bool operator()(std::size_t lhsSubject, const ViolatedSubjectRequest& rhs) const
    {
        return lhsSubject < rhs.Subject;
    }
};



[[nodiscard]] std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data,
                                                                        const ScheduleResult& result);

[[nodiscard]] std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data,
                                                                        const ScheduleResult& result);

[[nodiscard]] std::vector<ViolatedSubjectRequest> FindViolatedSubjectRequests(const ScheduleData& data,
                                                                              const ScheduleResult& result);
