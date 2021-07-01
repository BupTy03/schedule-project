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

ScheduleResult ToScheduleResult(const ScheduleChromosomes& chromosomes,
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
                resultSchedule.insert(ScheduleItem(l,
                                                   request.ID(),
                                                   classrooms.at(r).Classroom));
            }

            it = std::find(std::next(it), lessons.end(), l);
        }
    }

    return resultSchedule;
}

ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    const auto& subjectRequests = data.SubjectRequests();

    ScheduleGA algo(params_);
    const auto stat = algo.Start(data);
    const auto& bestIndividual = algo.Individuals().front();

    auto resultSchedule = ToScheduleResult(bestIndividual.Chromosomes(), data);
//    std::cout << '\n';
//    Print(bestIndividual, data);
//    std::cout << "\nSchedule done [";
//    std::cout << "score: " << bestIndividual.Evaluate() << "; ";
//    std::cout << "time: " << std::chrono::duration_cast<std::chrono::seconds>(stat.Time).count() << "s; ";
//    std::cout << "requests: " << subjectRequests.size() << "; ";
//    std::cout << "results: " << resultSchedule.items().size() << ']' << std::endl;
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
    for(auto&& locked : data.LockedLessons())
    {
        const std::size_t r = data.IndexOfSubjectRequestWithID(locked.SubjectRequestID);
        lessons_.at(r) = locked.Address;

        auto&& request = data.SubjectRequests().at(r);
        for(auto&& classroom : request.Classrooms())
        {
            if(!ClassroomsIntersects(locked.Address, classroom))
            {
                classrooms_.at(r) = classroom;
                break;
            }
        }
    }

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

    if(data.SubjectRequestHasLockedLesson(request))
        return;

    for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
    {
        for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
        {
            if(!request.RequestedWeekDay(day))
                continue;

            const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
            if(IsLateScheduleLessonInSaturday(scheduleLesson))
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

std::size_t ScheduleChromosomes::UnassignedLessonsCount() const
{
    return std::ranges::count(lessons_, NO_LESSON);
}

std::size_t ScheduleChromosomes::UnassignedClassroomsCount() const
{
    return std::ranges::count(classrooms_, ClassroomAddress::NoClassroom());
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



ScheduleIndividual::ScheduleIndividual(std::random_device& randomDevice,
                                       const ScheduleData* pData)
    : pData_(pData)
    , evaluatedValue_(NOT_EVALUATED)
    , chromosomes_(*pData)
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

    evaluatedValue_ = ::Evaluate(chromosomes_, *pData_);
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
    if(pData_->SubjectRequestHasLockedLesson(request))
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
    return ScheduleGAParams{
        .IndividualsCount = 1000,
        .IterationsCount = 1100,
        .SelectionCount = 360,
        .CrossoverCount = 220,
        .MutationChance = 49
    };
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
        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(),
                      ScheduleIndividualMutator(params_.MutationChance));

        // select best
        std::ranges::nth_element(individuals_, individuals_.begin() + params_.SelectionCount, ScheduleIndividualLess());
//        std::cout << "Iteration: " << iteration << "; Best: " << std::min_element(individuals_.begin(),
//                                                                                  individuals_.begin() + params_.SelectionCount,
//                                                                                  ScheduleIndividualLess())->Evaluate() << '\n';

        // crossover
        for(std::size_t i = 0; i < params_.CrossoverCount; ++i)
        {
            auto& firstInd = individuals_.at(selectionBestDist(randGen));
            auto& secondInd = individuals_.at(individualsDist(randGen));
            firstInd.Crossover(secondInd);
        }

        std::for_each(std::execution::par_unseq, individuals_.begin(), individuals_.end(),
                      ScheduleIndividualEvaluator());

        // natural selection
        std::ranges::nth_element(individuals_, individuals_.end() - params_.SelectionCount, ScheduleIndividualLess());
        std::copy_n(individuals_.begin(), params_.SelectionCount, individuals_.end() - params_.SelectionCount);
    }

    std::ranges::sort(individuals_, ScheduleIndividualLess());
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
    return result;
}

const std::vector<ScheduleIndividual>& ScheduleGA::Individuals() const
{
    assert(std::ranges::is_sorted(individuals_, ScheduleIndividualLess()));
    return individuals_;
}

