#include "ScheduleChromosomes.h"
#include "ScheduleData.h"
#include <array>
#include <utility>
#include <numeric>
#include <algorithm>
#include <experimental/generator>


ScheduleChromosomes::ScheduleChromosomes(std::size_t count)
    : lessons_(count, NO_LESSON)
    , classrooms_(count, ClassroomAddress::NoClassroom())
{
}

ScheduleChromosomes::ScheduleChromosomes(std::vector<std::size_t> lessons,
                                         std::vector<ClassroomAddress> classrooms)
    : lessons_(std::move(lessons))
    , classrooms_(std::move(classrooms))
{
    assert(lessons_.size() == classrooms_.size());
}

bool ScheduleChromosomes::GroupsOrProfessorsOrClassroomsIntersects(const ScheduleData& data,
                                                                   std::size_t currentRequest,
                                                                   std::size_t currentLesson) const
{
    if(classrooms_.at(currentRequest) == ClassroomAddress::Any())
        return GroupsOrProfessorsIntersects(data, currentRequest, currentLesson);

    const auto& requests = data.SubjectRequests();
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons_.begin(), lessons_.end(), currentLesson);
    while(it != lessons_.end())
    {
        const std::size_t requestIndex = std::distance(lessons_.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() ||
            classrooms_.at(currentRequest) == classrooms_.at(requestIndex) ||
           set_intersects(thisRequest.Groups(), otherRequest.Groups()))
        {
            return true;
        }

        it = std::find(std::next(it), lessons_.end(), currentLesson);
    }

    return false;
}

bool ScheduleChromosomes::GroupsOrProfessorsIntersects(const ScheduleData& data,
                                                       std::size_t currentRequest,
                                                       std::size_t currentLesson) const
{
    const auto& requests = data.SubjectRequests();
    const auto& thisRequest = requests.at(currentRequest);
    auto it = std::find(lessons_.begin(), lessons_.end(), currentLesson);
    while(it != lessons_.end())
    {
        const std::size_t requestIndex = std::distance(lessons_.begin(), it);
        const auto& otherRequest = requests.at(requestIndex);
        if(thisRequest.Professor() == otherRequest.Professor() || set_intersects(thisRequest.Groups(), otherRequest.Groups()))
            return true;

        it = std::find(std::next(it), lessons_.end(), currentLesson);
    }

    return false;
}

bool ScheduleChromosomes::ClassroomsIntersects(std::size_t currentLesson,
                                               const ClassroomAddress& currentClassroom) const
{
    if(currentClassroom == ClassroomAddress::Any())
        return false;

    auto it = std::find(classrooms_.begin(), classrooms_.end(), currentClassroom);
    while(it != classrooms_.end())
    {
        const std::size_t requestIndex = std::distance(classrooms_.begin(), it);
        const std::size_t otherLesson = lessons_.at(requestIndex);
        if(currentLesson == otherLesson)
            return true;

        it = std::find(std::next(it), classrooms_.end(), currentClassroom);
    }

    return false;
}

std::size_t ScheduleChromosomes::UnassignedLessonsCount() const
{
    return std::ranges::count(lessons_, NO_LESSON);
}

std::size_t ScheduleChromosomes::UnassignedClassroomsCount() const
{
    return std::ranges::count(classrooms_, ClassroomAddress::NoClassroom());
}


void InsertRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex)
{
    const auto& requests = data.SubjectRequests();
    const auto& request = requests.at(requestIndex);
    const auto& lessons = request.Lessons();
    const auto& classrooms = request.Classrooms();

    assert(!lessons.empty());
    for(auto&& lesson : lessons)
    {
        if(chromosomes.GroupsOrProfessorsIntersects(data, requestIndex, lesson))
            continue;

        chromosomes.Lesson(requestIndex) = lesson;
        if(classrooms.empty())
        {
            chromosomes.Classroom(requestIndex) = ClassroomAddress::Any();
            return;
        }

        for(auto&& classroom : classrooms)
        {
            if(!chromosomes.ClassroomsIntersects(lesson, classroom))
            {
                chromosomes.Classroom(requestIndex) = classroom;
                return;
            }
        }
    }
}

