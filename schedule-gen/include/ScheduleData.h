#pragma once
#include <string>
#include <cstdint>
#include <map>
#include <set>
#include <vector>


extern const std::size_t SCHEDULE_DAYS_COUNT;

enum class WeekDay : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);


class SubjectRequest
{
public:
    explicit SubjectRequest(std::size_t hours,
                            std::set<WeekDay> days,
                            std::set<std::size_t> classrooms)
        : hours_(hours)
        , days_(std::move(days))
        , classrooms_(std::move(classrooms))
    {}

    bool Requested(WeekDay d) const
    {
        return days_.count(d) > 0;
    }

    std::size_t HoursPerWeek() const
    {
        return hours_;
    }

private:
    std::size_t hours_;
    std::set<WeekDay> days_;
    std::set<std::size_t> classrooms_;
};


class ScheduleData
{
public:
    explicit ScheduleData(std::size_t countLessonsPerDay,
                          std::size_t countGroups,
                          std::size_t countSubjects,
                          std::size_t countClassrooms,
                          std::vector<SubjectRequest> professorRequests)
      : countLessonsPerDay_(countLessonsPerDay)
      , countGroups_(countGroups)
      , countSubjects_(countSubjects)
      , countClassrooms_(countClassrooms)
      , professorRequests_(std::move(professorRequests))
    {
    }

    std::size_t MaxCountLessonsPerDay() const { return 6; }
    std::size_t RequestedCountLessonsPerDay() const { return countLessonsPerDay_; }
    std::size_t CountGroups() const { return countGroups_; }
    std::size_t CountSubjects() const { return countSubjects_; }
    std::size_t CountProfessors() const { return professorRequests_.size(); }
    std::size_t CountClassrooms() const { return countClassrooms_; }

private:
    std::size_t countLessonsPerDay_ = 0;
    std::size_t countGroups_ = 0;
    std::size_t countSubjects_ = 0;
    std::size_t countClassrooms_ = 0;
    std::vector<SubjectRequest> professorRequests_;
};
