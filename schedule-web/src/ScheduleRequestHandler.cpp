#include "ScheduleRequestHandler.h"
#include "ScheduleServer.h"
#include "GAScheduleGenerator.hpp"
#include "ScheduleDataSerialization.h"
#include "Poco/URI.h"
#include "spdlog/spdlog.h"
#include <cassert>


using namespace Poco;
using namespace Poco::Net;


MakeScheduleRequestHandler::MakeScheduleRequestHandler(std::unique_ptr<ScheduleGenerator> generator)
    : generator_(std::move(generator))
{
    assert(generator_ != nullptr);
}

void MakeScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                         Poco::Net::HTTPServerResponse& response)
{
    std::istream& in = request.stream();
    std::vector<char> requestBody(request.getContentLength(), '\0');
    in.read(requestBody.data(), requestBody.size());

    nlohmann::json jsonResponse;
    try {
        const auto jsonRequest = nlohmann::json::parse(requestBody);

        auto logger = spdlog::get("server");
        logger->info("Start generate schedule...");
        jsonResponse = generator_->Generate(jsonRequest);
        logger->info("Schedule done: requests: {}, responses: {}", jsonRequest["subject_requests"].size(), jsonResponse.size());

        response.setStatus(HTTPResponse::HTTP_OK);
    }
    catch(std::exception& e)
    {
        jsonResponse = {{"error", e.what()}};
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
    }

    response.setContentType("text/json");
    std::ostream& out = response.send();
    out << jsonResponse.dump(4);
    out.flush();
}


void CheckScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                Poco::Net::HTTPServerResponse& response)
{
    std::istream& in = request.stream();
    std::vector<char> requestBody(request.getContentLength(), '\0');
    in.read(requestBody.data(), requestBody.size());

    nlohmann::json jsonResponse;
    try {
        const auto jsonRequest = nlohmann::json::parse(requestBody);
        const ScheduleData scheduleData = jsonRequest;
        jsonResponse = CheckSchedule(scheduleData, jsonRequest.at("placed_lessons"));
        response.setStatus(HTTPResponse::HTTP_OK);
    }
    catch(std::exception& e)
    {
        jsonResponse = {{"error", e.what()}};
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
    }

    response.setContentType("text/json");
    std::ostream& out = response.send();
    out << jsonResponse.dump(4);
    out.flush();
}


ScheduleRequestHandlerFactory::ScheduleRequestHandlerFactory(std::unique_ptr<ScheduleGenerator> generator)
    : generator_(std::move(generator))
{
    assert(generator_ != nullptr);
}

Poco::Net::HTTPRequestHandler* ScheduleRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    if(!request.serverAddress().host().isLoopback())
        return nullptr;

    URI uri(request.getURI());
    if(uri.getPath() == "/makeSchedule")
    {
        return new MakeScheduleRequestHandler(generator_->Clone());
    }
    else if(uri.getPath() == "/checkSchedule")
    {
        return new CheckScheduleRequestHandler;
    }
    else
    {
        return nullptr;
    }
}
