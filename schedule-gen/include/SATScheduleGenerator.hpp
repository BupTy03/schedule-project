#pragma once
#include "ScheduleGenerator.hpp"


struct LessonsMatrixItemAddress
{
    explicit LessonsMatrixItemAddress(std::size_t day,
                                      std::size_t group,
                                      std::size_t professor,
                                      std::size_t lesson,
                                      std::size_t classroom,
                                      std::size_t subject)
        : Day(day)
        , Group(group)
        , Professor(professor)
        , Lesson(lesson)
        , Classroom(classroom)
        , Subject(subject)
    {
    }

    friend bool operator==(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day == rhs.Day &&
            lhs.Group == rhs.Group &&
            lhs.Professor == rhs.Professor &&
            lhs.Lesson == rhs.Lesson &&
            lhs.Classroom == rhs.Classroom &&
            lhs.Subject == rhs.Subject;
    }

    friend bool operator!=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day < rhs.Day ||
                (lhs.Day == rhs.Day && lhs.Group < rhs.Group) ||
                (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor < rhs.Professor) ||
                (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson < rhs.Lesson) ||
                (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom < rhs.Classroom) ||
                (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom == rhs.Classroom && lhs.Subject < rhs.Subject);
    }

    friend bool operator>(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs < rhs);
    }


    std::size_t Day;
    std::size_t Group;
    std::size_t Professor;
    std::size_t Lesson;
    std::size_t Classroom;
    std::size_t Subject;
};



class SATScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;
};
