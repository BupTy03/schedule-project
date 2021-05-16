#include "ScheduleData.hpp"
#include <algorithm>
#include <iostream>


SubjectRequest::SubjectRequest(std::size_t professor,
                               std::size_t hours,
                               std::size_t complexity,
                               WeekDays days,
                               SortedSet<std::size_t> groups,
                               SortedSet<std::size_t> classrooms)
        : professor_(professor)
        , hours_(hours)
        , complexity_(complexity)
        , days_(days)
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
{
    if(days_.empty())
        days_ = WeekDays::fullWeek();
}

bool SubjectRequest::RequestedClassroom(std::size_t c) const { return classrooms_.contains(c); }

bool SubjectRequest::RequestedGroup(std::size_t g) const { return groups_.contains(g); }

bool SubjectRequest::Requested(WeekDay d) const { return days_.contains(d); }

std::size_t SubjectRequest::HoursPerWeek() const { return hours_; }

std::size_t SubjectRequest::Professor() const { return professor_; }

std::size_t SubjectRequest::Complexity() const { return complexity_; }

const std::vector<std::size_t>& SubjectRequest::Groups() const { return groups_.elems(); }

const std::vector<std::size_t>& SubjectRequest::Classrooms() const { return classrooms_.elems(); }

bool SubjectRequest::RequestedWeekDay(std::size_t day) const { return days_.contains(static_cast<WeekDay>(day % 6)); }


ScheduleData::ScheduleData(std::size_t countGroups,
                           std::size_t countProfessors,
                           std::size_t countClassrooms,
                           std::vector<SubjectRequest> subjectRequests,
                           std::vector<SubjectWithAddress> occupiedLessons)
        : countGroups_(countGroups)
        , countProfessors_(countProfessors)
        , countClassrooms_(countClassrooms)
        , subjectRequests_(std::move(subjectRequests))
        , occupiedLessons_(std::move(occupiedLessons))
{
    std::sort(occupiedLessons_.begin(), occupiedLessons_.end());
}

std::size_t ScheduleData::CountGroups() const { return countGroups_; }

std::size_t ScheduleData::CountSubjects() const { return subjectRequests_.size(); }

std::size_t ScheduleData::CountProfessors() const { return countProfessors_; }

std::size_t ScheduleData::CountClassrooms() const { return countClassrooms_; }

const std::vector<SubjectRequest>& ScheduleData::SubjectRequests() const { return subjectRequests_; }

bool ScheduleData::LessonIsOccupied(const LessonAddress& lessonAddress) const
{
    auto it = std::lower_bound(occupiedLessons_.begin(), occupiedLessons_.end(), lessonAddress, SubjectWithAddressLess());
    return it != occupiedLessons_.end() && it->Address == lessonAddress;
}


void Print(const ScheduleData& data)
{
    std::cout << "CountProfessors: " << data.CountProfessors() << '\n';
    std::cout << "CountGroups: " << data.CountGroups() << '\n';
    std::cout << "CountSubjects: " << data.CountSubjects() << '\n';
    std::cout << "CountClassrooms: " << data.CountClassrooms() << std::endl;
}

ScheduleDataValidationResult Validate(const ScheduleData& data)
{
    if (data.CountGroups() <= 0)
        return ScheduleDataValidationResult::NoGroups;

    if (data.CountProfessors() <= 0)
        return ScheduleDataValidationResult::NoProfessors;

    if (data.CountClassrooms() <= 0)
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

    return subj.HoursPerWeek();
}
