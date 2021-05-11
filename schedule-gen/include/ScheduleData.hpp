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
                            std::size_t complexity,
                            WeekDays days,
                            SortedSet<std::size_t> groups,
                            SortedSet<std::size_t> classrooms);

    [[nodiscard]] bool RequestedClassroom(std::size_t c) const;
    [[nodiscard]] bool RequestedGroup(std::size_t g) const;
    [[nodiscard]] bool Requested(WeekDay d) const;
    [[nodiscard]] std::size_t HoursPerWeek() const;
    [[nodiscard]] std::size_t Complexity() const;
    [[nodiscard]] std::size_t Professor() const;
    [[nodiscard]] const std::vector<std::size_t>& Groups() const;
    [[nodiscard]] const std::vector<std::size_t>& Classrooms() const;
    [[nodiscard]] bool RequestedWeekDay(std::size_t day) const;

private:
    std::size_t professor_;
    std::size_t hours_;
    std::size_t complexity_;
    WeekDays days_;
    SortedSet<std::size_t> groups_;
    SortedSet<std::size_t> classrooms_;
};


class ScheduleData
{
public:
    explicit ScheduleData(std::size_t countLessonsPerDay,
                          std::size_t countGroups,
                          std::size_t countProfessors,
                          std::size_t countClassrooms,
                          std::vector<SubjectRequest> subjectRequests,
                          std::vector<LessonAddress> occupiedLessons);

    [[nodiscard]] std::size_t MaxCountLessonsPerDay() const;
    [[nodiscard]] std::size_t RequestedCountLessonsPerDay() const;
    [[nodiscard]] std::size_t CountGroups() const;
    [[nodiscard]] std::size_t CountSubjects() const;
    [[nodiscard]] std::size_t CountProfessors() const;
    [[nodiscard]] std::size_t CountClassrooms() const;
    [[nodiscard]] const std::vector<SubjectRequest>& SubjectRequests() const;
    [[nodiscard]] bool LessonIsOccupied(const LessonAddress& lessonAddress) const;

private:
    std::size_t countLessonsPerDay_;
    std::size_t countGroups_;
    std::size_t countProfessors_;
    std::size_t countClassrooms_;
    std::vector<SubjectRequest> subjectRequests_;
    std::vector<LessonAddress> occupiedLessons_;
};


enum class ScheduleDataValidationResult
{
    Ok,
    ToFewLessonsPerDayRequested,
    NoGroups,
    NoSubjects,
    NoProfessors,
    NoClassrooms
};

void Print(const ScheduleData& data);
[[nodiscard]] ScheduleDataValidationResult Validate(const ScheduleData& data);
[[nodiscard]] std::size_t CalculateHours(const ScheduleData& data, std::size_t professor, std::size_t group, std::size_t subject);
[[nodiscard]] bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t scheduleDay);
[[nodiscard]] bool ClassroomRequestedForSubject(const ScheduleData& data, std::size_t subject, std::size_t classroom);
