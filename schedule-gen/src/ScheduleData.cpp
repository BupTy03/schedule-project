#include "ScheduleData.hpp"
#include <algorithm>
#include <iostream>


SubjectRequest::SubjectRequest(std::size_t id,
                               std::size_t professor,
                               std::size_t complexity,
                               WeekDays days,
                               SortedSet<std::size_t> groups,
                               SortedSet<std::size_t> classrooms)
        : id_(id)
        , professor_(professor)
        , complexity_(complexity)
        , days_(days)
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
{
    if(days_ == WeekDays::emptyWeek())
        days_ = WeekDays::fullWeek();
}

bool SubjectRequest::RequestedClassroom(std::size_t c) const { return classrooms_.contains(c); }

bool SubjectRequest::RequestedGroup(std::size_t g) const { return groups_.contains(g); }

bool SubjectRequest::Requested(WeekDay d) const { return days_.contains(d); }

std::size_t SubjectRequest::Professor() const { return professor_; }

std::size_t SubjectRequest::Complexity() const { return complexity_; }

std::size_t SubjectRequest::ID() const { return id_; }

const std::vector<std::size_t>& SubjectRequest::Groups() const { return groups_.elems(); }

const std::vector<std::size_t>& SubjectRequest::Classrooms() const { return classrooms_.elems(); }

bool SubjectRequest::RequestedWeekDay(std::size_t day) const { return days_.contains(static_cast<WeekDay>(day % 6)); }


ScheduleData::ScheduleData(std::vector<std::size_t> groups,
                           std::vector<std::size_t> professors,
                           std::vector<std::size_t> classrooms,
                           std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> occupiedLessons)
        : groups_(std::move(groups))
        , professors_(std::move(professors))
        , classrooms_(std::move(classrooms))
        , subjectRequests_(std::move(subjectRequests))
        , occupiedLessons_(std::move(occupiedLessons))
{
    std::sort(groups_.begin(), groups_.end());
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::sort(professors_.begin(), professors_.end());
    professors_.erase(std::unique(professors_.begin(), professors_.end()), professors_.end());

    std::sort(classrooms_.begin(), classrooms_.end());
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());

    std::sort(occupiedLessons_.begin(), occupiedLessons_.end());
    occupiedLessons_.erase(std::unique(occupiedLessons_.begin(), occupiedLessons_.end()), occupiedLessons_.end());

    std::sort(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDLess());
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(), SubjectRequestIDEqual()), subjectRequests_.end());
}

const std::vector<std::size_t>& ScheduleData::Groups() const { return groups_; }

const std::vector<std::size_t>& ScheduleData::Professors() const { return professors_; }

const std::vector<std::size_t>& ScheduleData::Classrooms() const { return classrooms_; }

std::size_t ScheduleData::CountSubjects() const { return subjectRequests_.size(); }

const std::vector<SubjectRequest>& ScheduleData::SubjectRequests() const { return subjectRequests_; }

bool ScheduleData::LessonIsOccupied(const LessonAddress& lessonAddress) const
{
    auto it = std::lower_bound(occupiedLessons_.begin(), occupiedLessons_.end(), lessonAddress, SubjectWithAddressLess());
    return it != occupiedLessons_.end() && it->Address == lessonAddress;
}


ScheduleDataValidationResult Validate(const ScheduleData& data)
{
    if (std::empty(data.Groups()))
        return ScheduleDataValidationResult::NoGroups;

    if (std::empty(data.Professors()))
        return ScheduleDataValidationResult::NoProfessors;

    if (std::empty(data.Classrooms()))
        return ScheduleDataValidationResult::NoClassrooms;

    if (data.CountSubjects() <= 0)
        return ScheduleDataValidationResult::NoSubjects;

    if(data.CountSubjects() > MAX_LESSONS_COUNT)
        return ScheduleDataValidationResult::ToMuchLessonsPerDayRequested;

    return ScheduleDataValidationResult::Ok;
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t scheduleDay)
{
    return data.SubjectRequests().at(subject).Requested(ScheduleDayNumberToWeekDay(scheduleDay));
}

bool ClassroomRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t classroom)
{
    return data.SubjectRequests().at(subject).RequestedClassroom(classroom);
}

std::size_t CalculateHours(const ScheduleData& data, std::size_t professor, std::size_t group, std::size_t subject)
{
    const auto& subj = data.SubjectRequests().at(subject);
    if(subj.Professor() != professor)
        return 0;

    if(!subj.RequestedGroup(group))
        return 0;

    return 1;
}
