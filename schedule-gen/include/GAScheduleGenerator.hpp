#pragma once
#include "ScheduleGenerator.hpp"

#include <random>
#include <chrono>
#include <vector>


class ScheduleIndividual
{
public:
    explicit ScheduleIndividual(const std::vector<SubjectRequest>& requests);

    [[nodiscard]] const std::vector<ClassroomAddress>& Classrooms() const;
    [[nodiscard]] const std::vector<std::size_t>& Lessons() const;

    void Mutate(const std::vector<SubjectRequest>& requests, std::mt19937& randGen);
    std::size_t Evaluate(const std::vector<SubjectRequest>& requests) const;
    void Crossover(ScheduleIndividual& other, std::size_t requestIndex);

private:
    [[nodiscard]] bool GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                                    std::size_t currentRequest,
                                                    std::size_t currentLesson) const;

    [[nodiscard]] bool ClassroomsIntersects(std::size_t currentLesson, const ClassroomAddress& currentClassroom) const;
    void Init(const std::vector<SubjectRequest>& requests, std::size_t requestIndex);
    void Change(const std::vector<SubjectRequest>& requests, std::mt19937& randGen);

private:
    void ChooseClassroom(const std::vector<SubjectRequest>& requests, std::size_t requestIndex, std::mt19937& randGen);
    void ChooseLesson(const std::vector<SubjectRequest>& requests, std::size_t requestIndex, std::mt19937& randGen);

private:
    mutable bool evaluated_;
    mutable std::size_t evaluatedValue_;
    std::vector<ClassroomAddress> classrooms_;
    std::vector<std::size_t> lessons_;
};

struct ScheduleGAStatistics
{
    ScheduleGAStatistics()
        : Time(0)
        , Iterations(0)
    {}

    explicit ScheduleGAStatistics(std::chrono::milliseconds time,
                                  std::size_t iterations)
        : Time(time)
        , Iterations(iterations)
    {}

    std::chrono::milliseconds Time;
    std::size_t Iterations;
};

class ScheduleGA
{
public:
    explicit ScheduleGA(std::vector<ScheduleIndividual> individuals);

    ScheduleGA& IterationsCount(std::size_t iterations);
    [[nodiscard]] std::size_t IterationsCount() const;

    ScheduleGA& SelectionCount(std::size_t selectionCount);
    [[nodiscard]] std::size_t SelectionCount() const;

    ScheduleGA& CrossoverCount(std::size_t crossoverCount);
    [[nodiscard]] std::size_t CrossoverCount() const;

    ScheduleGA& MutationChance(std::size_t mutationChance);
    [[nodiscard]] std::size_t MutationChance() const;

    [[nodiscard]] const ScheduleIndividual& Best() const;

    ScheduleGAStatistics Start(const std::vector<SubjectRequest>& requests);

private:
    std::size_t iterationsCount_;
    std::size_t selectionCount_;
    std::size_t crossoverCount_;
    std::size_t mutationChance_;
    std::vector<ScheduleIndividual> individuals_;
};


void Print(const ScheduleIndividual& individ,
           const std::vector<SubjectRequest>& requests);


class GAScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;
};
