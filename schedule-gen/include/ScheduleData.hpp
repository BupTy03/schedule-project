#pragma once
#include "ScheduleCommon.hpp"
#include <string>
#include <cstdint>
#include <map>
#include <set>
#include <vector>


class SubjectRequest
{
public:
    SubjectRequest() = default;
    explicit SubjectRequest(std::size_t professor,
                            std::size_t hours,
                            std::size_t complexity,
                            WeekDays days,
                            SortedSet<std::size_t> groups,
                            SortedSet<std::size_t> classrooms);

    [[nodiscard]] bool RequestedClassroom(std::size_t c) const;
    [[nodiscard]] bool RequestedGroup(std::size_t g) const;
    [[nodiscard]] bool Requested(WeekDay d) const;
    [[nodiscard]] std::size_t HoursPerWeek() const;
    [[nodiscard]] std::size_t Complexity() const;
    [[nodiscard]] std::size_t Professor() const;
    [[nodiscard]] const std::vector<std::size_t>& Groups() const;
    [[nodiscard]] const std::vector<std::size_t>& Classrooms() const;
    [[nodiscard]] bool RequestedWeekDay(std::size_t day) const;

    friend bool operator==(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return lhs.professor_ == rhs.professor_ &&
            lhs.hours_ == rhs.hours_ &&
            lhs.complexity_ == rhs.complexity_ &&
            lhs.days_ == rhs.days_ &&
            lhs.groups_ == rhs.groups_ &&
            lhs.classrooms_ == rhs.classrooms_;
    }

private:
    std::size_t professor_ = 0;
    std::size_t hours_ = 0;
    std::size_t complexity_ = 0;
    WeekDays days_;
    SortedSet<std::size_t> groups_;
    SortedSet<std::size_t> classrooms_;
};

struct SubjectWithAddress
{
    SubjectWithAddress() = default;
    explicit SubjectWithAddress(std::size_t Subject, LessonAddress Address)
            : Subject(Subject)
            , Address(Address)
    {}

    [[nodiscard]] friend bool operator<(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject < rhs.Subject;
    }

    [[nodiscard]] friend bool operator==(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject == rhs.Subject;
    }

    [[nodiscard]] friend bool operator!=(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.Subject != rhs.Subject;
    }

    std::size_t Subject = 0;
    LessonAddress Address;
};

struct SubjectWithAddressLess
{
    bool operator()(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs) const
    {
        return lhs.Address < rhs.Address;
    }

    bool operator()(const SubjectWithAddress& lhs, const LessonAddress& rhsAddress) const
    {
        return lhs.Address < rhsAddress;
    }

    bool operator()(const LessonAddress& lhsAddress, const SubjectWithAddress& rhs) const
    {
        return lhsAddress < rhs.Address;
    }
};

class ScheduleData
{
public:
    explicit ScheduleData(std::vector<std::size_t> groups,
                          std::vector<std::size_t> professors,
                          std::vector<std::size_t> classrooms,
                          std::vector<SubjectRequest> subjectRequests,
                          std::vector<SubjectWithAddress> occupiedLessons);

    [[nodiscard]] const std::vector<std::size_t>& Groups() const;
    [[nodiscard]] const std::vector<std::size_t>& Professors() const;
    [[nodiscard]] const std::vector<std::size_t>& Classrooms() const;
    [[nodiscard]] std::size_t CountSubjects() const;
    [[nodiscard]] const std::vector<SubjectRequest>& SubjectRequests() const;
    [[nodiscard]] bool LessonIsOccupied(const LessonAddress& lessonAddress) const;

private:
    std::vector<std::size_t> groups_;
    std::vector<std::size_t> professors_;
    std::vector<std::size_t> classrooms_;
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<SubjectWithAddress> occupiedLessons_;
};


enum class ScheduleDataValidationResult
{
    Ok,
    ToMuchLessonsPerDayRequested,
    NoGroups,
    NoSubjects,
    NoProfessors,
    NoClassrooms
};

[[nodiscard]] ScheduleDataValidationResult Validate(const ScheduleData& data);
[[nodiscard]] std::size_t CalculateHours(const ScheduleData& data, std::size_t professor, std::size_t group, std::size_t subject);
[[nodiscard]] bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t scheduleDay);
[[nodiscard]] bool ClassroomRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t classroom);
