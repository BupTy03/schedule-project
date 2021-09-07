#include "ScheduleData.h"
#include <string>
#include <algorithm>


SubjectRequest::SubjectRequest(std::size_t id,
                               std::size_t professor,
                               std::size_t complexity,
                               WeekDays days,
                               std::vector<std::size_t> groups,
                               std::vector<ClassroomAddress> classrooms,
                               ClassesType classesType)
        : isEveningClass_(classesType == ClassesType::Evening)
        , id_(id)
        , professor_(professor)
        , complexity_(complexity)
        , days_(days)
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
{
    if(days_ == WeekDays::emptyWeek())
        days_ = WeekDays::fullWeek();

    std::ranges::sort(groups_);
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::ranges::sort(classrooms_);
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());
}

bool SubjectRequest::RequestedWeekDay(std::size_t day) const
{
    return days_.contains(static_cast<WeekDay>(day % DAYS_IN_SCHEDULE_WEEK));
}


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> lockedLessons)
    : subjectRequests_(std::move(subjectRequests))
    , lockedLessons_(std::move(lockedLessons))
    , professorRequests_()
    , groupRequests_()
{
    std::ranges::sort(subjectRequests_, {}, &SubjectRequest::ID);
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(),
                                       SubjectRequestIDEqual()), subjectRequests_.end());

    std::ranges::sort(lockedLessons_, {}, &SubjectWithAddress::SubjectRequestID);
    lockedLessons_.erase(std::unique(lockedLessons_.begin(), lockedLessons_.end()), lockedLessons_.end());

    for(std::size_t r = 0; r < subjectRequests_.size(); ++r)
    {
        const auto& request = subjectRequests_.at(r);
        professorRequests_[request.Professor()].insert(r);
        for(std::size_t g : request.Groups())
            groupRequests_[g].insert(r);
    }
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

std::size_t ScheduleData::IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return std::distance(subjectRequests_.begin(), it);
}

bool ScheduleData::SubjectRequestHasLockedLesson(const SubjectRequest& request) const
{
    return std::ranges::binary_search(lockedLessons_, request.ID(), {}, &SubjectWithAddress::SubjectRequestID);
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subjectRequestID, std::size_t scheduleDay)
{
    return data.SubjectRequestAtID(subjectRequestID).RequestedWeekDay(scheduleDay);
}