ScheduleChromosomes InitializeChromosomes(const ScheduleData& data)
{
    const auto& requests = data.SubjectRequests();
    assert(!requests.empty());

    std::vector<std::size_t> requestsIndexes(requests.size());
    std::iota(requestsIndexes.begin(), requestsIndexes.end(), 0);
    std::sort(requestsIndexes.begin(), requestsIndexes.end(), [&](std::size_t lhs, std::size_t rhs) { 
        return std::size(requests.at(lhs).Lessons()) < std::size(requests.at(rhs).Lessons()); 
    });

    ScheduleChromosomes result(requests.size());
    for(std::size_t r : requestsIndexes)
        InsertRequest(result, data, r);

    return result;
}

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r)
{
    const auto firstLesson = first.Lesson(r);
    const auto firstClassroom = first.Classroom(r);

    const auto secondLesson = second.Lesson(r);
    const auto secondClassroom = second.Classroom(r);

    if(first.ClassroomsIntersects(secondLesson, secondClassroom) ||
       second.ClassroomsIntersects(firstLesson, firstClassroom))
        return false;

    if(first.GroupsOrProfessorsOrClassroomsIntersects(data, r, secondLesson) ||
       second.GroupsOrProfessorsOrClassroomsIntersects(data, r, firstLesson))
        return false;

    return true;
}

