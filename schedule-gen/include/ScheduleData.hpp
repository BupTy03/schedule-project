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
    explicit SubjectRequest(std::size_t id,
                            std::size_t professor,
                            std::size_t complexity,
                            WeekDays days,
                            std::vector<std::size_t> groups,
                            std::vector<ClassroomAddress> classrooms);

    [[nodiscard]] bool RequestedClassroom(const ClassroomAddress& classroomAddress) const;
    [[nodiscard]] bool RequestedGroup(std::size_t g) const;
    [[nodiscard]] bool Requested(WeekDay d) const;
    [[nodiscard]] std::size_t Complexity() const;
    [[nodiscard]] std::size_t Professor() const;
    [[nodiscard]] std::size_t ID() const;
    [[nodiscard]] const std::vector<std::size_t>& Groups() const;
    [[nodiscard]] const std::vector<ClassroomAddress>& Classrooms() const;
    [[nodiscard]] bool RequestedWeekDay(std::size_t day) const;

    friend bool operator==(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return lhs.professor_ == rhs.professor_ &&
            lhs.complexity_ == rhs.complexity_ &&
            lhs.days_ == rhs.days_ &&
            lhs.groups_ == rhs.groups_ &&
            lhs.classrooms_ == rhs.classrooms_;
    }

    friend bool operator!=(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return !(lhs == rhs);
    }

private:
    std::size_t id_ = 0;
    std::size_t professor_ = 0;
    std::size_t complexity_ = 0;
    WeekDays days_;
    std::vector<std::size_t> groups_;
    std::vector<ClassroomAddress> classrooms_;
};

struct SubjectRequestIDLess
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() < rhs.ID();
    }
    bool operator()(const SubjectRequest& lhs, std::size_t rhsID) const
    {
        return lhs.ID() < rhsID;
    }
    bool operator()(std::size_t lhsID, const SubjectRequest& rhs) const
    {
        return lhsID < rhs.ID();
    }
};

struct SubjectRequestIDEqual
{
    bool operator()(const SubjectRequest& lhs, const SubjectRequest& rhs) const
    {
        return lhs.ID() == rhs.ID();
    }
};


struct SubjectWithAddress
{
    SubjectWithAddress() = default;
    explicit SubjectWithAddress(std::size_t SubjectRequestID, std::size_t Address)
            : SubjectRequestID(SubjectRequestID)
            , Address(Address)
    {}

    friend bool operator==(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return lhs.SubjectRequestID == rhs.SubjectRequestID && lhs.Address == rhs.Address;
    }

    friend bool operator!=(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t SubjectRequestID = 0;
    std::size_t Address = 0;
};

struct SubjectWithAddressLess
{
    bool operator()(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs) const
    {
        return lhs.Address < rhs.Address;
    }

    bool operator()(const SubjectWithAddress& lhs, std::size_t rhsAddress) const
    {
        return lhs.Address < rhsAddress;
    }

    bool operator()(std::size_t lhsAddress, const SubjectWithAddress& rhs) const
    {
        return lhsAddress < rhs.Address;
    }
};

class ScheduleData
{
public:
    ScheduleData() = default;
    explicit ScheduleData(std::vector<std::size_t> groups,
                          std::vector<std::size_t> professors,
                          std::vector<ClassroomAddress> classrooms,
                          std::vector<SubjectRequest> subjectRequests,
                          std::vector<SubjectWithAddress> lockedLessons);

    const std::vector<std::size_t>& Groups() const;
    const std::vector<std::size_t>& Professors() const;
    const std::vector<ClassroomAddress>& Classrooms() const;
    std::size_t CountSubjects() const;
    const std::vector<SubjectRequest>& SubjectRequests() const;
    const SubjectRequest& SubjectRequestAtID(std::size_t subjectRequestID) const;
    const std::vector<SubjectWithAddress>& LockedLessons() const;
    bool LessonIsOccupied(std::size_t lessonAddress) const;

private:
    std::vector<std::size_t> groups_;
    std::vector<std::size_t> professors_;
    std::vector<ClassroomAddress> classrooms_;
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<SubjectWithAddress> lockedLessons_;
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
[[nodiscard]] bool ClassroomRequestedForSubject(const ScheduleData& data, std::size_t subject, const ClassroomAddress& classroomAddress);
