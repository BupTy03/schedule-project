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

bool SubjectRequest::RequestedClassroom(const ClassroomAddress& classroomAddress) const
{
    return std::binary_search(classrooms_.begin(), classrooms_.end(), classroomAddress);
}

bool SubjectRequest::RequestedGroup(std::size_t g) const
{
    return std::binary_search(groups_.begin(), groups_.end(), g);
}

bool SubjectRequest::Requested(WeekDay d) const { return days_.contains(d); }

std::size_t SubjectRequest::Professor() const { return professor_; }

std::size_t SubjectRequest::Complexity() const { return complexity_; }

std::size_t SubjectRequest::ID() const { return id_; }

const std::vector<std::size_t>& SubjectRequest::Groups() const { return groups_; }

const std::vector<ClassroomAddress>& SubjectRequest::Classrooms() const { return classrooms_; }

bool SubjectRequest::RequestedWeekDay(std::size_t day) const { return days_.contains(static_cast<WeekDay>(day % 6)); }


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> lockedLessons)
        : subjectRequests_(std::move(subjectRequests))
        , lockedLessons_(std::move(lockedLessons))
{
    std::sort(lockedLessons_.begin(), lockedLessons_.end(), SubjectWithAddressLess());
    lockedLessons_.erase(std::unique(lockedLessons_.begin(), lockedLessons_.end()), lockedLessons_.end());

    std::sort(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDLess());
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDEqual()), subjectRequests_.end());
}

const std::vector<SubjectRequest>& ScheduleData::SubjectRequests() const { return subjectRequests_; }

bool ScheduleData::LessonIsOccupied(std::size_t lessonAddress) const
{
    auto it = std::lower_bound(lockedLessons_.begin(), lockedLessons_.end(), lessonAddress, SubjectWithAddressLess());
    return it != lockedLessons_.end() && it->Address == lessonAddress;
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::lower_bound(subjectRequests_.begin(), subjectRequests_.end(), subjectRequestID, SubjectRequestIDLess());
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

const std::vector<SubjectWithAddress>& ScheduleData::LockedLessons() const
{
    return lockedLessons_;
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subjectRequestID, std::size_t scheduleDay)
{
    return data.SubjectRequestAtID(subjectRequestID).Requested(ScheduleDayNumberToWeekDay(scheduleDay));
}
