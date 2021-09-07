#pragma once
#include "ScheduleChromosomes.h"
#include <vector>
#include <random>


class ScheduleData;

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

void Print(const ScheduleIndividual& individ,
           const ScheduleData& data);

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
