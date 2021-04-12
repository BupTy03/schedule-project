#include "ScheduleResult.hpp"
#include "ScheduleData.hpp"

#include <map>
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

std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedClassroom> overlappedClassrooms;
    for(std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
    {
        for(std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
        {
            std::map<std::size_t, SortedSet<SubjectWithAddress>> classroomsAndSubjects;
            for(std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                const auto item = result.At(g, d, l);
                if(item)
                    classroomsAndSubjects[item->Classroom].Add(SubjectWithAddress(item->Subject, LessonAddress(g, d, l)));
            }

            for(const auto&[classroom, subjects] : classroomsAndSubjects)
            {
                if(subjects.size() > 1)
                {
                    SortedSet<LessonAddress> lessons;
                    for(auto&& subject : subjects)
                        lessons.Add(subject.Address);

                    overlappedClassrooms.emplace_back(classroom, std::move(lessons));
                }
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


std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedProfessor> overlappedProfessors;
    for(std::size_t d = 0; d < SCHEDULE_DAYS_COUNT; ++d)
    {
        for(std::size_t l = 0; l < data.MaxCountLessonsPerDay(); ++l)
        {
            std::map<std::size_t, SortedSet<SubjectWithAddress>> professorsAndSubjects;
            for(std::size_t g = 0; g < data.CountGroups(); ++g)
            {
                const auto item = result.At(g, d, l);
                if(item)
                    professorsAndSubjects[item->Professor].Add(SubjectWithAddress(item->Subject, LessonAddress(g, d, l)));
            }

            for(const auto&[classroom, subjects] : professorsAndSubjects)
            {
                if(subjects.size() > 1)
                {
                    SortedSet<LessonAddress> lessons;
                    for(auto&& subject : subjects)
                        lessons.Add(subject.Address);

                    overlappedProfessors.emplace_back(classroom, std::move(lessons));
                }
            }
        }
    }

    return overlappedProfessors;
}
