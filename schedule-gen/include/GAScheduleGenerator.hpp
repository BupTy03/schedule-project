#pragma once
#include "ScheduleUtils.hpp"
#include "ScheduleGenerator.hpp"

#include <vector>
#include <random>
#include <chrono>


class ScheduleChromosomes
{
public:
    // for testing
    explicit ScheduleChromosomes(std::vector<std::size_t> lessons,
                                 std::vector<ClassroomAddress> classrooms);

    explicit ScheduleChromosomes(const ScheduleData& data);

    const std::vector<std::size_t>& Lessons() const { return lessons_; }
    const std::vector<ClassroomAddress>& Classrooms() const { return classrooms_; }

    std::size_t Lesson(std::size_t r) const { return lessons_.at(r); }
    std::size_t& Lesson(std::size_t r) { return lessons_.at(r); }

    ClassroomAddress Classroom(std::size_t r) const { return classrooms_.at(r); }
    ClassroomAddress& Classroom(std::size_t r) { return classrooms_.at(r); }

    bool GroupsOrProfessorsOrClassroomsIntersects(const ScheduleData& data,
                                                  std::size_t currentRequest,
                                                  std::size_t currentLesson) const;

    bool GroupsOrProfessorsIntersects(const ScheduleData& data,
                                      std::size_t currentRequest,
                                      std::size_t currentLesson) const;

    bool ClassroomsIntersects(std::size_t currentLesson,
                              const ClassroomAddress& currentClassroom) const;

private:
    void InitFromRequest(const ScheduleData& data, std::size_t requestIndex);

private:
    std::vector<std::size_t> lessons_;
    std::vector<ClassroomAddress> classrooms_;
};

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r);

void Crossover(ScheduleChromosomes& first,
               ScheduleChromosomes& second,
               std::size_t r);

std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const ScheduleData& scheduleData,
                             const ScheduleChromosomes& scheduleChromosomes);


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
    mutable std::vector<std::uint8_t> buffer_;
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


class GAScheduleGenerator : public ScheduleGenerator
{
public:
    void SetOptions(const std::map<std::string, ScheduleGenOption>& options) override;
    ScheduleResult Generate(const ScheduleData& data) override;

public:
    ScheduleGAParams params_ = ScheduleGA::DefaultParams();
};
