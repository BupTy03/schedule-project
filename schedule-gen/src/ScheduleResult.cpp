#include "ScheduleResult.hpp"
#include "ScheduleData.hpp"

#include <iostream>
#include <algorithm>


ScheduleItem::ScheduleItem(std::size_t lessonAddress,
                           std::size_t subjectRequest,
                           std::size_t subjectRequestID,
                           std::size_t classroom)
        : Address(lessonAddress)
        , SubjectRequest(subjectRequest)
        , SubjectRequestID(subjectRequestID)
        , Classroom(classroom)
{}


bool ScheduleResult::empty() const { return items_.empty(); }

const std::vector<ScheduleItem>& ScheduleResult::items() const { return items_; }

Range<std::vector<ScheduleItem>::const_iterator> ScheduleResult::at(std::size_t lessonAddress) const
{
    auto itPair = std::equal_range(items_.begin(), items_.end(), lessonAddress, ScheduleItemLess());
    return make_range(itPair.first, itPair.second);
}

std::vector<ScheduleItem>::iterator ScheduleResult::insert(const ScheduleItem& item)
{
    auto it = std::lower_bound(items_.begin(), items_.end(), item.Address, ScheduleItemLess());
    return items_.insert(it, item);
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
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
        {
            for (std::size_t g : data.SubjectRequests().at(item.SubjectRequest).Groups())
                classroomsAndSubjects[item.Classroom].insert(SubjectWithAddress(item.SubjectRequest, LessonAddress(g, l)));
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
        const auto lessonsRange = result.at(l);
        for(auto&& item : lessonsRange)
        {
            const auto& request = data.SubjectRequests().at(item.SubjectRequest);
            for(std::size_t g : data.SubjectRequests().at(item.SubjectRequest).Groups())
                professorsAndSubjects[request.Professor()].insert(SubjectWithAddress(item.SubjectRequest, LessonAddress(g, l)));
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

std::vector<OverlappedGroups> FindOverlappedGroups(const ScheduleData& data,
                                                   const ScheduleResult& result)
{
    std::vector<OverlappedGroups> overlapped;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        const auto lessonsRange = result.at(l);
        for(auto f = lessonsRange.begin(); f != lessonsRange.end(); ++f)
        {
            for(auto s = lessonsRange.begin(); s != lessonsRange.end(); ++s)
            {
                if(f == s)
                    continue;

                OverlappedGroups ovGroups;
                ovGroups.FirstSubjectID = f->SubjectRequestID;
                ovGroups.SecondSubjectID = s->SubjectRequestID;
                const auto& firstGroups = data.SubjectRequests().at(f->SubjectRequest).Groups();
                const auto& secondGroups = data.SubjectRequests().at(s->SubjectRequest).Groups();
                std::set_intersection(firstGroups.begin(), firstGroups.end(), secondGroups.begin(), secondGroups.end(), std::back_inserter(ovGroups.Groups));
                overlapped.emplace_back(std::move(ovGroups));
            }
        }
    }

    overlapped.erase(std::remove_if(overlapped.begin(), overlapped.end(), [](const OverlappedGroups& og){
        return og.Groups.empty();
    }), overlapped.end());

    return overlapped;
}

std::vector<ViolatedSubjectRequest> FindViolatedSubjectRequests(const ScheduleData& data,
                                                                const ScheduleResult& result)
{
    std::vector<ViolatedSubjectRequest> violatedRequests;
    for(auto&& item : result.items())
    {
        const std::size_t day = LessonToScheduleDay(item.Address);

        const auto& request = data.SubjectRequests().at(item.SubjectRequest);
        for(std::size_t g : request.Groups())
        {
            if(!WeekDayRequestedForSubject(data, item.SubjectRequest, day) || data.LessonIsOccupied(LessonAddress(g, item.Address)))
            {
                auto it = std::lower_bound(violatedRequests.begin(), violatedRequests.end(), item.SubjectRequest, ViolatedSubjectRequestLess());

                if (it == violatedRequests.end() || it->Subject != item.SubjectRequest)
                    it = violatedRequests.emplace(it, item.SubjectRequest);

                it->Lessons.insert(LessonAddress(g, item.Address));
            }
        }
    }

    return violatedRequests;
}
