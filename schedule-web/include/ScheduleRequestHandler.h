#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>

#include <nlohmann/json.hpp>


nlohmann::json RequireField(const nlohmann::json& object, const std::string& field);
WeekDays ParseWeekDays(const nlohmann::json& weekDays);
SortedSet<std::size_t> ParseIDsSet(const nlohmann::json& arr);
SubjectRequest ParseSubjectRequest(const nlohmann::json& subjectRequest);
LessonAddress ParseLessonAddress(const nlohmann::json& lessonAddress);
SubjectWithAddress ParseLockedLesson(const nlohmann::json& lockedLesson);
ScheduleData ParseScheduleData(const nlohmann::json& scheduleData);

nlohmann::json ToJson(const ScheduleItem& scheduleItem);
nlohmann::json ToJson(const ScheduleResult& scheduleResult);

std::vector<std::size_t> Merge(const std::vector<std::size_t>& lhs,
                               const std::vector<std::size_t>& rhs);

void InsertUniqueOrdered(std::vector<std::size_t>& vec, std::size_t value);


class ScheduleRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};


class ScheduleRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest&) override;
};