void Crossover(ScheduleChromosomes& first,
               ScheduleChromosomes& second,
               std::size_t r)
{
    using std::swap;
    swap(first.Lesson(r), second.Lesson(r));
    swap(first.Classroom(r), second.Classroom(r));
}

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes,
                     const ScheduleData& scheduleData)
{
    std::size_t maxDayComplexity = 0;
    std::size_t maxLessonsGapsForGroups = 0;
    std::size_t maxLessonsGapsForProfessors = 0;
    std::size_t maxBuildingsChangesForGroups = 0;
    std::size_t maxBuildingsChangesForProfessors = 0;

    auto calculateGap = [](auto&& lhs, auto&& rhs) {
        return lhs.first - rhs.first - 1;
    };

    auto buildingsChanged = [&](auto&& lhs, auto&& rhs) -> bool {
        const auto lhsLesson = lhs.first;
        const auto rhsLesson = rhs.first;
        const auto lhsBuilding = scheduleChromosomes.Classroom(lhs.second).Building;
        const auto rhsBuilding = scheduleChromosomes.Classroom(rhs.second).Building;

        return !(lhsBuilding == rhsBuilding ||
                lhsLesson - rhsLesson > 1  ||
                lhsBuilding == NO_BUILDING ||
                rhsBuilding == NO_BUILDING);
    };

    const auto& requests = scheduleData.SubjectRequests();
    std::vector<std::pair<std::size_t, std::size_t>> lessonsBuffer;
    for(auto&&[professor, professorRequests] : scheduleData.Professors())
    {
        lessonsBuffer.clear();
        lessonsBuffer.reserve(professorRequests.size());
        std::ranges::transform(professorRequests, std::back_inserter(lessonsBuffer),
                               [&](std::size_t r) { return std::pair{scheduleChromosomes.Lesson(r), r}; });

        std::ranges::sort(lessonsBuffer);
        auto lastLesson = std::ranges::lower_bound(lessonsBuffer, NO_LESSON,
                                                   std::less<>{}, [](auto&& p) { return p.first; });

        for(auto firstDayLessonIt = lessonsBuffer.begin(); firstDayLessonIt != lastLesson;)
        {
            const std::size_t day = firstDayLessonIt->first / MAX_LESSONS_PER_DAY;
            const std::size_t nextDayFirstLesson = (day + 1) * MAX_LESSONS_PER_DAY;
            const auto lastDayLessonIt = std::ranges::lower_bound(std::next(firstDayLessonIt), lastLesson, nextDayFirstLesson,
                                                                  std::less<>{}, [](auto&& p) { return p.first; });

            maxLessonsGapsForProfessors = std::max(maxLessonsGapsForProfessors,
                                                   std::inner_product(std::next(firstDayLessonIt), lastDayLessonIt,
                                                                      firstDayLessonIt, std::size_t{0}, std::plus<>{}, calculateGap));

            maxBuildingsChangesForProfessors = std::max(maxBuildingsChangesForProfessors,
                                                        std::inner_product(std::next(firstDayLessonIt), lastDayLessonIt,
                                                                           firstDayLessonIt, std::size_t{0}, std::plus<>{}, buildingsChanged));

            firstDayLessonIt = lastDayLessonIt;
        }
    }

    for(auto&&[group, groupRequests] : scheduleData.Groups())
    {
        lessonsBuffer.clear();
        lessonsBuffer.reserve(groupRequests.size());
        std::ranges::transform(groupRequests, std::back_inserter(lessonsBuffer),
                               [&](std::size_t r) { return std::pair{scheduleChromosomes.Lesson(r), r}; });

        std::ranges::sort(lessonsBuffer);
        auto lastLesson = std::ranges::lower_bound(lessonsBuffer, NO_LESSON,
                                                   std::less<>{}, [](auto&& p) { return p.first; });

        for(auto firstDayLessonIt = lessonsBuffer.begin(); firstDayLessonIt != lastLesson;)
        {
            const std::size_t day = firstDayLessonIt->first / MAX_LESSONS_PER_DAY;
            const std::size_t nextDayFirstLesson = (day + 1) * MAX_LESSONS_PER_DAY;
            const auto lastDayLessonIt = std::ranges::lower_bound(std::next(firstDayLessonIt), lastLesson, nextDayFirstLesson,
                                                                  std::less<>{}, [](auto&& p) { return p.first; });

            maxLessonsGapsForGroups = std::max(maxLessonsGapsForGroups,
                                               std::inner_product(std::next(firstDayLessonIt), lastDayLessonIt,
                                                                  firstDayLessonIt, std::size_t{0}, std::plus<>{}, calculateGap));

            maxBuildingsChangesForGroups = std::max(maxBuildingsChangesForGroups,
                                                    std::inner_product(std::next(firstDayLessonIt), lastDayLessonIt,
                                                                       firstDayLessonIt, std::size_t{0}, std::plus<>{}, buildingsChanged));

            maxDayComplexity = std::max(maxDayComplexity,
                                        std::accumulate(firstDayLessonIt, lastDayLessonIt, std::size_t{0}, [&](auto accum, auto&& p){
                                               return accum + (p.first % MAX_LESSONS_PER_DAY) * requests.at(p.second).Complexity();
                                        }));

            firstDayLessonIt = lastDayLessonIt;
        }
    }

    return maxLessonsGapsForGroups * 3 +
           maxLessonsGapsForProfessors * 2 +
           maxDayComplexity * 4 +
           maxBuildingsChangesForProfessors * 64 +
           maxBuildingsChangesForGroups * 64 +
           scheduleChromosomes.UnassignedLessonsCount() * 128 +
           scheduleChromosomes.UnassignedClassroomsCount() * 128;
}

ScheduleResult MakeScheduleResult(const ScheduleChromosomes& chromosomes,
                                  const ScheduleData& scheduleData)
{
    const auto& lessons = chromosomes.Lessons();
    const auto& classrooms = chromosomes.Classrooms();

    ScheduleResult resultSchedule;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        auto it = std::find(lessons.begin(), lessons.end(), l);
        while(it != lessons.end())
        {
            const std::size_t r = std::distance(lessons.begin(), it);
            if(classrooms.at(r) != ClassroomAddress::NoClassroom())
            {
                const auto& request = scheduleData.SubjectRequests().at(r);
                resultSchedule.insert(ScheduleItem{.Address = l,
                                                   .SubjectRequestID = request.ID(),
                                                   .Classroom = classrooms.at(r).Classroom});
            }

            it = std::find(std::next(it), lessons.end(), l);
        }
    }

    return resultSchedule;
}
