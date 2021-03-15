#include "ScheduleData.hpp"
#include <algorithm>


std::size_t CountHoursPerSubject(const ScheduleData& data, std::size_t subject)
{
    return data.SubjectRequests().at(subject).HoursPerWeek() * 2;
}

bool SubjectBelongsToProfessor(const ScheduleData& data, std::size_t subject, std::size_t professor)
{
    return data.SubjectRequests().at(subject).Professor() == professor;
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, WeekDay day)
{
    return data.SubjectRequests().at(subject).Requested(day);
}
