#include "scheduleapplicationdatamodel.hpp"


std::size_t ScheduleApplicationDataModel::AddDiscipline(const Discipline& discipline)
{
    const auto [it, success] = disciplines_.emplace(disciplineIdGenerator_.NextID(), discipline);
    if(success)
        return it->first;

    return 0;
}

std::size_t ScheduleApplicationDataModel::AddGroup(const QString& group)
{
    if(std::any_of(groups_.begin(), groups_.end(), [&](auto&& p) {return p.second == group;}))
        return 0;

    const auto [it, success] = groups_.emplace(groupIdGenerator_.NextID(), group);
    return success ? it->first : 0;
}

std::size_t ScheduleApplicationDataModel::AddProfessor(const QString& professor)
{
    if(std::any_of(professors_.begin(), professors_.end(), [&](auto&& p) {return p.second == professor;}))
        return 0;

    const auto [it, success] = professors_.emplace(professorIdGenerator_.NextID(), professor);
    return success ? it->first : 0;
}

std::size_t ScheduleApplicationDataModel::AddClassroom(const QString& classroom)
{
    if(std::any_of(classrooms_.begin(), classrooms_.end(), [&](auto&& p) {return p.second == classroom;}))
        return 0;

    const auto [it, success] = classrooms_.emplace(classesIdGenerator_.NextID(), classroom);
    return success ? it->first : 0;
}

const Discipline* ScheduleApplicationDataModel::FindDiscipline(std::size_t id) const
{
    auto it = disciplines_.find(id);
    if(it == disciplines_.end())
        return nullptr;

    return &it->second;
}

const QString* ScheduleApplicationDataModel::FindGroup(std::size_t id) const
{
    auto it = groups_.find(id);
    if(it == groups_.end())
        return nullptr;

    return &it->second;
}

const QString* ScheduleApplicationDataModel::FindProfessor(std::size_t id) const
{
    auto it = professors_.find(id);
    if(it == professors_.end())
        return nullptr;

    return &it->second;
}

const QString* ScheduleApplicationDataModel::FindClassroom(std::size_t id) const
{
    auto it = classrooms_.find(id);
    if(it == classrooms_.end())
        return nullptr;

    return &it->second;
}

void ScheduleApplicationDataModel::RemoveDiscipline(std::size_t id)
{
    disciplines_.erase(id);
}

void ScheduleApplicationDataModel::RemoveGroup(std::size_t id)
{
    groups_.erase(id);
}

void ScheduleApplicationDataModel::RemoveProfessor(std::size_t id)
{
    professors_.erase(id);
}

void ScheduleApplicationDataModel::RemoveClassroom(std::size_t id)
{
    classrooms_.erase(id);
}

const Discipline& ScheduleApplicationDataModel::DisciplineAt(std::size_t index) {
    if(index >= disciplines_.size())
        throw std::out_of_range("index is out of range");

    return std::next(disciplines_.begin(), index)->second;
}

const QString& ScheduleApplicationDataModel::GroupAt(std::size_t index) {
    if(index >= groups_.size())
        throw std::out_of_range("index is out of range");

    return std::next(groups_.begin(), index)->second;
}

const QString& ScheduleApplicationDataModel::ProfessorAt(std::size_t index) {
    if(index >= professors_.size())
        throw std::out_of_range("index is out of range");

    return std::next(professors_.begin(), index)->second;
}

const QString& ScheduleApplicationDataModel::ClassroomAt(std::size_t index) {
    if(index >= classrooms_.size())
        throw std::out_of_range("index is out of range");

    return std::next(classrooms_.begin(), index)->second;
}
