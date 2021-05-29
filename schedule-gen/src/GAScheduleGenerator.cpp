#include "GAScheduleGenerator.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <exception>
#include <execution>

#undef min
#undef max

static constexpr std::size_t NO_LESSON = std::numeric_limits<std::size_t>::max();
static constexpr std::size_t NO_BUILDING = std::numeric_limits<std::size_t>::max();
static constexpr std::size_t NOT_EVALUATED = std::numeric_limits<std::size_t>::max();
static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024;


std::ostream& operator<<(std::ostream& os, const ScheduleGAParams& params)
{
    os << "IndividualsCount: " << params.IndividualsCount << '\n';
    os << "IterationsCount: " << params.IterationsCount << '\n';
    os << "SelectionCount: " << params.IterationsCount << '\n';
    os << "CrossoverCount: " << params.CrossoverCount << '\n';
    os << "MutationChance: " << params.MutationChance << '\n';
    return os;
}

ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    const auto& subjectRequests = data.SubjectRequests();

    ScheduleGA algo(params_);
    const auto stat = algo.Start(data);
    const auto& bestIndividual = algo.Individuals().front();

    const auto& lessons = bestIndividual.Chromosomes().Lessons();
    const auto& classrooms = bestIndividual.Chromosomes().Classrooms();

    ScheduleResult resultSchedule;
    for(std::size_t l = 0; l < MAX_LESSONS_COUNT; ++l)
    {
        auto it = std::find(lessons.begin(), lessons.end(), l);
        while(it != lessons.end())
        {
            const std::size_t r = std::distance(lessons.begin(), it);
            if(classrooms.at(r) != ClassroomAddress::NoClassroom())
            {
                const auto& request = subjectRequests.at(r);
                resultSchedule.insert(ScheduleItem(l,
                                                   request.ID(),
                                                   classrooms.at(r).Classroom));
            }

            it = std::find(std::next(it), lessons.end(), l);
        }
    }

    std::cout << '\n';
    Print(bestIndividual, data);
    std::cout << "\nSchedule done [";
    std::cout << "score: " << bestIndividual.Evaluate() << "; ";
    std::cout << "time: " << std::chrono::duration_cast<std::chrono::seconds>(stat.Time).count() << "s; ";
    std::cout << "requests: " << subjectRequests.size() << "; ";
    std::cout << "results: " << resultSchedule.items().size() << ']' << std::endl;
    return resultSchedule;
}

void GAScheduleGenerator::SetOptions(const std::map<std::string, ScheduleGenOption>& options)
{
    ScheduleGAParams newParams;
    newParams.IndividualsCount = RequireOption<int>(options, "individuals_count");
    newParams.IterationsCount = RequireOption<int>(options, "iterations_count");
    newParams.SelectionCount = RequireOption<int>(options, "selection_count");
    newParams.CrossoverCount = RequireOption<int>(options, "crossover_count");
    newParams.MutationChance = RequireOption<int>(options, "mutation_chance");
    params_ = newParams;
}

ScheduleGenOptions GAScheduleGenerator::DefaultOptions() const
{
    const auto defaultParams = ScheduleGA::DefaultParams();
    ScheduleGenOptions result;
    result.emplace("individuals_count", defaultParams.IndividualsCount);
    result.emplace("iterations_count", defaultParams.IterationsCount);
    result.emplace("selection_count", defaultParams.SelectionCount);
    result.emplace("crossover_count", defaultParams.CrossoverCount);
    result.emplace("mutation_chance", defaultParams.MutationChance);
    return result;
}

std::unique_ptr<ScheduleGenerator> GAScheduleGenerator::Clone() const
{
    auto result = std::make_unique<GAScheduleGenerator>();
    result->params_ = params_;
    return result;
}


ScheduleChromosomes::ScheduleChromosomes(std::vector<std::size_t> lessons,
                                         std::vector<ClassroomAddress> classrooms)
    : lessons_(std::move(lessons))
    , classrooms_(std::move(classrooms))
{
    assert(lessons_.size() == classrooms_.size());
}

