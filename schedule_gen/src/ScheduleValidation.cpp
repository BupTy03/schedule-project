#include "ScheduleValidation.h"


std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data,
                                                          const ScheduleResult& result)
{
    std::vector<OverlappedClassroom> overlappedClassrooms;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::map<std::size_t, std::vector<std::size_t>> classroomsAndSubjects;
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
        {
            if(item.Classroom == ClassroomAddress::Any().Classroom)
                continue;

            classroomsAndSubjects[item.Classroom].emplace_back(item.SubjectRequestID);
        }

        for(auto& [classroom, subjects] : classroomsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                std::sort(subjects.begin(), subjects.end());

                OverlappedClassroom overlappedClassroom;
                overlappedClassroom.Address = l;
                overlappedClassroom.Classroom = classroom;
                overlappedClassroom.SubjectRequestsIDs = std::move(subjects);
                overlappedClassrooms.emplace_back(std::move(overlappedClassroom));
            }
        }
    }

    return overlappedClassrooms;
}

std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data,
                                                          const ScheduleResult& result)
{
    std::vector<OverlappedProfessor> overlappedProfessors;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::map<std::size_t, std::vector<std::size_t>> professorsAndSubjects;
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
        {
            const auto& request = data.SubjectRequestAtID(item.SubjectRequestID);
            professorsAndSubjects[request.Professor()].emplace_back(item.SubjectRequestID);
        }

        for(auto& [professor, subjects] : professorsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                std::sort(subjects.begin(), subjects.end());

                OverlappedProfessor overlappedProfessor;
                overlappedProfessor.Address = l;
                overlappedProfessor.Professor = professor;
                overlappedProfessor.SubjectRequestsIDs = std::move(subjects);
                overlappedProfessors.emplace_back(std::move(overlappedProfessor));
            }
        }
    }

    return overlappedProfessors;
}

std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                   const ScheduleResult& result)
{
    std::vector<OverlappedGroups> overlapped;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        const auto lessonsRange = result.at(l);
        std::map<std::pair<std::size_t, std::size_t>, std::vector<std::size_t>>
            subjectGroupsIntersections;
        for(auto f = lessonsRange.begin(); f != lessonsRange.end(); ++f)
        {
            for(auto s = lessonsRange.begin(); s != lessonsRange.end(); ++s)
            {
                if(f == s)
                    continue;

                if(subjectGroupsIntersections.count({s->SubjectRequestID, f->SubjectRequestID}) > 0)
                    continue;

                const auto& firstGroups = data.SubjectRequestAtID(f->SubjectRequestID).Groups();
                const auto& secondGroups = data.SubjectRequestAtID(s->SubjectRequestID).Groups();

                std::vector<std::size_t> intersectedGroups;
                std::set_intersection(firstGroups.begin(),
                                      firstGroups.end(),
                                      secondGroups.begin(),
                                      secondGroups.end(),
                                      std::back_inserter(intersectedGroups));
                if(intersectedGroups.empty())
                    continue;

                subjectGroupsIntersections.emplace(
                    std::pair{f->SubjectRequestID, s->SubjectRequestID},
                    std::move(intersectedGroups));
            }
        }

        for(auto& [subjPair, groups] : subjectGroupsIntersections)
        {
            OverlappedGroups overlappedGroups;
            overlappedGroups.Address = l;
            overlappedGroups.Groups = std::move(groups);

            // sorted
            overlappedGroups.SubjectRequestsIDs = {std::min(subjPair.first, subjPair.second),
                                                   std::max(subjPair.first, subjPair.second)};
            overlapped.emplace_back(std::move(overlappedGroups));
        }
    }

    return overlapped;
}

CheckScheduleResult CheckSchedule(const ScheduleData& data, const ScheduleResult& result)
{
    return CheckScheduleResult{.OverlappedClassroomsList = FindOverlappedClassrooms(data, result),
                               .OverlappedProfessorsList = FindOverlappedProfessors(data, result),
                               .OverlappedGroupsList = FindOverlappedGroups(data, result)};
}
