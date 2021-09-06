#include "GAScheduleGenerator.h"

#include <array>
#include <cassert>
#include <iostream>
#include <exception>
#include <execution>

#undef min
#undef max


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
    if(request.IsEveningClass())
    {
        while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
           !SuitableForEveningClasses(scheduleLesson) ||
           chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(*pData_, requestIndex, scheduleLesson)))
        {
            scheduleLesson = lessonsDistrib(randomGenerator_);
            ++chooseLessonTry;
        }
    }
    else
    {
        while(chooseLessonTry < MAX_LESSONS_COUNT &&
          (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
           IsLateScheduleLessonInSaturday(scheduleLesson) ||
           chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(*pData_, requestIndex, scheduleLesson)))
        {
            scheduleLesson = lessonsDistrib(randomGenerator_);
            ++chooseLessonTry;
        }
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

