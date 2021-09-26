#include "ScheduleGA.h"

#include <algorithm>
#include <execution>


void ScheduleGA::SetParams(const ScheduleGAParams& params)
{
    if(params.IndividualsCount <= 0)
        throw std::invalid_argument("Invalid IndividualsCount option: must be greater than zero");

    if(params.IterationsCount < 0)
        throw std::invalid_argument(
            "Invalid IterationsCount option: must be greater or equal to zero");

    if(params.SelectionCount < 0 || params.SelectionCount >= params.IndividualsCount)
        throw std::invalid_argument(
            "Invalid SelectionCount option: must be greater or equal to zero and less than "
            "IndividualsCount");

    if(params.CrossoverCount < 0)
        throw std::invalid_argument(
            "Invalid CrossoverCount option: must be greater or equal to zero");

    if(params.MutationChance < 0 || params.MutationChance > 100)
        throw std::invalid_argument("Invalid MutationChance option: must be in range [0, 100]");

    params_ = params;
}

ScheduleGAParams ScheduleGA::DefaultParams()
{
    return ScheduleGAParams{.IndividualsCount = 1000,
                            .IterationsCount = 1100,
                            .SelectionCount = 360,
                            .CrossoverCount = 220,
                            .MutationChance = 49};
}

ScheduleIndividual ScheduleGA::operator()(const ScheduleData& scheduleData) const
{
    std::random_device randomDevice;
    const ScheduleIndividual firstIndividual(randomDevice, &scheduleData);
    firstIndividual.Evaluate();

    std::vector<ScheduleIndividual> individuals(params_.IndividualsCount, firstIndividual);

    std::mt19937 randGen(randomDevice());
    std::uniform_int_distribution<std::size_t> selectionBestDist(0, params_.SelectionCount - 1);
    std::uniform_int_distribution<std::size_t> individualsDist(0, individuals.size() - 1);

    const std::chrono::seconds maxTime{5};
    const auto beginTime = std::chrono::steady_clock::now();
    for(std::size_t iteration = 0; iteration < std::numeric_limits<std::size_t>::max(); ++iteration)
    {
        // mutate
        std::for_each(std::execution::par_unseq,
                      individuals.begin(),
                      individuals.end(),
                      ScheduleIndividualMutator(params_.MutationChance));

        // select best
        std::ranges::nth_element(
            individuals, individuals.begin() + params_.SelectionCount, ScheduleIndividualLess());
        std::cout << "Iteration: " << iteration << "; Best: " <<
        std::min_element(individuals.begin(),
                        individuals.begin()
                        +
                        params_.SelectionCount,
                        ScheduleIndividualLess())->Evaluate()
                        << '\n';

        // crossover
        for(std::size_t i = 0; i < params_.CrossoverCount; ++i)
        {
            auto& firstInd = individuals.at(selectionBestDist(randGen));
            auto& secondInd = individuals.at(individualsDist(randGen));
            firstInd.Crossover(secondInd);
        }

        std::for_each(std::execution::par_unseq,
                      individuals.begin(),
                      individuals.end(),
                      ScheduleIndividualEvaluator());

        // natural selection
        std::ranges::nth_element(
            individuals, individuals.end() - params_.SelectionCount, ScheduleIndividualLess());
        std::copy_n(individuals.begin(),
                    params_.SelectionCount,
                    individuals.end() - params_.SelectionCount);

        if(std::chrono::steady_clock::now() - beginTime > maxTime)
            break;
    }

    auto it = std::min_element(individuals.begin(), individuals.end(), ScheduleIndividualLess());
    return *it;
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


ScheduleResult Generate(const ScheduleGA& generator, const ScheduleData& data)
{
    const auto bestIndividual = generator(data);
    return MakeScheduleResult(bestIndividual.Chromosomes(), data);
}
