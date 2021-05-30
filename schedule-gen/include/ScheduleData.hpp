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

    std::size_t ID() const { return id_; }
    std::size_t Professor() const { return professor_; }
    std::size_t Complexity() const { return complexity_; }
    const std::vector<std::size_t>& Groups() const { return groups_; }
    const std::vector<ClassroomAddress>& Classrooms() const { return classrooms_; }
    const WeekDays& RequestedWeekDays() const { return days_; }
    bool RequestedWeekDay(std::size_t day) const;

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

struct SubjectWithAddressLessBySubjectRequestID
{
    bool operator()(const SubjectWithAddress& lhs, const SubjectWithAddress& rhs) const
    {
        return lhs.SubjectRequestID < rhs.SubjectRequestID;
    }

    bool operator()(const SubjectWithAddress& lhs, std::size_t rhsSubjectRequestID) const
    {
        return lhs.SubjectRequestID < rhsSubjectRequestID;
    }

    bool operator()(std::size_t lhsSubjectRequestID, const SubjectWithAddress& rhs) const
    {
        return lhsSubjectRequestID < rhs.SubjectRequestID;
    }
};

class ScheduleData
{
public:
    ScheduleData() = default;
    explicit ScheduleData(std::vector<SubjectRequest> subjectRequests,
                          std::vector<SubjectWithAddress> lockedLessons);

    const std::vector<SubjectRequest>& SubjectRequests() const { return subjectRequests_; }
    const SubjectRequest& SubjectRequestAtID(std::size_t subjectRequestID) const;
    std::size_t IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const;

    const std::vector<SubjectWithAddress>& LockedLessons() const { return lockedLessons_; }
    bool SubjectRequestHasLockedLesson(const SubjectRequest& request) const;

private:
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<SubjectWithAddress> lockedLessons_;
};

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subjectRequestID, std::size_t scheduleDay);
