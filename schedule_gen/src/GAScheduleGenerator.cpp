#include "GAScheduleGenerator.h"
#include "ScheduleGA.h"
#include <array>
#include <cassert>
#include <iostream>
#include <exception>
#include <execution>

#undef min
#undef max


ScheduleResult GAScheduleGenerator::Generate(const ScheduleData& data)
{
    const auto& subjectRequests = data.SubjectRequests();

    ScheduleGA algo(params_);
    const auto stat = algo.Start(data);
    const auto& bestIndividual = algo.Individuals().front();

    auto resultSchedule = MakeScheduleResult(bestIndividual.Chromosomes(), data);
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
