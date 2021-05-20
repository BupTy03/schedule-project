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
std::vector<std::size_t> ParseIDsSet(const nlohmann::json& arr);
std::vector<ClassroomAddress> ParseClassrooms(const nlohmann::json& arr);
SubjectRequest ParseSubjectRequest(const nlohmann::json& subjectRequest);
LessonAddress ParseLessonAddress(const nlohmann::json& lessonAddress);
SubjectWithAddress ParseLockedLesson(const nlohmann::json& lockedLesson);
ScheduleData ParseScheduleData(const nlohmann::json& scheduleData);

nlohmann::json ToJson(const ScheduleItem& scheduleItem);
nlohmann::json ToJson(const ScheduleResult& scheduleResult);


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