ScheduleChromosomes::ScheduleChromosomes(const ScheduleData& data)
    : lessons_(data.SubjectRequests().size(), NO_LESSON)
    , classrooms_(data.SubjectRequests().size(), ClassroomAddress::NoClassroom())
{
    assert(!data.SubjectRequests().empty());
    for(std::size_t r = 0; r < data.SubjectRequests().size(); ++r)
        InitFromRequest(data, r);
}

void ScheduleChromosomes::InitFromRequest(const ScheduleData& data,
                                          std::size_t requestIndex)
{
    const auto& requests = data.SubjectRequests();
    const auto& request = requests.at(requestIndex);
    const auto& requestClassrooms = request.Classrooms();
    const auto& lockedLessons = data.LockedLessons();

    auto it = std::find_if(lockedLessons.begin(), lockedLessons.end(), [&](const SubjectWithAddress& subject){
           return subject.SubjectRequestID == request.ID();
    });

    if(it != lockedLessons.end())
    {
        lessons_.at(requestIndex) = it->Address;
        if(requestClassrooms.empty())
        {
            classrooms_.at(requestIndex) = ClassroomAddress::Any();
            return;
        }

        for(auto&& classroom : requestClassrooms)
        {
            if(!ClassroomsIntersects(it->Address, classroom))
            {
                classrooms_.at(requestIndex) = classroom;
                break;
            }
        }

        return;
    }

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
            if(!request.RequestedWeekDay(day))
                continue;

            const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
            if(IsLateScheduleLessonInSaturday(scheduleLesson))
                continue;

            if(data.LessonIsLocked(scheduleLesson))
                continue;

            if(GroupsOrProfessorsIntersects(data, requestIndex, scheduleLesson))
                continue;

            lessons_.at(requestIndex) = scheduleLesson;
            if(requestClassrooms.empty())
            {
                classrooms_.at(requestIndex) = ClassroomAddress::Any();
                return;
            }

            for(auto&& classroom : requestClassrooms)
            {
                if(!ClassroomsIntersects(scheduleLesson, classroom))
                {
                    classrooms_.at(requestIndex) = classroom;
                    return;
                }
            }
        }
    }
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


