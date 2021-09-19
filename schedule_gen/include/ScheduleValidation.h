#pragma once
#include "ScheduleData.h"
#include "ScheduleResult.h"

#include <vector>


struct OverlappedClassroom
{
    friend bool operator==(const OverlappedClassroom& lhs, const OverlappedClassroom& rhs)
    {
        return lhs.Address == rhs.Address && lhs.Classroom == rhs.Classroom
               && lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
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
        return lhs.Address == rhs.Address && lhs.Professor == rhs.Professor
               && lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
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
        return lhs.Address == rhs.Address && lhs.Groups == rhs.Groups
               && lhs.SubjectRequestsIDs == rhs.SubjectRequestsIDs;
    }
    friend bool operator!=(const OverlappedGroups& lhs, const OverlappedGroups& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Address = 0;
    std::vector<std::size_t> Groups;
    std::vector<std::size_t> SubjectRequestsIDs;
};

struct ViolatedLessonRequest
{
    friend bool operator==(const ViolatedLessonRequest& lhs, const ViolatedLessonRequest& rhs)
    {
        return lhs.Address == rhs.Address && lhs.SubjectRequestID == rhs.SubjectRequestID;
    }
    friend bool operator!=(const ViolatedLessonRequest& lhs, const ViolatedLessonRequest& rhs)
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
    std::vector<ViolatedLessonRequest> ViolatedLessons;
    std::vector<std::size_t> OutOfBlockRequests;
};

bool empty(const CheckScheduleResult& r);

std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data,
                                                          const ScheduleResult& result);

std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data,
                                                          const ScheduleResult& result);

std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                   const ScheduleResult& result);

std::vector<ViolatedLessonRequest> FindViolatedLessons(const ScheduleData& data,
                                                       const ScheduleResult& result);

std::vector<std::size_t> FindOutOfBlockRequests(const ScheduleData& data,
                                                const ScheduleResult& result);

CheckScheduleResult CheckSchedule(const ScheduleData& data, const ScheduleResult& result);
