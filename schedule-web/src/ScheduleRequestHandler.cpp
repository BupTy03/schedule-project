#include "ScheduleRequestHandler.h"
#include "ScheduleServer.h"
#include "GAScheduleGenerator.hpp"

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

    return result;
}

std::vector<std::size_t> ParseIDsSet(const nlohmann::json& arr)
{
    if(!arr.is_array())
        throw std::invalid_argument("Json array expected");

    std::vector<std::size_t> result;
    result.reserve(arr.size());
    for(auto&& value : arr)
    {
        const auto v = value.get<std::int64_t>();
        if(value < 0)
            throw std::invalid_argument("ID can't be negative");

        result.emplace_back(static_cast<std::size_t>(v));
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    result.shrink_to_fit();
    return result;
}

std::vector<ClassroomAddress> ParseClassrooms(const nlohmann::json& arr)
{
    if(!arr.is_array())
        throw std::invalid_argument("Json array expected");

    std::vector<ClassroomAddress> result;
    for(std::size_t building = 0; building < arr.size(); ++building)
    {
        auto&& classrooms = arr.at(building);
        if(!classrooms.is_array())
            throw std::invalid_argument("Json array expected");

        for(auto&& cr : classrooms)
        {
            std::int64_t classroom = cr.get<std::int64_t>();
            if(classroom < 0)
                throw std::invalid_argument("Classroom ID must be positive number");

            result.emplace_back(building, classroom);
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    result.shrink_to_fit();
    return result;
}

SubjectRequest ParseSubjectRequest(const nlohmann::json& subjectRequest)
{
    return SubjectRequest(
        RequireField(subjectRequest, "id").get<std::size_t>(),
        RequireField(subjectRequest, "professor").get<std::size_t>(),
        RequireField(subjectRequest, "complexity").get<std::size_t>(),
        ParseWeekDays(RequireField(subjectRequest, "days")),
        ParseIDsSet(RequireField(subjectRequest, "groups")),
        ParseClassrooms(RequireField(subjectRequest, "classrooms")));
}

ScheduleData ParseScheduleData(const nlohmann::json& scheduleData)
{
    auto subjectRequests = RequireField(scheduleData, "subject_requests");
    if(!subjectRequests.is_array())
        throw std::invalid_argument("Json array expected");

    if(subjectRequests.empty())
        throw std::invalid_argument("'subject_requests' array is empty");

    std::vector<std::size_t> groups;
    std::vector<std::size_t> professors;
    std::vector<ClassroomAddress> classrooms;

    std::vector<SubjectRequest> requests;
    requests.reserve(subjectRequests.size());
    for(auto&& subjectRequest : subjectRequests)
    {
        requests.emplace_back(ParseSubjectRequest(subjectRequest));

        const SubjectRequest& request = requests.back();
        groups = Merge(groups, request.Groups());
        InsertUniqueOrdered(professors, request.Professor());
        classrooms = Merge(classrooms, request.Classrooms());
    }

    // 'locked_lessons' field is optional
    std::vector<SubjectWithAddress> locked;
    auto lockedLessonsIt = scheduleData.find("locked_lessons");
    if(lockedLessonsIt != scheduleData.end() && lockedLessonsIt->is_array())
    {
        locked.reserve(lockedLessonsIt->size());
        for(auto&& lockedLesson : *lockedLessonsIt)
            locked.emplace_back(ParseLockedLesson(lockedLesson));
    }

    return ScheduleData(std::move(groups),
                        std::move(professors),
                        std::move(classrooms),
                        std::move(requests),
                        std::move(locked));
}


void InsertUniqueOrdered(std::vector<std::size_t>& vec, std::size_t value)
{
    auto it = std::lower_bound(vec.begin(), vec.end(), value);
    if(it == vec.end() || *it != value)
        vec.emplace(it, value);
}

nlohmann::json ToJson(const ScheduleItem& scheduleItem)
{
    return nlohmann::json::object({{"address", scheduleItem.Address},
                                   {"subject_request_id", scheduleItem.SubjectRequestID},
                                   {"classroom", scheduleItem.Classroom}});
}

nlohmann::json ToJson(const ScheduleResult& scheduleResult)
{
    auto result = nlohmann::json::array();
    for(auto&& item : scheduleResult.items())
        result.emplace_back(ToJson(item));

    return result;
}


void ScheduleRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                         Poco::Net::HTTPServerResponse& response)
{
    URI uri(request.getURI());
    const auto requestStr = uri.toString();
    if(requestStr.find("/makeSchedule") != 0)
    {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        return;
    }

    std::istream& in = request.stream();
    std::vector<char> requestBody(request.getContentLength(), '\0');
    in.read(requestBody.data(), requestBody.size());

    nlohmann::json jsonResponse;
    try {
        GAScheduleGenerator generator;

        const auto jsonRequest = nlohmann::json::parse(requestBody);
        jsonResponse = ToJson(generator.Generate(ParseScheduleData(jsonRequest)));

        std::cout << "Subject requests count: " << jsonRequest["subject_requests"].size() << std::endl;
        std::cout << "Subject responses count: " << jsonResponse.size() << std::endl;
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