std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const ScheduleData& scheduleData,
                             const ScheduleChromosomes& scheduleChromosomes)
{
    std::size_t evaluatedValue = 0;

    using IntPair = std::pair<std::size_t, std::size_t>;
    using MapPair = std::pair<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>>;
    using BuilingPair = std::pair<std::size_t, std::array<std::size_t, MAX_LESSONS_PER_DAY>>;

    using ComplexityMap = SortedMap<std::size_t, std::size_t, LinearAllocator<IntPair>>;
    using WindowsMap = SortedMap<std::size_t, std::array<bool, MAX_LESSONS_PER_DAY>, LinearAllocator<MapPair>>;
    using BuildingsMap = SortedMap<std::size_t, std::array<std::size_t, MAX_LESSONS_PER_DAY>, LinearAllocator<BuilingPair>>;

    std::array<ComplexityMap, DAYS_IN_SCHEDULE> dayComplexity{
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

    std::array<WindowsMap, DAYS_IN_SCHEDULE> dayWindows{
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

    std::array<WindowsMap, DAYS_IN_SCHEDULE> professorsDayWindows{
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

    std::array<BuildingsMap, DAYS_IN_SCHEDULE> buildingsInDay{
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan)),
        BuildingsMap(LinearAllocator<BuilingPair>(&bufferSpan))
    };

    const auto requests = scheduleData.SubjectRequests();
    for(std::size_t r = 0; r < requests.size(); ++r)
    {
        const auto& request = requests.at(r);
        const std::size_t lesson = scheduleChromosomes.Lesson(r);
        if(lesson == NO_LESSON)
        {
            evaluatedValue += 100;
            continue;
        }

        const std::size_t day = lesson / MAX_LESSONS_PER_DAY;
        const std::size_t lessonInDay = lesson % MAX_LESSONS_PER_DAY;

        professorsDayWindows[day][request.Professor()][lessonInDay] = true;
        for(std::size_t group : request.Groups())
        {
            dayComplexity[day][group] += (lessonInDay * request.Complexity());
            dayWindows[day][group][lessonInDay] = true;

            auto& buildingsDay = buildingsInDay[day];
            auto it = buildingsDay.lower_bound(group);
            if(it == buildingsDay.end() || it->first != group)
            {
                std::array<std::size_t, MAX_LESSONS_PER_DAY> emptyBuildings;
                emptyBuildings.fill(NO_BUILDING);
                it = buildingsDay.emplace_hint(it, group, emptyBuildings);
            }

            if(scheduleChromosomes.Classroom(r) == ClassroomAddress::NoClassroom())
            {
                evaluatedValue += 100;
                continue;
            }

            it->second[lessonInDay] = scheduleChromosomes.Classroom(r).Building;
        }
    }

    std::size_t maxComplexityPerDay = 0;
    for(std::size_t d = 0; d < DAYS_IN_SCHEDULE; ++d)
    {
        // evaluating max complexity of day for group
        for(auto&& p : dayComplexity[d])
            maxComplexityPerDay = std::max(maxComplexityPerDay, p.second);

        // evaluating summary lessons gaps for groups
        for(auto&& p : dayWindows[d])
        {
            std::size_t prevLesson = 0;
            for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
            {
                if(p.second.at(lessonNum))
                {
                    const std::size_t lessonsGap = lessonNum - prevLesson;
                    prevLesson = lessonNum;

                    if(lessonsGap > 1)
                        evaluatedValue += lessonsGap * 3;
                }
            }
        }

        // evaluating summary lessons gaps for professors
        for(auto&& p : professorsDayWindows[d])
        {
            std::size_t prevLesson = 0;
            for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
            {
                if(p.second.at(lessonNum))
                {
                    const std::size_t lessonsGap = lessonNum - prevLesson;
                    prevLesson = lessonNum;

                    if(lessonsGap > 1)
                        evaluatedValue += lessonsGap * 2;
                }
            }
        }

        // evaluating count transitions between buildings per day
        for(auto&& p : buildingsInDay[d])
        {
            std::size_t prevBuilding = NO_BUILDING;
            for(std::size_t lessonNum = 0; lessonNum < MAX_LESSONS_PER_DAY; ++lessonNum)
            {
                const auto currentBuilding = p.second.at(lessonNum);
                if( !(currentBuilding == NO_BUILDING || prevBuilding == NO_BUILDING || currentBuilding == prevBuilding) )
                    evaluatedValue += 64;

                prevBuilding = currentBuilding;
            }
        }
    }

    evaluatedValue += maxComplexityPerDay;
    return evaluatedValue;
}


ScheduleIndividual::ScheduleIndividual(std::random_device& randomDevice,
                                       const ScheduleData* pData)
    : pData_(pData)
    , evaluatedValue_(NOT_EVALUATED)
    , chromosomes_(*pData)
    , buffer_(DEFAULT_BUFFER_SIZE)
    , randomGenerator_(randomDevice())
{
    assert(pData != nullptr);
}

void ScheduleIndividual::swap(ScheduleIndividual& other) noexcept
{
    std::swap(evaluatedValue_, other.evaluatedValue_);
    std::swap(chromosomes_, other.chromosomes_);
}

ScheduleIndividual::ScheduleIndividual(const ScheduleIndividual& other)
    : pData_(other.pData_)
    , evaluatedValue_(other.evaluatedValue_)
    , chromosomes_(other.chromosomes_)
    , buffer_(other.buffer_.size())
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(const ScheduleIndividual& other)
{
    ScheduleIndividual tmp(other);
    tmp.swap(*this);
    return *this;
}

ScheduleIndividual::ScheduleIndividual(ScheduleIndividual&& other) noexcept
    : pData_(other.pData_)
    , evaluatedValue_(other.evaluatedValue_)
    , chromosomes_(std::move(other.chromosomes_))
    , buffer_(other.buffer_.size())
    , randomGenerator_(other.randomGenerator_)
{
}

ScheduleIndividual& ScheduleIndividual::operator=(ScheduleIndividual&& other) noexcept
{
    other.swap(*this);
    return *this;
}

std::size_t ScheduleIndividual::MutationProbability() const
{
    std::uniform_int_distribution<std::size_t> mutateDistrib(0, 100);
    return mutateDistrib(randomGenerator_);
}

void ScheduleIndividual::Mutate()
{
    std::uniform_int_distribution<std::size_t> requestsDistrib(0, pData_->SubjectRequests().size() - 1);
    const std::size_t requestIndex = requestsDistrib(randomGenerator_);

    std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
    if(headsOrTails(randomGenerator_))
        ChangeClassroom(requestIndex);
    else
        ChangeLesson(requestIndex);
}

std::size_t ScheduleIndividual::Evaluate() const
{
    if(evaluatedValue_ != NOT_EVALUATED)
        return evaluatedValue_;

    LinearAllocatorBufferSpan bufferSpan(buffer_.data(), buffer_.size());
    evaluatedValue_ = EvaluateSchedule(bufferSpan, *pData_, chromosomes_);
    buffer_.resize(std::max(bufferSpan.peak, buffer_.size()));
    return evaluatedValue_;
}

void ScheduleIndividual::Crossover(ScheduleIndividual& other)
{
    std::uniform_int_distribution<std::size_t> requestsDist(0, pData_->SubjectRequests().size() - 1);
    const auto requestIndex = requestsDist(randomGenerator_);
    if(ReadyToCrossover(chromosomes_, other.chromosomes_, *pData_, requestIndex))
    {
        evaluatedValue_ = NOT_EVALUATED;
        other.evaluatedValue_ = NOT_EVALUATED;
        ::Crossover(chromosomes_, other.chromosomes_, requestIndex);
    }
}


void ScheduleIndividual::ChangeClassroom(std::size_t requestIndex)
{
    const auto& request = pData_->SubjectRequests().at(requestIndex);
    const auto& classrooms = request.Classrooms();

    if(classrooms.empty())
        return;

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() &&
          chromosomes_.ClassroomsIntersects(chromosomes_.Lesson(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator_));
        ++chooseClassroomTry;
    }

    if(chooseClassroomTry < classrooms.size())
    {
        chromosomes_.Classroom(requestIndex) = scheduleClassroom;
        evaluatedValue_ = NOT_EVALUATED;
    }
}

