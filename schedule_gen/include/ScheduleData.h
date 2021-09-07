#pragma once
#include "ScheduleCommon.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>


class SubjectRequest
{
public:
    SubjectRequest() = default;
    explicit SubjectRequest(std::size_t id,
                            std::size_t professor,
                            std::size_t complexity,
                            WeekDays days,
                            std::vector<std::size_t> groups,
                            std::vector<ClassroomAddress> classrooms,
                            bool isEveningClass = false);

    std::size_t ID() const { return id_; }
    std::size_t Professor() const { return professor_; }
    std::size_t Complexity() const { return complexity_; }
    const std::vector<std::size_t>& Groups() const { return groups_; }
    const std::vector<ClassroomAddress>& Classrooms() const { return classrooms_; }
    bool RequestedWeekDay(std::size_t day) const;
    bool IsEveningClass() const { return isEveningClass_; }

    friend bool operator==(const SubjectRequest& lhs, const SubjectRequest& rhs)
    {
        return lhs.id_ == rhs.id_ &&
            lhs.isEveningClass_ == rhs.isEveningClass_ &&
            lhs.professor_ == rhs.professor_ &&
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
    bool isEveningClass_ = false;
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

    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Professors() const { return professorRequests_; }
    const std::unordered_map<std::size_t, std::unordered_set<std::size_t>>& Groups() const { return groupRequests_; }

private:
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<SubjectWithAddress> lockedLessons_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> professorRequests_;
    std::unordered_map<std::size_t, std::unordered_set<std::size_t>> groupRequests_;
};

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subjectRequestID, std::size_t scheduleDay);
