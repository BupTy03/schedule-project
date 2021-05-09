#pragma once
#include "ScheduleGenerator.hpp"

#include <random>
#include <iostream>
#include <execution>


constexpr auto MAX_LESSONS_PER_DAY = 6;
constexpr auto DAYS_IN_SCHEDULE_WEEK = 6;
constexpr auto DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr auto MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE_WEEK * 2;

class ScheduleIndividual
{
public:
    explicit ScheduleIndividual(const std::vector<SubjectRequest>& requests)
            : evaluated_(false)
            , evaluatedValue_(std::numeric_limits<std::size_t>::max())
            , classrooms_(requests.size(), std::numeric_limits<std::size_t>::max())
            , lessons_(requests.size(), std::numeric_limits<std::size_t>::max())
    {
        for(std::size_t i = 0; i < requests.size(); ++i)
            Init(requests, i);
    }

    const std::vector<std::size_t>& Classrooms() const { return classrooms_; }
    const std::vector<std::size_t>& Lessons() const { return lessons_; }

    void Mutate(const std::vector<SubjectRequest>& requests, std::mt19937& randGen)
    {
        assert(requests.size() == classrooms_.size());
        assert(requests.size() == lessons_.size());

        evaluated_ = false;
        Change(requests, randGen);
    }

    std::size_t Evaluate(const std::vector<SubjectRequest>& requests) const
    {
        if(evaluated_)
            return evaluatedValue_;

        evaluated_ = true;
        evaluatedValue_ = 0;

        std::vector<SortedMap<std::size_t, std::size_t>> dayComplexity_(DAYS_IN_SCHEDULE);
        std::vector<SortedMap<std::size_t, SortedSet<std::size_t>>> dayWindows_(DAYS_IN_SCHEDULE);
        std::vector<SortedMap<std::size_t, SortedSet<std::size_t>>> professorsDayWindows_(DAYS_IN_SCHEDULE);

        for(std::size_t r = 0; r < lessons_.size(); ++r)
        {
            const auto& request = requests.at(r);
            const std::size_t lesson = lessons_.at(r);
            const std::size_t day = lesson / MAX_LESSONS_PER_DAY;
            const std::size_t lessonInDay = lesson % MAX_LESSONS_PER_DAY;

            professorsDayWindows_.at(day)[request.Professor()].insert(lessonInDay);
            for(std::size_t group : request.Groups())
            {
                dayComplexity_.at(day)[group] += (lessonInDay * request.Complexity());
                dayWindows_.at(day)[group].insert(lessonInDay);
            }
        }

        // evaluating summary complexity of day for group
        for(auto&& day : dayComplexity_)
        {
            for(auto&& p : day)
                evaluatedValue_ = std::max(evaluatedValue_, p.second);
        }

        // evaluating summary lessons gaps
        for(auto&& day : dayWindows_)
        {
            for(auto&& p : day)
            {
                std::size_t prevLesson = 0;
                for(std::size_t lesson : p.second)
                {
                    const std::size_t lessonsGap = lesson - prevLesson;
                    if(lessonsGap > 1)
                        evaluatedValue_ += lessonsGap * 3;

                    prevLesson = lesson;
                }
            }
        }

        // evaluating summary lessons gaps for professors
        for(auto&& day : professorsDayWindows_)
        {
            for(auto&& p : day)
            {
                std::size_t prevLesson = 0;
                for(std::size_t lesson : p.second)
                {
                    const std::size_t lessonsGap = lesson - prevLesson;
                    if(lessonsGap > 1)
                        evaluatedValue_ += lessonsGap * 2;

                    prevLesson = lesson;
                }
            }
        }

        return evaluatedValue_;
    }

