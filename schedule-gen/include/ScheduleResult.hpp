#pragma once
#include "ScheduleUtils.hpp"
#include "ScheduleCommon.hpp"

#include <map>
#include <cstdint>
#include <vector>
#include <optional>
#include <stdexcept>


class ScheduleData;

struct ScheduleItem
{
    ScheduleItem() = default;
    explicit ScheduleItem(std::size_t lessonAddress,
                          std::size_t subjectRequestID,
                          std::size_t classroom);

    std::size_t Address = 0;
    std::size_t SubjectRequestID = 0;
    std::size_t Classroom = 0;
};

struct ScheduleItemLess
{
    bool operator()(const ScheduleItem& lhs, const ScheduleItem& rhs) const
    {
        return lhs.Address < rhs.Address;
    }
    bool operator()(const ScheduleItem& lhs, std::size_t rhsLessonAddress) const
    {
        return lhs.Address < rhsLessonAddress;
    }
    bool operator()(std::size_t lhsLessonAddress, const ScheduleItem& rhs) const
    {
        return lhsLessonAddress < rhs.Address;
    }
};

class ScheduleResult
{
public:
    ScheduleResult() = default;
    explicit ScheduleResult(std::vector<ScheduleItem> items);

    const std::vector<ScheduleItem>& items() const { return items_; }
    std::vector<ScheduleItem>::iterator insert(const ScheduleItem& item);
    Range<std::vector<ScheduleItem>::const_iterator> at(std::size_t lessonAddress) const;

private:
    std::vector<ScheduleItem> items_;
};


struct OverlappedClassroom
{
    friend bool operator==(const OverlappedClassroom& lhs, const OverlappedClassroom& rhs)
    {
        return lhs.Address == rhs.Address &&
               lhs.Classroom == rhs.Classroom &&
               lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
    }
    friend bool operator!=(const OverlappedClassroom& lhs, const OverlappedClassroom& rhs)
    {
        return !(lhs == rhs);
    }


    std::size_t Address = 0;
    std::size_t Classroom = 0;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct OverlappedProfessor
{
    friend bool operator==(const OverlappedProfessor& lhs, const OverlappedProfessor& rhs)
    {
        return lhs.Address == rhs.Address &&
            lhs.Professor == rhs.Professor &&
            lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
    }
    friend bool operator!=(const OverlappedProfessor& lhs, const OverlappedProfessor& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Address = 0;
    std::size_t Professor = 0;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct OverlappedGroups
{
    friend bool operator==(const OverlappedGroups& lhs, const OverlappedGroups& rhs)
    {
        return lhs.Address == rhs.Address &&
            lhs.Groups == rhs.Groups &&
            lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
    }
    friend bool operator!=(const OverlappedGroups& lhs, const OverlappedGroups& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Address = 0;
    std::vector<std::size_t> Groups;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct ViolatedWeekdayRequest
{
    friend bool operator==(const ViolatedWeekdayRequest& lhs, const ViolatedWeekdayRequest& rhs)
    {
        return lhs.Address == rhs.Address &&
            lhs.SubjectRequestID == rhs.SubjectRequestID;
    }
    friend bool operator!=(const ViolatedWeekdayRequest& lhs, const ViolatedWeekdayRequest& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Address = 0;
    std::size_t SubjectRequestID = 0;
};


struct CheckScheduleResult
{
    std::vector<OverlappedClassroom> OverlappedClassroomsList;
    std::vector<OverlappedProfessor> OverlappedProfessorsList;
    std::vector<OverlappedGroups> OverlappedGroupsList;
    std::vector<ViolatedWeekdayRequest> ViolatedWeekdayRequestsList;
};


std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data,
                                                          const ScheduleResult& result);

std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data,
                                                          const ScheduleResult& result);

std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                   const ScheduleResult& result);

std::vector<ViolatedWeekdayRequest> FindViolatedWeekdayRequests(const ScheduleData& data,
                                                                const ScheduleResult& result);

CheckScheduleResult CheckSchedule(const ScheduleData& data,
                                  const ScheduleResult& result);
