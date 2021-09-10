#include "ScheduleData.h"
#include <string>
#include <algorithm>


SubjectRequest::SubjectRequest(std::size_t id,
                            std::size_t professor,
                            std::size_t complexity,
                            std::vector<std::size_t> groups,
                            std::vector<std::size_t> lessons,
                            std::vector<ClassroomAddress> classrooms)
        : id_(id)
        , professor_(professor)
        , complexity_(complexity)
        , groups_(std::move(groups))
        , lessons_(std::move(lessons))
        , classrooms_(std::move(classrooms))
{
    std::ranges::sort(groups_);
    groups_.erase(std::unique(groups_.begin(), groups_.end()), groups_.end());

    std::ranges::sort(lessons_);
    lessons_.erase(std::unique(lessons_.begin(), lessons_.end()), lessons_.end());

    if(lessons_.empty())
        lessons_ = AllLessons();

    std::ranges::sort(classrooms_);
    classrooms_.erase(std::unique(classrooms_.begin(), classrooms_.end()), classrooms_.end());
}


ScheduleData::ScheduleData(std::vector<SubjectRequest> subjectRequests)
    : subjectRequests_(std::move(subjectRequests))
    , professorRequests_()
    , groupRequests_()
{
    std::ranges::sort(subjectRequests_, {}, &SubjectRequest::ID);
    subjectRequests_.erase(std::unique(subjectRequests_.begin(), subjectRequests_.end(),
                                       SubjectRequestIDEqual()), subjectRequests_.end());

    for(std::size_t r = 0; r < subjectRequests_.size(); ++r)
    {
        const auto& request = subjectRequests_.at(r);
        professorRequests_[request.Professor()].insert(r);
        for(std::size_t g : request.Groups())
            groupRequests_[g].insert(r);
    }
}

const SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return *it;
}

SubjectRequest& ScheduleData::SubjectRequestAtID(std::size_t subjectRequestID)
{
    return const_cast<SubjectRequest&>(const_cast<const ScheduleData*>(this)->SubjectRequestAtID(subjectRequestID));
}

std::size_t ScheduleData::IndexOfSubjectRequestWithID(std::size_t subjectRequestID) const
{
    auto it = std::ranges::lower_bound(subjectRequests_, subjectRequestID, {}, &SubjectRequest::ID);
    if(it == subjectRequests_.end() || it->ID() != subjectRequestID)
        throw std::out_of_range("Subject request with ID=" + std::to_string(subjectRequestID) + " is not found!");

    return std::distance(subjectRequests_.begin(), it);
}


std::vector<std::size_t> ConvertToRequestedLessons(const WeekDays& days, ClassesType classesType)
{
    std::vector<std::size_t> result;
    for(std::size_t lesson = 0; lesson < MAX_LESSONS_COUNT; ++lesson)
    {
        const WeekDay wd = LessonToWeekDay(lesson);
        if(!days.contains(wd))
            continue;

        if(classesType == ClassesType::Morning)
        {
            if(IsLateScheduleLessonInSaturday(lesson))
                continue;
        }
        else if(classesType == ClassesType::Evening)
        {
            if(!SuitableForEveningClasses(lesson))
                continue;
        }

        result.emplace_back(lesson);
    }

    return result;
}
