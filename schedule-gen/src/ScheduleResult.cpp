#include "ScheduleResult.hpp"
#include "ScheduleData.hpp"

#include <iostream>
#include <algorithm>


ScheduleItem::ScheduleItem(const LessonAddress& address,
                           std::size_t subject,
                           std::size_t professor,
                           std::size_t classroom)
        : Address(address)
        , Subject(subject)
        , Professor(professor)
        , Classroom(classroom)
{}


bool ScheduleResult::empty() const { return items_.empty(); }

const std::vector<ScheduleItem>& ScheduleResult::items() const { return items_; }

const ScheduleItem* ScheduleResult::at(const LessonAddress& address) const
{
    auto it = std::lower_bound(items_.begin(), items_.end(), address, ScheduleItemLess());
    if(it == items_.end() || it->Address != address)
        return nullptr;

    return &(*it);
}

std::vector<ScheduleItem>::iterator ScheduleResult::insert(const ScheduleItem& item)
{
    auto it = std::lower_bound(items_.begin(), items_.end(), item.Address, ScheduleItemLess());
    if(it == items_.end() || it->Address != item.Address)
        it = items_.insert(it, item);

    return it;
}


void Print(const OverlappedClassroom& overlappedClassroom)
{
    std::cout << "Overlapped classroom: " << overlappedClassroom.Classroom << " {\n";
    for(auto lesson : overlappedClassroom.Lessons)
    {
        std::cout << "    (g: " << lesson.Group << ", l: " << lesson.Lesson << ")\n";
    }

    std::cout << "}\n" << std::endl;
}

std::vector<OverlappedClassroom> FindOverlappedClassrooms(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedClassroom> overlappedClassrooms;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        SortedMap<std::size_t, SortedSet<SubjectWithAddress>> classroomsAndSubjects;
        for(std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            const auto item = result.at(LessonAddress(g, l));
            if(item)
                classroomsAndSubjects[item->Classroom].insert(SubjectWithAddress(item->Subject, LessonAddress(g, l)));
        }

        for(const auto&[classroom, subjects] : classroomsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                SortedSet<LessonAddress> lessons;
                for(auto&& subject : subjects)
                    lessons.insert(subject.Address);

                overlappedClassrooms.emplace_back(classroom, std::move(lessons));
            }
        }
    }

    return overlappedClassrooms;
}

std::vector<OverlappedProfessor> FindOverlappedProfessors(const ScheduleData& data, const ScheduleResult& result)
{
    std::vector<OverlappedProfessor> overlappedProfessors;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        SortedMap<std::size_t, SortedSet<SubjectWithAddress>> professorsAndSubjects;
        for(std::size_t g = 0; g < data.CountGroups(); ++g)
        {
            const auto item = result.at(LessonAddress(g, l));
            if(item)
                professorsAndSubjects[item->Professor].insert(SubjectWithAddress(item->Subject, LessonAddress(g, l)));
        }

        for(const auto&[professor, subjects] : professorsAndSubjects)
        {
            if(subjects.size() > 1)
            {
                SortedSet<LessonAddress> lessons;
                for(auto&& subject : subjects)
                    lessons.insert(subject.Address);

                overlappedProfessors.emplace_back(professor, std::move(lessons));
            }
        }
    }

    return overlappedProfessors;
}

std::vector<ViolatedSubjectRequest> FindViolatedSubjectRequests(const ScheduleData& data,
                                                                const ScheduleResult& result)
{
    std::vector<ViolatedSubjectRequest> violatedRequests;
    for(std::size_t g = 0; g < data.CountGroups(); ++g)
    {
        for (std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
        {
            const std::size_t day = LessonToScheduleDay(l);
            const auto item = result.at(LessonAddress(g, l));
            if(item && (!WeekDayRequestedForSubject(data, item->Subject, day) || data.LessonIsOccupied(LessonAddress(g, l))))
            {
                auto it = std::lower_bound(violatedRequests.begin(), violatedRequests.end(),
                                           item->Subject, ViolatedSubjectRequestLess());

                if(it == violatedRequests.end() || it->Subject != item->Subject)
                    it = violatedRequests.emplace(it, item->Subject);

                it->Lessons.insert(LessonAddress(g, l));
            }
        }
    }

    return violatedRequests;
}