    void Crossover(ScheduleIndividual& other, std::size_t requestIndex)
    {
        assert(classrooms_.size() == lessons_.size());
        assert(other.classrooms_.size() == other.lessons_.size());
        assert(other.lessons_.size() == lessons_.size());

        evaluated_ = false;
        other.evaluated_ = false;

        std::swap(classrooms_.at(requestIndex), other.classrooms_.at(requestIndex));
        std::swap(lessons_.at(requestIndex), other.lessons_.at(requestIndex));
    }

private:
    bool GroupsOrProfessorsIntersects(const std::vector<SubjectRequest>& requests,
                                      std::size_t currentRequest,
                                      std::size_t currentLesson) const
    {
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

    bool ClassroomsIntersects(std::size_t currentLesson,
                              std::size_t currentClassroom) const
    {
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

    void Init(const std::vector<SubjectRequest>& requests, std::size_t requestIndex)
    {
        assert(requests.size() == classrooms_.size());
        assert(requests.size() == lessons_.size());

        const auto& request = requests.at(requestIndex);
        const auto& classrooms = request.Classrooms();

        for(std::size_t dayLesson = 0; dayLesson < MAX_LESSONS_PER_DAY; ++dayLesson)
        {
            for(std::size_t day = 0; day < DAYS_IN_SCHEDULE; ++day)
            {
                if(!request.RequestedWeekDay(day))
                    continue;

                const std::size_t scheduleLesson = day * MAX_LESSONS_PER_DAY + dayLesson;
                if(!GroupsOrProfessorsIntersects(requests, requestIndex, scheduleLesson))
                {
                    for(std::size_t classroom : classrooms)
                    {
                        if(!ClassroomsIntersects(scheduleLesson, classroom))
                        {
                            classrooms_.at(requestIndex) = classroom;
                            lessons_.at(requestIndex) = scheduleLesson;
                            return;
                        }
                    }
                }
            }
        }

        assert(false);
    }

    void Change(const std::vector<SubjectRequest>& requests, std::mt19937& randGen)
    {
        assert(requests.size() == classrooms_.size());
        assert(requests.size() == lessons_.size());

        std::uniform_int_distribution<std::size_t> distrib(0, requests.size() - 1);
        const std::size_t requestIndex = distrib(randGen);
        ChooseClassroom(requests, requestIndex, randGen);
        ChooseLesson(requests, requestIndex, randGen);
    }

private:
    void ChooseClassroom(const std::vector<SubjectRequest>& requests, std::size_t requestIndex, std::mt19937& randGen)
    {
        const auto& request = requests.at(requestIndex);
        const auto& classrooms = request.Classrooms();

        std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
        std::size_t scheduleClassroom = classrooms.at(classroomDistrib(randGen));

        std::size_t chooseClassroomTry = 0;
        while(chooseClassroomTry < classrooms.size() * 2 && ClassroomsIntersects(lessons_.at(requestIndex), scheduleClassroom))
        {
            scheduleClassroom = classrooms.at(classroomDistrib(randGen));
            ++chooseClassroomTry;
        }

        if(chooseClassroomTry < classrooms.size() * 2)
            classrooms_.at(requestIndex) = scheduleClassroom;
    }

    void ChooseLesson(const std::vector<SubjectRequest>& requests, std::size_t requestIndex, std::mt19937& randGen)
    {
        std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);
        std::size_t scheduleLesson = lessonsDistrib(randGen);

        const auto& request = requests.at(requestIndex);

        std::size_t chooseLessonTry = 0;
        while(chooseLessonTry < MAX_LESSONS_COUNT * 2 &&
              (!request.RequestedWeekDay(scheduleLesson / MAX_LESSONS_PER_DAY) ||
               ClassroomsIntersects(scheduleLesson, classrooms_.at(requestIndex)) ||
               GroupsOrProfessorsIntersects(requests, requestIndex, scheduleLesson)))
        {
            scheduleLesson = lessonsDistrib(randGen);
            ++chooseLessonTry;
        }

        if(chooseLessonTry < MAX_LESSONS_COUNT * 2)
            lessons_.at(requestIndex) = scheduleLesson;
    }

private:
    mutable bool evaluated_;
    mutable std::size_t evaluatedValue_;
    std::vector<std::size_t> classrooms_;
    std::vector<std::size_t> lessons_;
};

struct ScheduleGAStatistics
{
    std::chrono::milliseconds Time;
    std::size_t Iterations;
};

class ScheduleGA
{
public:
    explicit ScheduleGA(std::vector<ScheduleIndividual> individuals)
            : iterationsCount_(1000)
            , selectionCount_(125)
            , crossoverCount_(25)
            , mutationChance_(45)
            , individuals_(std::move(individuals))
    { }

