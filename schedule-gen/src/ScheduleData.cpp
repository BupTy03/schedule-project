#include "ScheduleData.hpp"
#include <algorithm>
#include <iostream>


SubjectRequest::SubjectRequest(std::size_t professor,
                               std::size_t hours,
                               std::size_t complexity,
                               std::set<WeekDay> days,
                               std::set<std::size_t> groups,
                               std::set<std::size_t> classrooms)
        : professor_(professor)
        , hours_(hours)
        , complexity_(complexity)
        , days_(std::move(days))
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
{}

bool SubjectRequest::RequestedClassroom(std::size_t c) const
{
    return classrooms_.count(c) > 0;
}

bool SubjectRequest::RequestedGroup(std::size_t g) const
{
    return groups_.count(g) > 0;
}

bool SubjectRequest::Requested(WeekDay d) const
{
    return days_.count(d) > 0;
}

std::size_t SubjectRequest::HoursPerWeek() const
{
    return hours_;
}

std::size_t SubjectRequest::Professor() const
{
    return professor_;
}

std::size_t SubjectRequest::Complexity() const
{
    return complexity_;
}


ScheduleData::ScheduleData(std::size_t countLessonsPerDay, std::size_t countGroups, std::size_t countProfessors,
                           std::size_t countClassrooms, std::vector<SubjectRequest> subjectRequests)
        : countLessonsPerDay_(countLessonsPerDay)
        , countGroups_(countGroups)
        , countProfessors_(countProfessors)
        , countClassrooms_(countClassrooms)
        , subjectRequests_(std::move(subjectRequests))
{
}

std::size_t ScheduleData::MaxCountLessonsPerDay() const
{
    return 6;
}

std::size_t ScheduleData::RequestedCountLessonsPerDay() const
{
    return countLessonsPerDay_;
}

std::size_t ScheduleData::CountGroups() const
{
    return countGroups_;
}

std::size_t ScheduleData::CountSubjects() const
{
    return subjectRequests_.size();
}

std::size_t ScheduleData::CountProfessors() const
{
    return countProfessors_;
}

std::size_t ScheduleData::CountClassrooms() const
{
    return countClassrooms_;
}

const std::vector<SubjectRequest> &ScheduleData::SubjectRequests() const
{
    return subjectRequests_;
}


void Print(const ScheduleData& data)
{
    std::cout << "MaxCountLessonsPerDay: " << data.MaxCountLessonsPerDay() << '\n';
    std::cout << "CountProfessors: " << data.CountProfessors() << '\n';
    std::cout << "CountGroups: " << data.CountGroups() << '\n';
    std::cout << "CountSubjects: " << data.CountSubjects() << '\n';
    std::cout << "CountClassrooms: " << data.CountClassrooms() << '\n';
    std::cout << "RequestedCountLessonsPerDay: " << data.RequestedCountLessonsPerDay() << std::endl;
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

    if(data.CountSubjects() > data.MaxCountLessonsPerDay() * SCHEDULE_DAYS_COUNT)
        return ScheduleDataValidationResult::ToFewLessonsPerDayRequested;

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
