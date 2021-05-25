#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"
#include "ScheduleGenerator.hpp"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>


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