void ScheduleIndividual::ChangeLesson(std::size_t requestIndex)
{
    const auto& request = pData_->SubjectRequests().at(requestIndex);
    if(pData_->RequestHasLockedLesson(request))
        return;

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
    std::size_t scheduleLesson = lessonsDistrib(randomGenerator_);

    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
           IsLateScheduleLessonInSaturday(scheduleLesson) ||
           chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(*pData_, requestIndex, scheduleLesson)))
    {
        scheduleLesson = lessonsDistrib(randomGenerator_);
        ++chooseLessonTry;
    }

    if(chooseLessonTry < MAX_LESSONS_COUNT)
    {
        chromosomes_.Lesson(requestIndex) = scheduleLesson;
        evaluatedValue_ = NOT_EVALUATED;
    }
}


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs) { lhs.swap(rhs); }

void Print(const ScheduleIndividual& individ,
           const ScheduleData& data)
{
    const auto& requests = data.SubjectRequests();
    const auto& lessons = individ.Chromosomes().Lessons();
    const auto& classrooms = individ.Chromosomes().Classrooms();

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
                std::cout << "[s:" << request.ID() <<
                          ", p:" << request.Professor() <<
                          ", c:(" << classrooms.at(r).Building << ", " << classrooms.at(r).Classroom << "), g: {";

                for(auto&& g : request.Groups())
                    std::cout << ' ' << g;

                std::cout << " }]";

                it = std::find(std::next(it), lessons.end(), l);
            }
        }

        std::cout << '\n';
    }
}


