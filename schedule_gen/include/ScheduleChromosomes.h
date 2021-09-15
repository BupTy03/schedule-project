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


template<class RandomGenerator>
void ChangeLessonsBlock(ScheduleChromosomes& chromosomes,
                        const ScheduleData& data,
                        const SubjectsBlock& block,
                        std::size_t requestIndex,
                        RandomGenerator& randomGenerator)
{
    assert(data.IndexOfSubjectRequestWithID(block.front()) == requestIndex);

    std::vector<std::size_t> blockFirstLessons;
    for(std::size_t lesson : data.SubjectRequestAtID(block.front()).Lessons())
    {
        bool matches = true;
        for(std::size_t i = 1, l = lesson; i < block.size(); ++i, ++l)
        {
            const std::size_t subjectRequestID = block[i];
            const auto& lessons = data.SubjectRequestAtID(subjectRequestID).Lessons();
            matches = std::binary_search(lessons.begin(), lessons.end(), l);
            if(!matches)
                break;
        }

        if(matches)
            blockFirstLessons.emplace_back(lesson);
    }

    if(blockFirstLessons.size() < 2)
        return;

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, blockFirstLessons.size() - 1);
    for(std::size_t tryNum = 0; tryNum < blockFirstLessons.size(); ++tryNum)
    {
        const std::size_t lesson = blockFirstLessons.at(lessonsDistrib(randomGenerator));
        bool flag = true;
        for(std::size_t b = 0; b < block.size(); ++b)
        {
            const std::size_t subjectRequestIndex = data.IndexOfSubjectRequestWithID(block.at(b));
            if(chromosomes.GroupsOrProfessorsOrClassroomsIntersects(
                   data, subjectRequestIndex, lesson + b))
            {
                flag = false;
                break;
            }
        }

        if(!flag)
            continue;

        for(std::size_t b = 0; b < block.size(); ++b)
        {
            const std::size_t subjectRequestIndex = data.IndexOfSubjectRequestWithID(block.at(b));
            chromosomes.Lesson(subjectRequestIndex) = lesson + b;
        }

        return;
    }
}

template<class RandomGenerator>
void ChangeLesson(ScheduleChromosomes& chromosomes,
                  const ScheduleData& data,
                  std::size_t requestIndex,
                  RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const auto& lessons = request.Lessons();
    const auto& blocks = data.Blocks();
    assert(lessons.size() > 1);

    auto blockIt =
        std::find_if(blocks.begin(),
                     blocks.end(),
                     [&](const SubjectsBlock& block) { return block.front() == request.ID(); });

    if(blockIt != blocks.end())
        return ChangeLessonsBlock(chromosomes, data, *blockIt, requestIndex, randomGenerator);

    std::uniform_int_distribution<std::size_t> lessonsDistrib(0, lessons.size() - 1);
    for(std::size_t tryNum = 0; tryNum < lessons.size(); ++tryNum)
    {
        const std::size_t lesson = lessons.at(lessonsDistrib(randomGenerator));
        if(!chromosomes.GroupsOrProfessorsOrClassroomsIntersects(data, requestIndex, lesson))
        {
            chromosomes.Lesson(requestIndex) = lesson;
            return;
        }
    }
}

template<class RandomGenerator>
void ChangeClassroom(ScheduleChromosomes& chromosomes,
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
        {
            chromosomes.Classroom(requestIndex) = scheduleClassroom;
            return;
        }
    }
}

template<class RandomGenerator>
void ChangeChromosome(ScheduleChromosomes& chromosomes,
                      const ScheduleData& data,
                      std::size_t requestIndex,
                      RandomGenerator& randomGenerator)
{
    const auto& request = data.SubjectRequests().at(requestIndex);
    const bool canChangeLesson = request.Lessons().size() > 1;
    const bool canChangeClassroom = request.Classrooms().size() > 1;
    if(!(canChangeLesson || canChangeClassroom))
        return;

    if(canChangeLesson && canChangeClassroom)
    {
        std::uniform_int_distribution<std::size_t> headsOrTails(0, 1);
        if(headsOrTails(randomGenerator))
        {
            ChangeLesson(chromosomes, data, requestIndex, randomGenerator);
        }
        else
        {
            ChangeClassroom(chromosomes, data, requestIndex, randomGenerator);
        }

        return;
    }

    if(canChangeLesson)
    {
        ChangeLesson(chromosomes, data, requestIndex, randomGenerator);
    }
    else
    {
        ChangeClassroom(chromosomes, data, requestIndex, randomGenerator);
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
    ChangeChromosome(chromosomes, data, requestIndex, randomGenerator);

    return !(oldLesson == chromosomes.Lesson(requestIndex)
             && oldClassrooms == chromosomes.Classroom(requestIndex));
}
