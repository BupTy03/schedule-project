#pragma once
#include "ScheduleUtils.h"
#include "ScheduleGenerator.h"
#include "ScheduleChromosomes.h"

#include <vector>
#include <random>
#include <chrono>


class ScheduleIndividual
{
public:
    explicit ScheduleIndividual(std::random_device& randomDevice,
                                const ScheduleData* pData);
    void swap(ScheduleIndividual& other) noexcept;

    ScheduleIndividual(const ScheduleIndividual& other);
    ScheduleIndividual& operator=(const ScheduleIndividual& other);

    ScheduleIndividual(ScheduleIndividual&& other) noexcept;
    ScheduleIndividual& operator=(ScheduleIndividual&& other) noexcept;

    const ScheduleData& Data() const { return *pData_; }
    const ScheduleChromosomes& Chromosomes() const { return chromosomes_; }

    std::size_t MutationProbability() const;
    void Mutate();
    std::size_t Evaluate() const;
    void Crossover(ScheduleIndividual& other);

private:
    void ChangeClassroom(std::size_t requestIndex);
    void ChangeLesson(std::size_t requestIndex);

private:
    const ScheduleData* pData_;
    mutable std::size_t evaluatedValue_;
    ScheduleChromosomes chromosomes_;
    mutable std::mt19937 randomGenerator_;
};

void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs);


struct ScheduleIndividualLess
{
    bool operator()(const ScheduleIndividual& lhs, const ScheduleIndividual& rhs) const
    {
        return lhs.Evaluate() < rhs.Evaluate();
    }
};

struct ScheduleIndividualEvaluator
{
    void operator()(ScheduleIndividual& individual) const
    {
        individual.Evaluate();
    }
};

struct ScheduleIndividualMutator
{
    explicit ScheduleIndividualMutator(std::size_t mutationChance)
        : MutationChance(mutationChance)
    { }

    void operator()(ScheduleIndividual& individual) const
    {
        if(individual.MutationProbability() <= MutationChance)
        {
            individual.Mutate();
            individual.Evaluate();
        }
    }

    std::size_t MutationChance;
};


void Print(const ScheduleIndividual& individ,
           const ScheduleData& data);

struct ScheduleGAStatistics
{
    std::chrono::milliseconds Time;
};


struct ScheduleGAParams
{
    int IndividualsCount = 0;
    int IterationsCount = 0;
    int SelectionCount = 0;
    int CrossoverCount = 0;
    int MutationChance = 0;
};

class ScheduleGA
{
public:
    ScheduleGA();
    explicit ScheduleGA(const ScheduleGAParams& params);

    static ScheduleGAParams DefaultParams();
    const ScheduleGAParams& Params() const { return params_; }

    ScheduleGAStatistics Start(const ScheduleData& scheduleData);
    const std::vector<ScheduleIndividual>& Individuals() const;

private:
    ScheduleGAParams params_;
    std::vector<ScheduleIndividual> individuals_;
};


ScheduleResult ToScheduleResult(const ScheduleChromosomes& chromosomes,
                                const ScheduleData& scheduleData);

class GAScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;

    void SetOptions(const ScheduleGenOptions& options) override;
    ScheduleGenOptions DefaultOptions() const override;

    std::unique_ptr<ScheduleGenerator> Clone() const override;

public:
    ScheduleGAParams params_ = ScheduleGA::DefaultParams();
};