ScheduleGA::ScheduleGA() : ScheduleGA(ScheduleGA::DefaultParams())
{
}

ScheduleGA::ScheduleGA(const ScheduleGAParams& params)
    : params_(params)
    , individuals_()
{
    if(params_.IndividualsCount <= 0)
        throw std::invalid_argument("Invalid IndividualsCount option: must be greater than zero");

    if(params_.IterationsCount < 0)
        throw std::invalid_argument("Invalid IterationsCount option: must be greater or equal to zero");

    if(params_.SelectionCount < 0 || params_.SelectionCount >= params_.IndividualsCount)
        throw std::invalid_argument("Invalid SelectionCount option: must be greater or equal to zero and less than IndividualsCount");

    if(params_.CrossoverCount < 0)
        throw std::invalid_argument("Invalid CrossoverCount option: must be greater or equal to zero");

    if(params_.MutationChance < 0 || params_.MutationChance > 100)
        throw std::invalid_argument("Invalid MutationChance option: must be in range [0, 100]");
}

ScheduleGAParams ScheduleGA::DefaultParams()
{
    ScheduleGAParams result;
    result.IndividualsCount = 1000;
    result.IterationsCount = 1100;
    result.SelectionCount = 360;
    result.CrossoverCount = 220;
    result.MutationChance = 49;
    return result;
}

ScheduleGAStatistics ScheduleGA::Start(const ScheduleData& scheduleData)
{
    std::random_device randomDevice;
    const ScheduleIndividual firstIndividual(randomDevice, &scheduleData);
    firstIndividual.Evaluate();

    individuals_.clear();
    individuals_.resize(params_.IndividualsCount, firstIndividual);

    std::mt19937 randGen(randomDevice());

    std::uniform_int_distribution<std::size_t> selectionBestDist(0, params_.SelectionCount - 1);
    std::uniform_int_distribution<std::size_t> individualsDist(0, individuals_.size() - 1);

    const auto beginTime = std::chrono::steady_clock::now();

    ScheduleGAStatistics result{};
    for(std::size_t iteration = 0; iteration < params_.IterationsCount; ++iteration)
    {
        // mutate
        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(), ScheduleIndividualMutator(params_.MutationChance));

        // select best
        std::nth_element(individuals_.begin(), individuals_.begin() + params_.SelectionCount, individuals_.end(), ScheduleIndividualLess());

        std::cout << "Iteration: " << iteration << "; Best: " << std::min_element(individuals_.begin(), individuals_.begin() + params_.SelectionCount, ScheduleIndividualLess())->Evaluate() << '\n';

        // crossover
        for(std::size_t i = 0; i < params_.CrossoverCount; ++i)
        {
            ScheduleIndividual& firstInd = individuals_.at(selectionBestDist(randGen));
            ScheduleIndividual& secondInd = individuals_.at(individualsDist(randGen));
            firstInd.Crossover(secondInd);
        }

        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(), ScheduleIndividualEvaluator());

        // natural selection
        std::nth_element(individuals_.begin(), individuals_.end() - params_.SelectionCount, individuals_.end(), ScheduleIndividualLess());
        std::copy_n(individuals_.begin(), params_.SelectionCount, individuals_.end() - params_.SelectionCount);
    }

    std::sort(individuals_.begin(), individuals_.end(), ScheduleIndividualLess());
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
    return result;
}

const std::vector<ScheduleIndividual>& ScheduleGA::Individuals() const
{
    assert(std::is_sorted(individuals_.begin(), individuals_.end(), ScheduleIndividualLess()));
    return individuals_;
}

