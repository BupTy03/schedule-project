#pragma once
#include "scheduleimportexport.hpp"
#include <cstddef>
#include <set>
#include <map>
#include <vector>
#include <stdexcept>
#include <limits>
#include <QString>


class IDGenerator
{
public:
    explicit IDGenerator(std::size_t id = 0)
        : id_(id)
    {}

    [[nodiscard]] std::size_t NextID()
    {
        if(id_ == std::numeric_limits<std::size_t>::max())
            throw std::overflow_error("Max ID number exceeded");

        ++id_;
        return id_;
    }

private:
    std::size_t id_;
};

class ScheduleApplicationDataModel
{
public:
    std::size_t AddDiscipline(const Discipline& discipline);
    std::size_t AddGroup(const QString& group);
    std::size_t AddProfessor(const QString& professor);
    std::size_t AddClassroom(const QString& classroom);

    const Discipline& DisciplineAt(std::size_t index);
    const QString& GroupAt(std::size_t index);
    const QString& ProfessorAt(std::size_t index);
    const QString& ClassroomAt(std::size_t index);


    const Discipline* FindDiscipline(std::size_t id) const;
    const QString* FindGroup(std::size_t id) const;
    const QString* FindProfessor(std::size_t id) const;
    const QString* FindClassroom(std::size_t id) const;

    void RemoveDiscipline(std::size_t id);
    void RemoveGroup(std::size_t id);
    void RemoveProfessor(std::size_t id);
    void RemoveClassroom(std::size_t id);

private:
    IDGenerator disciplineIdGenerator_;
    IDGenerator groupIdGenerator_;
    IDGenerator professorIdGenerator_;
    IDGenerator classesIdGenerator_;
    std::map<std::size_t, Discipline> disciplines_;
    std::map<std::size_t, QString> groups_;
    std::map<std::size_t, QString> professors_;
    std::map<std::size_t, QString> classrooms_;
};

