#include "GAScheduleGenerator.hpp"

#include <array>
#include <iostream>
#include <execution>


ScheduleIndividual::ScheduleIndividual(const std::vector<SubjectRequest> &requests)
        : evaluated_(false)
        , evaluatedValue_(std::numeric_limits<std::size_t>::max())
        , classrooms_(requests.size(), std::numeric_limits<std::size_t>::max())
        , lessons_(requests.size(), std::numeric_limits<std::size_t>::max())
{
    for(std::size_t i = 0; i < requests.size(); ++i)
        Init(requests, i);
}

const std::vector<std::size_t> &ScheduleIndividual::Classrooms() const { return classrooms_; }

const std::vector<std::size_t> &ScheduleIndividual::Lessons() const { return lessons_; }

void ScheduleIndividual::Mutate(const std::vector<SubjectRequest>& requests, std::mt19937& randGen)
{
    assert(requests.size() == classrooms_.size());
    assert(requests.size() == lessons_.size());

    evaluated_ = false;
    Change(requests, randGen);
}

std::size_t ScheduleIndividual::Evaluate(const std::vector<SubjectRequest>& requests) const
{
    if(evaluated_)
        return evaluatedValue_;

    evaluated_ = true;
    evaluatedValue_ = 0;

    std::array<std::uint8_t, 20480> buffer{};
    LinearAllocatorBufferSpan bufferSpan(buffer.data(), buffer.size());

    using IntPair = std::pair<std::size_t, std::size_t>;
    using MapPair = std::pair<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>>;

    using ComplexityMap = SortedMap<std::size_t, std::size_t, LinearAllocator<IntPair>>;
    using WindowsMap = SortedMap<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>, LinearAllocator<MapPair>>;

    std::array<ComplexityMap, DAYS_IN_SCHEDULE> dayComplexity_{
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan)),
            ComplexityMap(LinearAllocator<IntPair>(&bufferSpan))
    };

    std::array<WindowsMap, DAYS_IN_SCHEDULE> dayWindows_{
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan))
    };

    std::array<WindowsMap, DAYS_IN_SCHEDULE> professorsDayWindows_{
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan)),
            WindowsMap(LinearAllocator<MapPair>(&bufferSpan))
    };

    for(std::size_t r = 0; r < lessons_.size(); ++r)
    {
        const auto& request = requests.at(r);
        const std::size_t lesson = lessons_.at(r);
        const std::size_t day = LessonToScheduleDay(lesson);
        const std::size_t lessonInDay = lesson % MAX_LESSONS_PER_DAY;

        professorsDayWindows_.at(day)[request.Professor()][lessonInDay] = true;
        for(std::size_t group : request.Groups())
        {
            dayComplexity_.at(day)[group] += (lessonInDay * request.Complexity());
            dayWindows_.at(day)[group][lessonInDay] = true;
        }
    }

    // evaluating summary complexity of day for group
    for(auto&& day : dayComplexity_)
    {
        for(auto&& p : day)
            evaluatedValue_ = std::max(evaluatedValue_, p.second);
    }

    // evaluating summary lessons gaps
    for(auto&& day : dayWindows_)
    {
        for(auto&& p : day)
        {
            std::size_t prevLesson = 0;
            for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
            {
                if(p.second.at(lessonNum))
                {
                    const std::size_t lessonsGap = lessonNum - prevLesson;
                    prevLesson = lessonNum;

                    if(lessonsGap > 1)
                        evaluatedValue_ += lessonsGap * 3;
                }
            }
        }
    }

    // evaluating summary lessons gaps for professors
    for(auto&& day : professorsDayWindows_)
    {
        for(auto&& p : day)
        {
            std::size_t prevLesson = 0;
            for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
            {
                if(p.second.at(lessonNum))
                {
                    const std::size_t lessonsGap = lessonNum - prevLesson;
                    prevLesson = lessonNum;

                    if(lessonsGap > 1)
                        evaluatedValue_ += lessonsGap * 2;
                }
            }
        }
    }

    return evaluatedValue_;
}

void ScheduleIndividual::Crossover(ScheduleIndividual& other, std::size_t requestIndex)
{
    assert(classrooms_.size() == lessons_.size());
    assert(other.classrooms_.size() == other.lessons_.size());
    assert(other.lessons_.size() == lessons_.size());

    evaluated_ = false;
    other.evaluated_ = false;

    std::swap(classrooms_.at(requestIndex), other.classrooms_.at(requestIndex));
    std::swap(lessons_.at(requestIndex), other.lessons_.at(requestIndex));
}

