#include "ScheduleData.hpp"
#include <algorithm>
#include <iostream>


SubjectRequest::SubjectRequest(std::size_t id,
                               std::size_t professor,
                               std::size_t complexity,
                               WeekDays days,
                               std::vector<std::size_t> groups,
                               std::vector<ClassroomAddress> classrooms)
        : id_(id)
        , professor_(professor)
        , complexity_(complexity)
        , days_(days)
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
{
    if(days_ == WeekDays::emptyWeek())
        days_ = WeekDays::fullWeek();

    std::sort(groups_.begin(), groups_.end());
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::sort(classrooms_.begin(), classrooms_.end());
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
{
    std::sort(lockedLessons_.begin(), lockedLessons_.end(), SubjectWithAddressLessByAddress());
    lockedLessons_.erase(std::unique(lockedLessons_.begin(), lockedLessons_.end()), lockedLessons_.end());

    std::sort(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDLess());
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDEqual()), subjectRequests_.end());
}

bool ScheduleData::LessonIsLocked(std::size_t lessonAddress) const
{
    auto it = std::lower_bound(lockedLessons_.begin(), lockedLessons_.end(), lessonAddress, SubjectWithAddressLessByAddress());
    return it != lockedLessons_.end() && it->Address == lessonAddress;
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::lower_bound(subjectRequests_.begin(), subjectRequests_.end(), subjectRequestID, SubjectRequestIDLess());
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

bool ScheduleData::RequestHasLockedLesson(const SubjectRequest& request) const
{
    auto it = std::find_if(lockedLessons_.begin(), lockedLessons_.end(), [&](const SubjectWithAddress& subject){
        return subject.SubjectRequestID == request.ID();
    });

    return it != lockedLessons_.end();
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subjectRequestID, std::size_t scheduleDay)
{
    return data.SubjectRequestAtID(subjectRequestID).RequestedWeekDay(scheduleDay);
}
