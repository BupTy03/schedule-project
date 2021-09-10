#pragma once
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include "ScheduleCommon.h"
#include "ScheduleValidation.h"
#include <nlohmann/json.hpp>
#include <vector>


std::vector<std::size_t> ParseIDsSet(const nlohmann::json& arr);
std::vector<std::size_t> ParseLessonsSet(const nlohmann::json& arr);

void from_json(const nlohmann::json& j, SubjectRequest& subjectRequest);
void from_json(const nlohmann::json& j, std::vector<ClassroomAddress>& classrooms);
void from_json(const nlohmann::json& j, ScheduleData& scheduleData);
void from_json(const nlohmann::json& j, ScheduleItem& scheduleItem);
void from_json(const nlohmann::json& j, ScheduleResult& scheduleResult);

void to_json(nlohmann::json& j, const OverlappedClassroom& overlappedClassroom);
void to_json(nlohmann::json& j, const OverlappedProfessor& overlappedProfessor);
void to_json(nlohmann::json& j, const OverlappedGroups& overlappedGroups);
void to_json(nlohmann::json& j, const CheckScheduleResult& checkScheduleResult);
void to_json(nlohmann::json& j, const ScheduleItem& scheduleItem);
void to_json(nlohmann::json& j, const ScheduleResult& scheduleResult);
