#pragma once
#include "ScheduleCommon.hpp"
#include <string>
#include <cstdint>
#include <map>
#include <set>
#include <vector>


class SubjectRequest
{
public:
    explicit SubjectRequest(std::size_t professor,
                            std::size_t hours,
                            std::set<WeekDay> days,
                            std::set<std::size_t> groups,
                            std::set<std::size_t> classrooms)
        : professor_(professor)
        , hours_(hours)
        , days_(std::move(days))
        , groups_(std::move(groups))
        , classrooms_(std::move(classrooms))
    {}

    bool RequestedGroup(std::size_t g) const
    {
        return groups_.count(g) > 0;
    }

    bool Requested(WeekDay d) const
    {
        return days_.count(d) > 0;
    }

    std::size_t HoursPerWeek() const
    {
        return hours_;
    }

    std::size_t Professor() const
    {
        return professor_;
    }

private:
    std::size_t professor_;
    std::size_t hours_;
    std::set<WeekDay> days_;
    std::set<std::size_t> groups_;
    std::set<std::size_t> classrooms_;
};


class ScheduleData
{
public:
    explicit ScheduleData(std::size_t countLessonsPerDay,
                          std::size_t countGroups,
                          std::size_t countProfessors,
                          std::size_t countClassrooms,
                          std::vector<SubjectRequest> subjectRequests)
      : countLessonsPerDay_(countLessonsPerDay)
      , countGroups_(countGroups)
      , countProfessors_(countProfessors)
      , countClassrooms_(countClassrooms)
      , subjectRequests_(std::move(subjectRequests))
    {
    }

    std::size_t MaxCountLessonsPerDay() const { return 6; }
    std::size_t RequestedCountLessonsPerDay() const { return countLessonsPerDay_; }
    std::size_t CountGroups() const { return countGroups_; }
    std::size_t CountSubjects() const { return subjectRequests_.size(); }
    std::size_t CountProfessors() const { return countProfessors_; }
    std::size_t CountClassrooms() const { return countClassrooms_; }

    const std::vector<SubjectRequest>& SubjectRequests() const { return subjectRequests_; }

private:
    std::size_t countLessonsPerDay_ = 0;
    std::size_t countGroups_ = 0;
    std::size_t countProfessors_ = 0;
    std::size_t countClassrooms_ = 0;
    std::vector<SubjectRequest> subjectRequests_;
};

void Print(const ScheduleData& data);

std::size_t CalculateHours(const ScheduleData& data,
                           std::size_t professor,
                           std::size_t group,
                           std::size_t subject);

bool SubjectBelongsToProfessor(const ScheduleData& data, std::size_t subject, std::size_t professor);
bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, WeekDay day);
