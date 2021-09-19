#pragma once
#include "ScheduleData.h"
#include "ScheduleResult.h"

#include <random>
#include <vector>


class ScheduleData;

class ScheduleChromosomes
{
public:
    explicit ScheduleChromosomes(std::size_t count);
    explicit ScheduleChromosomes(std::vector<std::size_t> lessons,
                                 std::vector<ClassroomAddress> classrooms);

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

    std::size_t UnassignedLessonsCount() const;
    std::size_t UnassignedClassroomsCount() const;

private:
    std::vector<std::size_t> lessons_;
    std::vector<ClassroomAddress> classrooms_;
};


void InsertRequest(ScheduleChromosomes& chromosomes,
                   const ScheduleData& data,
                   std::size_t requestIndex);

void InsertBlock(ScheduleChromosomes& chromosomes,
                 const ScheduleData& data,
                 const SubjectsBlock& block);

ScheduleChromosomes InitializeChromosomes(const ScheduleData& data);

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r);

void Crossover(ScheduleChromosomes& first,
               ScheduleChromosomes& second,
               const ScheduleData& data,
               std::size_t r);

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes,
                     const ScheduleData& scheduleData);

ScheduleResult MakeScheduleResult(const ScheduleChromosomes& chromosomes,
                                  const ScheduleData& scheduleData);


class ChromosomesMutator
{
public:
    explicit ChromosomesMutator(ScheduleChromosomes& chromosomes,
                                const ScheduleData& data,
                                std::size_t requestIndex)
        : mutated_{false}
        , requestIndex_{requestIndex}
        , chromosomes_{chromosomes}
        , data_{data}
    {
    }

    bool Mutated() const { return mutated_; }

    template<class RandomGenerator>
    void ChangeLessonsBlock(const SubjectsBlock& block, RandomGenerator&& randomGenerator)
    {
        const auto& blockFirstLessons = block.Addresses();
        const auto& blockRequests = block.Requests();
        if(block.Addresses().size() <= 1 || blockRequests.front() != requestIndex_)
            return;

        std::uniform_int_distribution<std::size_t> lessonsDistrib(0, blockFirstLessons.size() - 1);
        for(std::size_t tryNum = 0; tryNum < blockFirstLessons.size(); ++tryNum)
        {
            const std::size_t lesson = blockFirstLessons.at(lessonsDistrib(randomGenerator));
            if(lesson == chromosomes_.Lesson(blockRequests.front()))
                continue;

            bool flag = true;
            for(std::size_t b = 0; b < blockRequests.size(); ++b)
            {
                const std::size_t subjectRequestIndex = blockRequests.at(b);
                if(chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(
                       data_, subjectRequestIndex, lesson + b))
                {
                    flag = false;
                    break;
                }
            }

            if(flag)
            {
                mutated_ = true;
                for(std::size_t b = 0; b < blockRequests.size(); ++b)
                {
                    const std::size_t subjectRequestIndex = blockRequests.at(b);
                    chromosomes_.Lesson(subjectRequestIndex) = lesson + b;
                }

                return;
            }
        }
    }

    template<class RandomGenerator> void ChangeLesson(RandomGenerator&& randomGenerator)
    {
        const auto& request = data_.SubjectRequests().at(requestIndex_);
        const auto& lessons = request.Lessons();
        assert(lessons.size() > 1);

        auto block = data_.FindBlockByRequestIndex(requestIndex_);
        if(block)
        {
            ChangeLessonsBlock(*block, randomGenerator);
            return;
        }

        std::uniform_int_distribution<std::size_t> lessonsDistrib(0, lessons.size() - 1);
        for(std::size_t tryNum = 0; tryNum < lessons.size(); ++tryNum)
        {
            const std::size_t lesson = lessons.at(lessonsDistrib(randomGenerator));
            if(!(lesson == chromosomes_.Lesson(requestIndex_)
                 || chromosomes_.GroupsOrProfessorsOrClassroomsIntersects(
                     data_, requestIndex_, lesson)))
            {
                mutated_ = true;
                chromosomes_.Lesson(requestIndex_) = lesson;
                return;
            }
        }
    }

    template<class RandomGenerator> void ChangeClassroom(RandomGenerator&& randomGenerator)
    {
        const auto& request = data_.SubjectRequests().at(requestIndex_);
        const auto& classrooms = request.Classrooms();
        assert(classrooms.size() > 1);

        std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
        for(std::size_t tryNum = 0; tryNum < classrooms.size(); ++tryNum)
        {
            const auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator));
            if(!(chromosomes_.Classroom(requestIndex_) == scheduleClassroom
                 || chromosomes_.ClassroomsIntersects(chromosomes_.Lesson(requestIndex_),
                                                      scheduleClassroom)))
            {
                mutated_ = true;
                chromosomes_.Classroom(requestIndex_) = scheduleClassroom;
                return;
            }
        }
    }

private:
    bool mutated_;
    std::size_t requestIndex_;
    ScheduleChromosomes& chromosomes_;
    const ScheduleData& data_;
};


template<class RandomGenerator>
bool Mutate(ScheduleChromosomes& chromosomes,
            const ScheduleData& data,
            RandomGenerator& randomGenerator)
{
    std::uniform_int_distribution<std::size_t> requestsDistrib(0,
                                                               data.SubjectRequests().size() - 1);
    const auto requestIndex = requestsDistrib(randomGenerator);
    const auto& request = data.SubjectRequests().at(requestIndex);
    const bool canChangeLesson = request.Lessons().size() > 1;
    const bool canChangeClassroom = request.Classrooms().size() > 1;
    if(!(canChangeLesson || canChangeClassroom))
        return false;

    ChromosomesMutator mutator{chromosomes, data, requestIndex};
    if(canChangeLesson && canChangeClassroom)
    {
        std::uniform_int_distribution<std::size_t> headsOrTails{0, 1};
        if(headsOrTails(randomGenerator))
            mutator.ChangeLesson(randomGenerator);
        else
            mutator.ChangeClassroom(randomGenerator);
    }
    else if(canChangeLesson)
    {
        mutator.ChangeLesson(randomGenerator);
    }
    else
    {
        mutator.ChangeClassroom(randomGenerator);
    }

    return mutator.Mutated();
}
