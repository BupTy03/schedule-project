#include "ScheduleGA.h"

#include <algorithm>
#include <execution>


ScheduleGA::ScheduleGA()
    : ScheduleGA(ScheduleGA::DefaultParams())
{
}

ScheduleGA::ScheduleGA(const ScheduleGAParams& params)
    : params_(params)
    , individuals_()
{
    if(params_.IndividualsCount <= 0)
        throw std::invalid_argument("Invalid IndividualsCount option: must be greater than zero");

    if(params_.IterationsCount < 0)
        throw std::invalid_argument(
            "Invalid IterationsCount option: must be greater or equal to zero");

    if(params_.SelectionCount < 0 || params_.SelectionCount >= params_.IndividualsCount)
        throw std::invalid_argument(
            "Invalid SelectionCount option: must be greater or equal to zero and less than "
            "IndividualsCount");

    if(params_.CrossoverCount < 0)
        throw std::invalid_argument(
            "Invalid CrossoverCount option: must be greater or equal to zero");

    if(params_.MutationChance < 0 || params_.MutationChance > 100)
        throw std::invalid_argument("Invalid MutationChance option: must be in range [0, 100]");
}

ScheduleGAParams ScheduleGA::DefaultParams()
{
    return ScheduleGAParams{.IndividualsCount = 1000,
                            .IterationsCount = 1100,
                            .SelectionCount = 360,
                            .CrossoverCount = 220,
                            .MutationChance = 49};
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
        std::for_each(std::execution::par_unseq,
                      individuals_.begin(),
                      individuals_.end(),
                      ScheduleIndividualMutator(params_.MutationChance));

        // select best
        std::ranges::nth_element(
            individuals_, individuals_.begin() + params_.SelectionCount, ScheduleIndividualLess());
        /*std::cout << "Iteration: " << iteration << "; Best: " <<
        std::min_element(individuals_.begin(),
                                                                                  individuals_.begin()
                                                                                  +
                                                                                  params_.SelectionCount,
                                                                                  ScheduleIndividualLess())->Evaluate()
                                                                                  << '\n';*/

        // crossover
        for(std::size_t i = 0; i < params_.CrossoverCount; ++i)
        {
            auto& firstInd = individuals_.at(selectionBestDist(randGen));
            auto& secondInd = individuals_.at(individualsDist(randGen));
            firstInd.Crossover(secondInd);
        }

        std::for_each(std::execution::par_unseq,
                      individuals_.begin(),
                      individuals_.end(),
                      ScheduleIndividualEvaluator());

        // natural selection
        std::ranges::nth_element(
            individuals_, individuals_.end() - params_.SelectionCount, ScheduleIndividualLess());
        std::copy_n(individuals_.begin(),
                    params_.SelectionCount,
                    individuals_.end() - params_.SelectionCount);
    }

    std::ranges::sort(individuals_, ScheduleIndividualLess());
    result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - beginTime);
    return result;
}

const std::vector<ScheduleIndividual>& ScheduleGA::Individuals() const
{
    assert(std::ranges::is_sorted(individuals_, ScheduleIndividualLess()));
    return individuals_;
}

std::ostream& operator<<(std::ostream& os, const ScheduleGAParams& params)
{
    os << "IndividualsCount: " << params.IndividualsCount << '\n';
    os << "IterationsCount: " << params.IterationsCount << '\n';
    os << "SelectionCount: " << params.IterationsCount << '\n';
    os << "CrossoverCount: " << params.CrossoverCount << '\n';
    os << "MutationChance: " << params.MutationChance << '\n';
    return os;
}
