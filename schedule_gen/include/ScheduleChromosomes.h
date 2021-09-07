#pragma once
#include "ScheduleResult.h"
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


void InsertMorningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex);
void InsertEveningRequest(ScheduleChromosomes& chromosomes, const ScheduleData& data, std::size_t requestIndex);
ScheduleChromosomes InitChromosomes(const ScheduleData& data);

bool ReadyToCrossover(const ScheduleChromosomes& first,
                      const ScheduleChromosomes& second,
                      const ScheduleData& data,
                      std::size_t r);

void Crossover(ScheduleChromosomes& first,
               ScheduleChromosomes& second,
               std::size_t r);

std::size_t Evaluate(const ScheduleChromosomes& scheduleChromosomes,
                     const ScheduleData& scheduleData);

ScheduleResult MakeScheduleResult(const ScheduleChromosomes& chromosomes,
                                const ScheduleData& scheduleData);
