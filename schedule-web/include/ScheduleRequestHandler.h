#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"
#include "ScheduleGenerator.hpp"

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

void to_json(nlohmann::json& j, const OverlappedClassroom& overlappedClassroom);
void to_json(nlohmann::json& j, const OverlappedProfessor& overlappedProfessor);
void to_json(nlohmann::json& j, const OverlappedGroups& overlappedGroups);
void to_json(nlohmann::json& j, const ViolatedWeekdayRequest& violatedWeekdayRequest);
void to_json(nlohmann::json& j, const CheckScheduleResult& checkScheduleResult);

void from_json(const nlohmann::json& j, ScheduleItem& scheduleItem);
ScheduleResult ParseScheduleResult(const nlohmann::json& j, const ScheduleData& data);


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


class MakeScheduleRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    explicit MakeScheduleRequestHandler(std::unique_ptr<ScheduleGenerator> generator);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;

private:
    std::unique_ptr<ScheduleGenerator> generator_;
};

class CheckScheduleRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};

class ScheduleRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    explicit ScheduleRequestHandlerFactory(const std::map<std::string, ScheduleGenOption>* pOptions);
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest&) override;

private:
    const std::map<std::string, ScheduleGenOption>* pOptions_;
};
