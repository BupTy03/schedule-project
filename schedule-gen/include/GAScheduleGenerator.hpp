#pragma once
#include "ScheduleUtils.hpp"
#include "ScheduleGenerator.hpp"

#include <vector>
#include <random>
#include <chrono>
#include <tuple>


bool GroupsOrProfessorsOrClassroomsIntersects(const std::vector<SubjectRequest>& requests,
                                              const std::vector<std::size_t>& lessons,
                                              const std::vector<ClassroomAddress>& classrooms,
                                              std::size_t currentRequest,
                                              std::size_t currentLesson);

bool GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                  const std::vector<std::size_t>& lessons,
                                  std::size_t currentRequest,
                                  std::size_t currentLesson);

bool ClassroomsIntersects(const std::vector<std::size_t>& lessons,
                          const std::vector<ClassroomAddress>& classrooms,
                          std::size_t currentLesson,
                          const ClassroomAddress& currentClassroom);

void InitChromosomes(std::vector<std::size_t>& lessons,
                     std::vector<ClassroomAddress>& classrooms,
                     const std::vector<SubjectRequest>& requests,
                     std::size_t requestIndex);

std::tuple<std::vector<std::size_t>, std::vector<ClassroomAddress>> InitChromosomes(const std::vector<SubjectRequest>& requests);

std::size_t EvaluateSchedule(LinearAllocatorBufferSpan& bufferSpan,
                             const std::vector<SubjectRequest>& requests,
                             const std::vector<std::size_t>& lessons,
                             const std::vector<ClassroomAddress>& classrooms);


class ScheduleIndividual
{
    static constexpr std::size_t DEFAULT_BUFFER_SIZE = 1024;
public:
    explicit ScheduleIndividual(std::random_device& randomDevice,
                                const std::vector<SubjectRequest>* pRequests);
    void swap(ScheduleIndividual& other) noexcept;

    ScheduleIndividual(const ScheduleIndividual& other);
    ScheduleIndividual& operator=(const ScheduleIndividual& other);

    ScheduleIndividual(ScheduleIndividual&& other) noexcept;
    ScheduleIndividual& operator=(ScheduleIndividual&& other) noexcept;

    const std::vector<SubjectRequest>& Requests() const;
    const std::vector<ClassroomAddress>& Classrooms() const;
    const std::vector<std::size_t>& Lessons() const;

    std::size_t MutationProbability() const;
    void Mutate();
    std::size_t Evaluate() const;
    void Crossover(ScheduleIndividual& other);

private:
    void ChangeClassroom(std::size_t requestIndex);
    void ChangeLesson(std::size_t requestIndex);

private:
    const std::vector<SubjectRequest>* pRequests_;
    mutable std::size_t evaluatedValue_;

    std::vector<ClassroomAddress> classrooms_;
    std::vector<std::size_t> lessons_;

    mutable std::vector<std::uint8_t> buffer_;
    mutable std::mt19937 randomGenerator_;
};

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


void swap(ScheduleIndividual& lhs, ScheduleIndividual& rhs) noexcept;

bool ReadyToCrossover(const ScheduleIndividual& first,
                      const ScheduleIndividual& second,
                      std::size_t requestIndex);

void Print(const ScheduleIndividual& individ);


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

std::ostream& operator<<(std::ostream& os, const ScheduleGAParams& params);


class ScheduleGA
{
public:
    ScheduleGA();
    explicit ScheduleGA(const ScheduleGAParams& params);

    static ScheduleGAParams DefaultParams();
    const ScheduleGAParams& Params() const;

    ScheduleGAStatistics Start(const std::vector<SubjectRequest>& requests);
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
