#include "ScheduleRequestHandler.h"
#include "ScheduleServer.h"

#include <nlohmann/json.hpp>

#include <Poco/URI.h>
#include <Poco/StringTokenizer.h>


using namespace Poco;
using namespace Poco::Net;


nlohmann::json RequireField(const nlohmann::json& object, const std::string& field)
{
    auto it = object.find(field);
    if(it == object.end())
        throw std::invalid_argument("Field '" + field + "' is not found");

    return *it;
}

LessonAddress ParseLessonAddress(const nlohmann::json& lessonAddress)
{
    return LessonAddress(RequireField(lessonAddress, "group").get<std::size_t>(),
                         RequireField(lessonAddress, "lesson").get<std::size_t>());
}

SubjectWithAddress ParseLockedLesson(const nlohmann::json& lockedLesson)
{
    return SubjectWithAddress(RequireField(lockedLesson, "subject_request").get<std::size_t>(),
                              ParseLessonAddress(RequireField(lockedLesson, "address")));
}

WeekDays ParseWeekDays(const nlohmann::json& weekDays)
{
    if(!weekDays.is_array())
        throw std::invalid_argument("Json array expected");

    WeekDays result = WeekDays::emptyWeek();
    for(auto&& wd : weekDays)
    {
        const auto value = wd.get<int>();
        if(value < 0 || value > 5)
            throw std::invalid_argument("Week day number must be in range [0, 5]");

        result.insert(static_cast<WeekDay>(value % 6));
    }

    if(result == WeekDays::emptyWeek())
        return WeekDays::fullWeek();

    return result;
}

SortedSet<std::size_t> ParseIDsSet(const nlohmann::json& arr)
{
    if(!arr.is_array())
        throw std::invalid_argument("Json array expected");

    SortedSet<std::size_t> result;
    for(auto&& value : arr)
    {
        const auto v = value.get<std::int64_t>();
        if(value < 0)
            throw std::invalid_argument("ID can't be negative");

        result.insert(static_cast<std::size_t>(v));
    }

    return result;
}

SubjectRequest ParseSubjectRequest(const nlohmann::json& subjectRequest)
{
    return SubjectRequest(
        RequireField(subjectRequest, "professor").get<std::size_t>(),
        RequireField(subjectRequest, "hours_count").get<std::size_t>(),
        RequireField(subjectRequest, "complexity").get<std::size_t>(),
        ParseWeekDays(RequireField(subjectRequest, "days")),
        ParseIDsSet(RequireField(subjectRequest, "groups")),
        ParseIDsSet(RequireField(subjectRequest, "classrooms"))
        );
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