bool ScheduleIndividual::GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                                      std::size_t currentRequest,
                                                      std::size_t currentLesson) const
{
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

bool ScheduleIndividual::ClassroomsIntersects(std::size_t currentLesson, std::size_t currentClassroom) const
{
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

void ScheduleIndividual::Init(const std::vector<SubjectRequest>& requests, std::size_t requestIndex)
{
    assert(requests.size() == classrooms_.size());
    assert(requests.size() == lessons_.size());

    const auto& request = requests.at(requestIndex);
    const auto& classrooms = request.Classrooms();

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
            if(!request.RequestedWeekDay(day))
                continue;

            const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;

            // lessons count is less in Saturday than in other days
            if(IsLateScheduleLessonInSaturday(scheduleLesson))
                continue;

            if(GroupsOrProfessorsIntersects(requests, requestIndex, scheduleLesson))
                continue;

            for(std::size_t classroom : classrooms)
            {
                if(!ClassroomsIntersects(scheduleLesson, classroom))
                {
                    classrooms_.at(requestIndex) = classroom;
                    lessons_.at(requestIndex) = scheduleLesson;
                    return;
                }
            }
        }
    }

    assert(false);
}

void ScheduleIndividual::Change(const std::vector<SubjectRequest>& requests, std::mt19937& randGen)
{
    assert(requests.size() == classrooms_.size());
    assert(requests.size() == lessons_.size());

    std::uniform_int_distribution<std::size_t> distrib(0, requests.size() - 1);
    const std::size_t requestIndex = distrib(randGen);
    ChooseClassroom(requests, requestIndex, randGen);
    ChooseLesson(requests, requestIndex, randGen);
}

void ScheduleIndividual::ChooseClassroom(const std::vector<SubjectRequest>& requests,
                                         std::size_t requestIndex,
                                         std::mt19937& randGen)
{
    const auto& request = requests.at(requestIndex);
    const auto& classrooms = request.Classrooms();

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    std::size_t scheduleClassroom = classrooms.at(classroomDistrib(randGen));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() && ClassroomsIntersects(lessons_.at(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randGen));
        ++chooseClassroomTry;
    }

    if(chooseClassroomTry < classrooms.size())
        classrooms_.at(requestIndex) = scheduleClassroom;
}

void ScheduleIndividual::ChooseLesson(const std::vector<SubjectRequest>& requests,
                                      std::size_t requestIndex,
                                      std::mt19937& randGen)
{
    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
    std::size_t scheduleLesson = lessonsDistrib(randGen);

    const auto& request = requests.at(requestIndex);

    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (IsLateScheduleLessonInSaturday(scheduleLesson) || !request.RequestedWeekDay(LessonToScheduleDay(scheduleLesson)) ||
           ClassroomsIntersects(scheduleLesson, classrooms_.at(requestIndex)) ||
           GroupsOrProfessorsIntersects(requests, requestIndex, scheduleLesson)))
    {
        scheduleLesson = lessonsDistrib(randGen);
        ++chooseLessonTry;
    }

    if(chooseLessonTry < MAX_LESSONS_COUNT)
        lessons_.at(requestIndex) = scheduleLesson;
}


ScheduleGA::ScheduleGA(std::vector<ScheduleIndividual> individuals)
        : iterationsCount_(1000)
        , selectionCount_(125)
        , crossoverCount_(25)
        , mutationChance_(45)
        , individuals_(std::move(individuals))
{ }

ScheduleGA& ScheduleGA::IterationsCount(std::size_t iterations) { iterationsCount_ = iterations; return *this; }

std::size_t ScheduleGA::IterationsCount() const { return iterationsCount_; }

ScheduleGA& ScheduleGA::SelectionCount(std::size_t selectionCount) { selectionCount_ = selectionCount; return *this; }

std::size_t ScheduleGA::SelectionCount() const { return selectionCount_; }

ScheduleGA& ScheduleGA::CrossoverCount(std::size_t crossoverCount) { crossoverCount_ = crossoverCount; return *this; }

std::size_t ScheduleGA::CrossoverCount() const { return crossoverCount_; }

ScheduleGA& ScheduleGA::MutationChance(std::size_t mutationChance)
{
    assert(mutationChance >= 0 && mutationChance <= 100);
    mutationChance_ = mutationChance;
    return *this;
}

std::size_t ScheduleGA::MutationChance() const { return mutationChance_; }

const ScheduleIndividual& ScheduleGA::Best() const { return individuals_.front(); }

