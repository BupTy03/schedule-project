#pragma once
#include "ScheduleData.h"
#include "ScheduleGA.h"
#include "ScheduleResult.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include <spdlog/spdlog.h>


class MakeScheduleRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    explicit MakeScheduleRequestHandler(ScheduleGA generator, std::shared_ptr<spdlog::logger> logger);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;

private:
    std::shared_ptr<spdlog::logger> logger_;
    ScheduleGA generator_;
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
    explicit ScheduleRequestHandlerFactory(ScheduleGA generator, std::shared_ptr<spdlog::logger> logger);
    Poco::Net::HTTPRequestHandler*
        createRequestHandler(const Poco::Net::HTTPServerRequest&) override;

private:
    std::shared_ptr<spdlog::logger> logger_;
    ScheduleGA generator_;
};
