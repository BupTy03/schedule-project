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

void Crossover(ScheduleChromosomes& first, ScheduleChromosomes& second, std::size_t r);

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes,
                     const ScheduleData& scheduleData);

ScheduleResult MakeScheduleResult(const ScheduleChromosomes& chromosomes,
                                  const ScheduleData& scheduleData);


template<class RandomGenerator>
std::size_t ChangeLesson(const ScheduleChromosomes& chromosomes,
                         const ScheduleData& data,
                         std::size_t requestIndex,
                         RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const auto& lessons = request.Lessons();
    assert(lessons.size() > 1);

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, lessons.size() - 1);
    for(std::size_t tryNum = 0; tryNum < lessons.size(); ++tryNum)
    {
        const std::size_t lesson = lessons.at(lessonsDistrib(randomGenerator));
        if(!chromosomes.GroupsOrProfessorsOrClassroomsIntersects(data, requestIndex, lesson))
            return lesson;
    }

    return chromosomes.Lesson(requestIndex);
}

template<class RandomGenerator>
ClassroomAddress ChangeClassroom(const ScheduleChromosomes& chromosomes,
                                 const ScheduleData& data,
                                 std::size_t requestIndex,
                                 RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const auto& classrooms = request.Classrooms();
    assert(classrooms.size() > 1);

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    for(std::size_t tryNum = 0; tryNum < classrooms.size(); ++tryNum)
    {
        const auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator));
        if(!chromosomes.ClassroomsIntersects(chromosomes.Lesson(requestIndex), scheduleClassroom))
            return scheduleClassroom;
    }

    return chromosomes.Classroom(requestIndex);
}

template<class RandomGenerator>
std::pair<std::size_t, ClassroomAddress> ChangeChromosome(const ScheduleChromosomes& chromosomes,
                                                          const ScheduleData& data,
                                                          std::size_t requestIndex,
                                                          RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const bool canChangeLesson = request.Lessons().size() > 1;
    const bool canChangeClassroom = request.Classrooms().size() > 1;
    if(!(canChangeLesson || canChangeClassroom))
        return {chromosomes.Lesson(requestIndex), chromosomes.Classroom(requestIndex)};

    if(canChangeLesson && canChangeClassroom)
    {
        std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
        if(headsOrTails(randomGenerator))
        {
            return {ChangeLesson(chromosomes, data, requestIndex, randomGenerator),
                    chromosomes.Classroom(requestIndex)};
        }
        else
        {
            return {chromosomes.Lesson(requestIndex),
                    ChangeClassroom(chromosomes, data, requestIndex, randomGenerator)};
        }
    }

    if(canChangeLesson)
    {
        return {ChangeLesson(chromosomes, data, requestIndex, randomGenerator),
                chromosomes.Classroom(requestIndex)};
    }
    else
    {
        return {chromosomes.Lesson(requestIndex),
                ChangeClassroom(chromosomes, data, requestIndex, randomGenerator)};
    }
}

template<class RandomGenerator>
bool Mutate(ScheduleChromosomes& chromosomes,
            const ScheduleData& data,
            RandomGenerator& randomGenerator)
{
    std::uniform_int_distribution<std::size_t> requestsDistrib(0,
                                                               data.SubjectRequests().size() - 1);
    const auto requestIndex = requestsDistrib(randomGenerator);

    const auto oldLesson = chromosomes.Lesson(requestIndex);
    const auto oldClassrooms = chromosomes.Classroom(requestIndex);

    std::tie(chromosomes.Lesson(requestIndex), chromosomes.Classroom(requestIndex)) =
        ChangeChromosome(chromosomes, data, requestIndex, randomGenerator);

    return !(oldLesson == chromosomes.Lesson(requestIndex)
             && oldClassrooms == chromosomes.Classroom(requestIndex));
}
