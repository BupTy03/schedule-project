#include "ScheduleResult.hpp"
#include "ScheduleData.hpp"

#include <iostream>
#include <functional>
#include <algorithm>


ScheduleItem::ScheduleItem(std::size_t lessonAddress,
                           std::size_t subjectRequest,
                           std::size_t subjectRequestID,
                           std::size_t classroom)
        : Address(lessonAddress)
        , SubjectRequest(subjectRequest)
        , SubjectRequestID(subjectRequestID)
        , Classroom(classroom)
{}


bool ScheduleResult::empty() const { return items_.empty(); }

const std::vector<ScheduleItem>& ScheduleResult::items() const { return items_; }

Range<std::vector<ScheduleItem>::const_iterator> ScheduleResult::at(std::size_t lessonAddress) const
{
    auto itPair = std::equal_range(items_.begin(), items_.end(), lessonAddress, ScheduleItemLess());
    return make_range(itPair.first, itPair.second);
}

std::vector<ScheduleItem>::iterator ScheduleResult::insert(const ScheduleItem& item)
{
    auto it = std::lower_bound(items_.begin(), items_.end(), item.Address, ScheduleItemLess());
    return items_.insert(it, item);
}

std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedClassroom> overlappedClassrooms;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::map<std::size_t, std::vector<std::size_t>> classroomsAndSubjects;
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
            classroomsAndSubjects[item.Classroom].emplace_back(item.SubjectRequest);

        for(auto&[classroom, subjects] : classroomsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                std::sort(subjects.begin(), subjects.end());

                OverlappedClassroom overlappedClassroom;
                overlappedClassroom.Address = l;
                overlappedClassroom.Classroom = classroom;
                overlappedClassroom.SubjectRequestsIDs = std::move(subjects);
                overlappedClassrooms.emplace_back(std::move(overlappedClassroom));
            }
        }
    }

    return overlappedClassrooms;
}

std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedProfessor> overlappedProfessors;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::map<std::size_t, std::vector<std::size_t>> professorsAndSubjects;
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
        {
            const auto& request = data.SubjectRequests().at(item.SubjectRequest);
            professorsAndSubjects[request.Professor()].emplace_back(item.SubjectRequest);
        }

        for(auto&[professor, subjects] : professorsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                std::sort(subjects.begin(), subjects.end());

                OverlappedProfessor overlappedProfessor;
                overlappedProfessor.Address = l;
                overlappedProfessor.Professor = professor;
                overlappedProfessor.SubjectRequestsIDs = std::move(subjects);
                overlappedProfessors.emplace_back(std::move(overlappedProfessor));
            }
        }
    }

    return overlappedProfessors;
}

std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                   const ScheduleResult& result)
{
    std::vector<OverlappedGroups> overlapped;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        const auto lessonsRange = result.at(l);
        std::map<std::pair<std::size_t, std::size_t>, std::vector<std::size_t>> subjectGroupsIntersections;
        for(auto f = lessonsRange.begin(); f != lessonsRange.end(); ++f)
        {
            for(auto s = lessonsRange.begin(); s != lessonsRange.end(); ++s)
            {
                if(f == s)
                    continue;

                if(subjectGroupsIntersections.count({s->SubjectRequestID, f->SubjectRequestID}))
                    continue;

                const auto& firstGroups = data.SubjectRequests().at(f->SubjectRequest).Groups();
                const auto& secondGroups = data.SubjectRequests().at(s->SubjectRequest).Groups();

                std::vector<std::size_t> intersectedGroups;
                std::set_intersection(firstGroups.begin(), firstGroups.begin(), secondGroups.begin(), secondGroups.end(), std::back_inserter(intersectedGroups));
                if(intersectedGroups.empty())
                    continue;

                subjectGroupsIntersections.emplace(std::pair{f->SubjectRequestID, s->SubjectRequestID}, std::move(intersectedGroups));
            }
        }

        for(auto&[subjPair, groups] : subjectGroupsIntersections)
        {
            OverlappedGroups overlappedGroups;
            overlappedGroups.Address = l;
            overlappedGroups.Groups = std::move(groups);
            overlappedGroups.SubjectRequestsIDs = {subjPair.first, subjPair.second};
            overlapped.emplace_back(std::move(overlappedGroups));
        }
    }

    return overlapped;
}

std::vector<ViolatedWeekdayRequest> FindViolatedWeekdayRequests(const ScheduleData& data,
                                                                const ScheduleResult& result)
{
    std::vector<ViolatedWeekdayRequest> violatedRequests;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        for(auto&& item : result.at(l))
        {
            const std::size_t day = LessonToScheduleDay(item.Address);
            const auto& request = data.SubjectRequests().at(item.SubjectRequest);
            if(!WeekDayRequestedForSubject(data, item.SubjectRequest, day))
            {
                ViolatedWeekdayRequest violatedRequest;
                violatedRequest.Address = l;
                violatedRequest.SubjectRequestID = item.SubjectRequestID;
                violatedRequests.emplace_back(violatedRequest);
            }
        }
    }

    return violatedRequests;
}

CheckScheduleResult CheckSchedule(const ScheduleData& data,
                                  const ScheduleResult& result)
{
    CheckScheduleResult checkResult;
    checkResult.OverlappedClassroomsList = FindOverlappedClassrooms(data, result);
    checkResult.OverlappedProfessorsList = FindOverlappedProfessors(data, result);
    checkResult.OverlappedGroupsList = FindOverlappedGroups(data, result);
    checkResult.ViolatedWeekdayRequestsList = FindViolatedWeekdayRequests(data, result);
    return checkResult;
}
