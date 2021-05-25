#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"
#include "ScheduleCommon.hpp"
#include <nlohmann/json.hpp>
#include <vector>


template<typename T>
std::vector<T> Merge(const std::vector<T>& lhs,
                     const std::vector<T>& rhs)
{
    assert(std::is_sorted(lhs.begin(), lhs.end()));
    assert(std::is_sorted(rhs.begin(), rhs.end()));

    std::vector<T> tmp;
    std::merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::back_inserter(tmp));
    return tmp;
}

std::vector<std::size_t> ParseIDsSet(const nlohmann::json& arr);
void InsertUniqueOrdered(std::vector<std::size_t>& vec, std::size_t value);

void from_json(const nlohmann::json& j, WeekDays& weekDays);
void from_json(const nlohmann::json& j, LessonAddress& lessonAddress);
void from_json(const nlohmann::json& j, SubjectWithAddress& subjectWithAddress);
void from_json(const nlohmann::json& j, SubjectRequest& subjectRequest);
void from_json(const nlohmann::json& j, std::vector<ClassroomAddress>& classrooms);
void from_json(const nlohmann::json& j, ScheduleData& scheduleData);
void from_json(const nlohmann::json& j, ScheduleItem& scheduleItem);
void from_json(const nlohmann::json& j, ScheduleResult& scheduleResult);

void to_json(nlohmann::json& j, const OverlappedClassroom& overlappedClassroom);
void to_json(nlohmann::json& j, const OverlappedProfessor& overlappedProfessor);
void to_json(nlohmann::json& j, const OverlappedGroups& overlappedGroups);
void to_json(nlohmann::json& j, const ViolatedWeekdayRequest& violatedWeekdayRequest);
void to_json(nlohmann::json& j, const CheckScheduleResult& checkScheduleResult);
void to_json(nlohmann::json& j, const ScheduleItem& scheduleItem);
void to_json(nlohmann::json& j, const ScheduleResult& scheduleResult);
