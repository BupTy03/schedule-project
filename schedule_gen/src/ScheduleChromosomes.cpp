#include "ScheduleChromosomes.h"
#include "ScheduleData.h"
#include <array>
#include <utility>
#include <numeric>
#include <algorithm>
#include <experimental/generator>


static std::experimental::generator<std::pair<std::size_t, std::size_t>> EveningClassesDaysLessons()
{
    // пытаемся разместить 5-6 парами (отсчёт с 0) в будние дни
    for(std::size_t dayLesson : std::array{5, 6})
        for(std::size_t day : std::array{0, 1, 2, 3, 4, 6, 7, 8, 9, 10})
            co_yield std::pair<std::size_t, std::size_t>{day, dayLesson};

    // не получилось разместить в будние дни - размещаем в субботу
    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
        for(std::size_t day : std::array{5, 11})
            co_yield std::pair<std::size_t, std::size_t>{day, dayLesson};
}

static std::experimental::generator<std::pair<std::size_t, std::size_t>> MorningClassesDaysLessons()
{
    // размещение занятий для дневников
    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
            co_yield std::pair<std::size_t, std::size_t>{day, dayLesson};
}


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


void InsertMorningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex)
{
    const auto& requests = data.SubjectRequests();
    const auto& request = requests.at(requestIndex);
    const auto& requestClassrooms = request.Classrooms();
    for(auto&&[day, lesson] : MorningClassesDaysLessons())
    {
        if(!request.RequestedWeekDay(day))
            continue;

        const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + lesson;
        if(IsLateScheduleLessonInSaturday(scheduleLesson))
            continue;

        if(chromosomes.GroupsOrProfessorsIntersects(data, requestIndex, scheduleLesson))
            continue;

        chromosomes.Lesson(requestIndex) = scheduleLesson;
        if(requestClassrooms.empty())
        {
            chromosomes.Classroom(requestIndex) = ClassroomAddress::Any();
            return;
        }

        for(auto&& classroom : requestClassrooms)
        {
            if(!chromosomes.ClassroomsIntersects(scheduleLesson, classroom))
            {
                chromosomes.Classroom(requestIndex) = classroom;
                return;
            }
        }
    }
}

void InsertEveningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex)
{
    const auto& requests = data.SubjectRequests();
    const auto& request = requests.at(requestIndex);
    const auto& requestClassrooms = request.Classrooms();
    for(auto&&[day, lesson] : EveningClassesDaysLessons())
    {
        if(!request.RequestedWeekDay(day))
            continue;

        if(ToWeekDay(day) == WeekDay::Saturday)
            continue;

        const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + lesson;
        if(chromosomes.GroupsOrProfessorsIntersects(data, requestIndex, scheduleLesson))
            continue;

        chromosomes.Lesson(requestIndex) = scheduleLesson;
        if(requestClassrooms.empty())
        {
            chromosomes.Classroom(requestIndex) = ClassroomAddress::Any();
            return;
        }

        for(auto&& classroom : requestClassrooms)
        {
            if(!chromosomes.ClassroomsIntersects(scheduleLesson, classroom))
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

    ScheduleChromosomes result(requests.size());
    for(auto&& locked : data.LockedLessons())
    {
        const std::size_t r = data.IndexOfSubjectRequestWithID(locked.SubjectRequestID);
        result.Lesson(r) = locked.Address;

        const auto& request = requests.at(r);
        for(auto&& classroom : request.Classrooms())
        {
            if(!result.ClassroomsIntersects(locked.Address, classroom))
            {
                result.Classroom(r) = classroom;
                break;
            }
        }
    }

    for(std::size_t r = 0; r < requests.size(); ++r)
    {
        const auto& request = requests.at(r);
        const auto& requestClassrooms = request.Classrooms();
        if(data.SubjectRequestHasLockedLesson(request))
            continue;

        if(request.IsEveningClass())
            InsertEveningRequest(result, data, r);
        else
            InsertMorningRequest(result, data, r);
    }

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
