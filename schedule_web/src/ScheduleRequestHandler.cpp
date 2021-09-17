#include "ScheduleRequestHandler.h"
#include "ScheduleDataSerialization.h"
#include "ScheduleServer.h"
#include "ScheduleValidation.h"

#include <Poco/URI.h>
#include <spdlog/spdlog.h>
#include <cassert>


using namespace Poco;
using namespace Poco::Net;


MakeScheduleRequestHandler::MakeScheduleRequestHandler(ScheduleGA generator, std::shared_ptr<spdlog::logger> logger)
    : logger_{std::move(logger)}
    , generator_{std::move(generator)}
{
    assert(logger_ != nullptr);
}

void MakeScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                               Poco::Net::HTTPServerResponse& response)
{
    nlohmann::json jsonResponse;
    try
    {
        nlohmann::json jsonRequest;
        request.stream() >> jsonRequest;

        logger_->info("Start generate schedule...");
        jsonResponse = Generate(generator_, jsonRequest);
        logger_->info(
            "Schedule done: requests: {}, responses: {}", jsonRequest.size(), jsonResponse.size());

        response.setStatus(HTTPResponse::HTTP_OK);
    }
    catch(std::exception& e)
    {
        jsonResponse = {{"error", e.what()}};
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
    }

    response.setContentType("text/json");
    response.send() << jsonResponse.dump(4) << std::flush;
}


void CheckScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                Poco::Net::HTTPServerResponse& response)
{
    nlohmann::json jsonResponse;
    try
    {
        nlohmann::json jsonRequest;
        request.stream() >> jsonRequest;

        jsonResponse = CheckSchedule(jsonRequest, jsonRequest.at("placed_lessons"));
        response.setStatus(HTTPResponse::HTTP_OK);
    }
    catch(std::exception& e)
    {
        jsonResponse = {{"error", e.what()}};
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
    }

    response.setContentType("text/json");
    response.send() << jsonResponse.dump(4) << std::flush;
}


ScheduleRequestHandlerFactory::ScheduleRequestHandlerFactory(ScheduleGA generator, std::shared_ptr<spdlog::logger> logger)
    : logger_{std::move(logger)}
    , generator_{std::move(generator)}
{
    assert(logger_ != nullptr);
}

Poco::Net::HTTPRequestHandler*
    ScheduleRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    if(!request.serverAddress().host().isLoopback())
        return nullptr;

    const URI uri{request.getURI()};
    if(uri.getPath() == "/makeSchedule")
        return new MakeScheduleRequestHandler(generator_, logger_);
    else if(uri.getPath() == "/checkSchedule")
        return new CheckScheduleRequestHandler;
    else
        return nullptr;
}
