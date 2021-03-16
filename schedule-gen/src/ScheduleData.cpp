#include "ScheduleData.hpp"
#include <algorithm>


bool SubjectBelongsToProfessor(const ScheduleData& data, std::size_t subject, std::size_t professor)
{
    return data.SubjectRequests().at(subject).Professor() == professor;
}

bool WeekDayRequestedForSubject(const ScheduleData& data, std::size_t subject, WeekDay day)
{
    return data.SubjectRequests().at(subject).Requested(day);
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