ScheduleGAStatistics ScheduleGA::Start(const std::vector<SubjectRequest>& requests)
{
    assert(selectionCount_ < individuals_.size());
    assert(crossoverCount_ < selectionCount_);

    std::random_device randomDevice;
    std::mt19937 randGen(randomDevice());

    std::uniform_int_distribution<std::size_t> selectionBestDist(0, SelectionCount() - 1);
    std::uniform_int_distribution<std::size_t> selectionDist(0, individuals_.size() - 1);
    std::uniform_int_distribution<std::size_t> requestsDist(0, requests.size() - 1);

    auto individualLess = [&](const ScheduleIndividual& lhs, const ScheduleIndividual& rhs) {
        return lhs.Evaluate(requests) < rhs.Evaluate(requests);
    };

    const auto beginTime = std::chrono::steady_clock::now();

    ScheduleGAStatistics result;
    std::size_t prevEvaluated = std::numeric_limits<std::size_t>::max();
    for(std::size_t iteration = 0; iteration < IterationsCount(); ++iteration)
    {
        // mutate
        std::for_each(std::execution::par, individuals_.begin(), individuals_.end(), [&](ScheduleIndividual& individual)
        {
            std::random_device rd;
            std::mt19937 rg(rd());
            std::uniform_int_distribution<std::size_t> mutateDistrib(0, 100);
            if(mutateDistrib(rg) <= MutationChance())
            {
                individual.Mutate(requests, rg);
                individual.Evaluate(requests);
            }
        });

        // select best
        std::partial_sort(individuals_.begin(), individuals_.begin() + SelectionCount(), individuals_.end(), individualLess);
        const std::size_t currentBestEvaluated = individuals_.front().Evaluate(requests);

        std::cout << "Iteration: " << iteration << "; Best: " << currentBestEvaluated << '\n';
        if(currentBestEvaluated < prevEvaluated)
            result.Iterations = iteration;

        prevEvaluated = currentBestEvaluated;

        if(currentBestEvaluated == 0)
            break;

        // crossover
        for(std::size_t i = 0; i < CrossoverCount(); ++i)
        {
            auto& firstInd = individuals_.at(selectionBestDist(randGen));
            auto& secondInd = individuals_.at(selectionDist(randGen));

            firstInd.Crossover(secondInd, requestsDist(randGen));
            firstInd.Evaluate(requests);
            secondInd.Evaluate(requests);
        }

        // natural selection
        std::nth_element(individuals_.begin(), individuals_.end() - SelectionCount(), individuals_.end(), individualLess);
        std::sort(individuals_.end() - SelectionCount(), individuals_.end(), individualLess);
        std::copy_n(individuals_.begin(), SelectionCount(), individuals_.end() - SelectionCount());
    }

    std::sort(individuals_.begin(), individuals_.end(), individualLess);
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
    return result;
}


void Print(const ScheduleIndividual& individ, const std::vector<SubjectRequest>& requests)
{
    const auto& lessons = individ.Lessons();
    const auto& classrooms = individ.Classrooms();

    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        std::cout << "Lesson " << l << ": ";

        auto it = std::find(lessons.begin(), lessons.end(), l);
        if(it == lessons.end())
        {
            std::cout << '-';
        }
        else
        {
            while(it != lessons.end())
            {
                const std::size_t r = std::distance(lessons.begin(), it);
                const auto& request = requests.at(r);
                std::cout << "[s:" << r << ", p:" << request.Professor() << ", c:" << classrooms.at(r) << "]";

                it = std::find(std::next(it), lessons.end(), l);
            }
        }

        std::cout << '\n';
    }
}

ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    const auto& subjectRequests = data.SubjectRequests();

    ScheduleGA algo(std::vector<ScheduleIndividual>(1000, ScheduleIndividual(subjectRequests)));
    const auto stat = algo.IterationsCount(1000)
            .SelectionCount(200)
            .CrossoverCount(25)
            .MutationChance(45)
            .Start(subjectRequests);

    const auto& bestIndividual = algo.Best();
    Print(bestIndividual, subjectRequests);
    std::cout << "Best: " << bestIndividual.Evaluate(subjectRequests) << '\n';
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(stat.Time).count() << "s.\n";
    std::cout << "Iterations: " << stat.Iterations << '\n';
    std::cout.flush();

    const auto& lessons = bestIndividual.Lessons();
    const auto& classrooms = bestIndividual.Classrooms();

    ScheduleResult resultSchedule;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        auto it = std::find(lessons.begin(), lessons.end(), l);
        while(it != lessons.end())
        {
            const std::size_t r = std::distance(lessons.begin(), it);
            const auto& request = subjectRequests.at(r);
            for(std::size_t g : request.Groups())
            {
                resultSchedule.insert(ScheduleItem(LessonAddress(g, l),
                                           r,
                                           request.ID(),
                                           classrooms.at(r)));
            }

            it = std::find(std::next(it), lessons.end(), l);
        }
    }

    return resultSchedule;
}
