#pragma once
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include <vector>
#include <random>


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


void InsertMorningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex);
void InsertEveningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex);
ScheduleChromosomes InitializeChromosomes(const ScheduleData& data);

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r);

void Crossover(ScheduleChromosomes& first, ScheduleChromosomes& second, std::size_t r);

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes, const ScheduleData& scheduleData);

ScheduleResult MakeScheduleResult(const ScheduleChromosomes& chromosomes, const ScheduleData& scheduleData);


bool CanChangeMorningLesson(const ScheduleChromosomes& chromosomes,
                            const ScheduleData& data,
                            std::size_t requestIndex,
                            std::size_t lesson);

bool CanChangeEveningLesson(const ScheduleChromosomes& chromosomes,
                            const ScheduleData& data,
                            std::size_t requestIndex,
                            std::size_t lesson);


template<class RandomGenerator>
std::size_t ChangeMorningLesson(const ScheduleChromosomes& chromosomes,
                                const ScheduleData& data,
                                std::size_t requestIndex,
                                RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    assert(!request.IsEveningClass());
    assert(!data.SubjectRequestHasLockedLesson(request));

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);

    std::size_t lesson = lessonsDistrib(randomGenerator);
    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT && 
          !CanChangeMorningLesson(chromosomes, data, requestIndex, lesson))
    {
        lesson = lessonsDistrib(randomGenerator);
        ++chooseLessonTry;
    }

    return (chooseLessonTry < MAX_LESSONS_COUNT) ? lesson : chromosomes.Lesson(requestIndex);
}

template<class RandomGenerator>
std::size_t ChangeEveningLesson(const ScheduleChromosomes& chromosomes,
                                const ScheduleData& data,
                                std::size_t requestIndex,
                                RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    assert(request.IsEveningClass());
    assert(!data.SubjectRequestHasLockedLesson(request));

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, MAX_LESSONS_COUNT - 1);

    std::size_t lesson = lessonsDistrib(randomGenerator);
    std::size_t chooseLessonTry = 0;
    while(chooseLessonTry < MAX_LESSONS_COUNT &&
          !CanChangeEveningLesson(chromosomes, data, requestIndex, lesson))
    {
        lesson = lessonsDistrib(randomGenerator);
        ++chooseLessonTry;
    }

    return (chooseLessonTry < MAX_LESSONS_COUNT) ? lesson : chromosomes.Lesson(requestIndex);
}

template<class RandomGenerator>
std::size_t ChangeLesson(const ScheduleChromosomes& chromosomes,
                         const ScheduleData& data,
                         std::size_t requestIndex,
                         RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    return request.IsEveningClass()
        ? ChangeEveningLesson(chromosomes, data, requestIndex, randomGenerator)
        : ChangeMorningLesson(chromosomes, data, requestIndex, randomGenerator);
}

template<class RandomGenerator>
ClassroomAddress ChangeClassroom(const ScheduleChromosomes& chromosomes,
                                 const ScheduleData& data,
                                 std::size_t requestIndex,
                                 RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const auto& classrooms = request.Classrooms();
    assert(!classrooms.empty());

    std::uniform_int_distribution<std::size_t> classroomDistrib(0, classrooms.size() - 1);
    auto scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator));

    std::size_t chooseClassroomTry = 0;
    while(chooseClassroomTry < classrooms.size() &&
          chromosomes.ClassroomsIntersects(chromosomes.Lesson(requestIndex), scheduleClassroom))
    {
        scheduleClassroom = classrooms.at(classroomDistrib(randomGenerator));
        ++chooseClassroomTry;
    }

    return (chooseClassroomTry < classrooms.size()) ? scheduleClassroom : chromosomes.Classroom(requestIndex);
}

template<class RandomGenerator>
std::pair<std::size_t, ClassroomAddress> ChangeChromosome(const ScheduleChromosomes& chromosomes,
                                                          const ScheduleData& data,
                                                          std::size_t requestIndex,
                                                          RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const bool canChangeLesson = !data.SubjectRequestHasLockedLesson(request);
    const bool canChangeClassroom = !std::empty(request.Classrooms());
    if(!(canChangeLesson || canChangeClassroom))
        return {chromosomes.Lesson(requestIndex), chromosomes.Classroom(requestIndex)};

    if(canChangeLesson && canChangeClassroom)
    {
        std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
        if(headsOrTails(randomGenerator))
        {
            return {
                ChangeLesson(chromosomes, data, requestIndex, randomGenerator), 
                chromosomes.Classroom(requestIndex)
            };
        }
        else
        {
            return {
                chromosomes.Lesson(requestIndex),
                ChangeClassroom(chromosomes, data, requestIndex, randomGenerator)
            };
        }
    }
    
    if(canChangeLesson)
    {
        return {
            ChangeLesson(chromosomes, data, requestIndex, randomGenerator), 
            chromosomes.Classroom(requestIndex)
        };
    }
    else
    {
        return {
            chromosomes.Lesson(requestIndex),
            ChangeClassroom(chromosomes, data, requestIndex, randomGenerator)
        };
    }
}

template<class RandomGenerator>
bool Mutate(ScheduleChromosomes& chromosomes, const ScheduleData& data, RandomGenerator& randomGenerator)
{
    std::uniform_int_distribution<std::size_t> requestsDistrib(0, data.SubjectRequests().size() - 1);
    const auto requestIndex = requestsDistrib(randomGenerator);

    const auto oldLesson = chromosomes.Lesson(requestIndex);
    const auto oldClassrooms = chromosomes.Classroom(requestIndex);

    std::tie(chromosomes.Lesson(requestIndex), chromosomes.Classroom(requestIndex)) = 
        ChangeChromosome(chromosomes, data, requestIndex, randomGenerator);

    return !(oldLesson == chromosomes.Lesson(requestIndex) && oldClassrooms == chromosomes.Classroom(requestIndex));
}
