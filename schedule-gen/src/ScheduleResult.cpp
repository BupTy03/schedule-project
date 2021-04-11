#include "ScheduleResult.hpp"
#include "ScheduleData.hpp"

#include <iostream>
#include <algorithm>


ScheduleItem::ScheduleItem(std::size_t subject, std::size_t professor, std::size_t classroom)
        : Subject(subject)
        , Professor(professor)
        , Classroom(classroom)
{}

ScheduleResult::ScheduleResult(std::vector<Group> groups)
        : groups_(std::move(groups))
{
}

bool ScheduleResult::Empty() const { return groups_.empty(); }

ScheduleResult::Lesson ScheduleResult::At(std::size_t group,
                                          std::size_t day,
                                          std::size_t lesson) const
{
    return groups_.at(group).at(day).at(lesson);
}


ScheduleValidationResult::ScheduleValidationResult(std::vector<OverlappedClassroom> overlappedClassrooms,
                                                   std::vector<OverlappedProfessor> overlappedProfessors,
                                                   std::vector<ViolatedSubjectRequest> violatedRequests,
                                                   std::vector<ExceedingComplexityItem> complexity)
    : overlappedClassrooms_(std::move(overlappedClassrooms))
    , overlappedProfessors_(std::move(overlappedProfessors))
    , violatedRequests_(std::move(violatedRequests))
    , complexity_(std::move(complexity))
{
}

const std::vector<OverlappedClassroom>& ScheduleValidationResult::OverlappedClassrooms() const
{
    return overlappedClassrooms_;
}

const std::vector<OverlappedProfessor>& ScheduleValidationResult::OverlappedProfessors() const
{
    return overlappedProfessors_;
}

const std::vector<ViolatedSubjectRequest>& ScheduleValidationResult::ViolatedRequests() const
{
    return violatedRequests_;
}

const std::vector<ExceedingComplexityItem>& ScheduleValidationResult::TotalComplexity() const
{
    return complexity_;
}


std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedClassroom> overlappedClassrooms;
    for(std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
    {
        for(std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
        {
            std::vector<std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>> classroomsAndSubjects;
            for(std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                const auto item = result.At(g, d, l);
                if(item)
                    classroomsAndSubjects.emplace_back(item->Classroom, item->Subject, g, d, l);
            }
            std::sort(classroomsAndSubjects.begin(), classroomsAndSubjects.end());

            auto equalComp = [](const std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>& lhs,
                                const std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>& rhs){
                return std::get<0>(lhs) == std::get<0>(rhs);
            };

            auto afIt = std::adjacent_find(classroomsAndSubjects.begin(), classroomsAndSubjects.end(), equalComp);
            while(afIt != classroomsAndSubjects.end())
            {
                OverlappedClassroom overlappedClassroom;
                overlappedClassroom.Classroom = std::get<0>(*afIt);
                overlappedClassroom.Lessons.Add(LessonAddress{std::get<2>(*afIt), std::get<3>(*afIt), std::get<4>(*afIt)});
                auto it = std::next(afIt);
                for(; it != classroomsAndSubjects.end() && std::get<0>(*it) == std::get<0>(*afIt); ++it, ++afIt)
                    overlappedClassroom.Lessons.Add(LessonAddress{std::get<2>(*it), std::get<3>(*it), std::get<4>(*it)});

                overlappedClassrooms.emplace_back(std::move(overlappedClassroom));
                afIt = std::adjacent_find(it, classroomsAndSubjects.end(), equalComp);
            }
        }
    }

    return overlappedClassrooms;
}

void Print(const OverlappedClassroom& overlappedClassroom)
{
    std::cout << "Overlapped classroom: " << overlappedClassroom.Classroom << " {\n";
    for(auto lesson : overlappedClassroom.Lessons)
    {
        std::cout << "    (g: " << lesson.Group << ", d: " << lesson.Day << ", l: " << lesson.Lesson << ")\n";
    }

    std::cout << "}\n" << std::endl;
}
