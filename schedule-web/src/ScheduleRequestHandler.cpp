#include "ScheduleRequestHandler.h"
#include "ScheduleServer.h"

#include <nlohmann/json.hpp>

#include <Poco/URI.h>
#include <Poco/StringTokenizer.h>


using namespace Poco;
using namespace Poco::Net;


static nlohmann::json RequireField(const nlohmann::json& object, const std::string& field)
{
    auto it = object.find(field);
    if(it == object.end())
        throw std::invalid_argument("Field '" + field + "' is not found");

    return *it;
}


static nlohmann::json ComputeResponse(const nlohmann::json& request)
{
    const int a = RequireField(request, "a").get<int>();
    const int b = RequireField(request, "b").get<int>();

    return {
            {"a", a},
            {"b", b},
            {"sum", a + b}
    };
}


void ScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                         Poco::Net::HTTPServerResponse& response)
{
    URI uri(request.getURI());
    const auto requestStr = uri.toString();
    if(requestStr.find("/plus") != 0)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        return;
    }

    std::istream& in = request.stream();
    std::vector<char> requestBody(request.getContentLength(), '\0');
    in.read(requestBody.data(), requestBody.size());

    nlohmann::json jsonResponse;
    try {
        jsonResponse = ComputeResponse(nlohmann::json::parse(requestBody));
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


Poco::Net::HTTPRequestHandler* ScheduleRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest&) {
    return new ScheduleRequestHandler();
}