    ScheduleGA& IterationsCount(std::size_t iterations) { iterationsCount_ = iterations; return *this; }
    [[nodiscard]] std::size_t IterationsCount() const { return iterationsCount_; }

    ScheduleGA& SelectionCount(std::size_t selectionCount) { selectionCount_ = selectionCount; return *this; }
    [[nodiscard]] std::size_t SelectionCount() const { return selectionCount_; }

    ScheduleGA& CrossoverCount(std::size_t crossoverCount) { crossoverCount_ = crossoverCount; return *this; }
    [[nodiscard]] std::size_t CrossoverCount() const { return crossoverCount_; }

    ScheduleGA& MutationChance(std::size_t mutationChance)
    {
        assert(mutationChance >= 0 && mutationChance <= 100);
        mutationChance_ = mutationChance;
        return *this;
    }
    [[nodiscard]] std::size_t MutationChance() const { return mutationChance_; }

    [[nodiscard]] const ScheduleIndividual& Best() const { return individuals_.front(); }

    ScheduleGAStatistics Start(const std::vector<SubjectRequest>& requests)
    {
        assert(selectionCount_ < individuals_.size());
        assert(crossoverCount_ < selectionCount_);

        std::random_device randomDevice;
        std::mt19937 randGen(randomDevice());

        std::uniform_int_distribution<std::size_t> selectionBestDist(0, SelectionCount() - 1);
        std::uniform_int_distribution<std::size_t> selectionDist(0, individuals_.size() - 1);
        std::uniform_int_distribution<std::size_t> requestsDist(0, requests.size() - 1);

        auto individualLess = [&](const ScheduleIndividual& lhs, const ScheduleIndividual& rhs) {
            return lhs.Evaluate(requests) < rhs.Evaluate(requests);
        };

        const auto beginTime = std::chrono::steady_clock::now();

        ScheduleGAStatistics result = {};
        std::size_t prevEvaluated = std::numeric_limits<std::size_t>::max();
        for(std::size_t iteration = 0; iteration < IterationsCount(); ++iteration)
        {
            // mutate
            std::for_each(std::execution::par, individuals_.begin(), individuals_.end(), [&](ScheduleIndividual& individual)
            {
                std::random_device rd;
                std::mt19937 rg(rd());
                std::uniform_int_distribution<std::size_t> mutateDistrib(0, 100);
                if(mutateDistrib(rg) <= MutationChance())
                {
                    individual.Mutate(requests, rg);
                    individual.Evaluate(requests);
                }
            });

            // select best
            std::partial_sort(individuals_.begin(), individuals_.begin() + SelectionCount(), individuals_.end(), individualLess);
            const std::size_t currentBestEvaluated = individuals_.front().Evaluate(requests);

            std::cout << "Iteration: " << iteration << "; Best: " << currentBestEvaluated << '\n';
            if(currentBestEvaluated < prevEvaluated)
                result.Iterations = iteration;

            prevEvaluated = currentBestEvaluated;

            if(currentBestEvaluated == 0)
                break;

            // crossover
            for(std::size_t i = 0; i < CrossoverCount(); ++i)
            {
                auto& firstInd = individuals_.at(selectionBestDist(randGen));
                auto& secondInd = individuals_.at(selectionDist(randGen));

                firstInd.Crossover(secondInd, requestsDist(randGen));
                firstInd.Evaluate(requests);
                secondInd.Evaluate(requests);
            }

            // natural selection
            std::nth_element(individuals_.begin(), individuals_.end() - SelectionCount(), individuals_.end(), individualLess);
            std::sort(individuals_.end() - SelectionCount(), individuals_.end(), individualLess);
            std::copy_n(individuals_.begin(), SelectionCount(), individuals_.end() - SelectionCount());
        }

        std::sort(individuals_.begin(), individuals_.end(), individualLess);
        result.Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime);
        return result;
    }

private:
    std::size_t iterationsCount_;
    std::size_t selectionCount_;
    std::size_t crossoverCount_;
    std::size_t mutationChance_;
    std::vector<ScheduleIndividual> individuals_;
};


void Print(const ScheduleIndividual& individ, const std::vector<SubjectRequest>& requests);


class GAScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;
};
