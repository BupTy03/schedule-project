#include "ScheduleDataSerialization.h"
#include "ScheduleUtils.hpp"


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


void from_json(const nlohmann::json& j, SubjectWithAddress& subjectWithAddress)
{
    j.at("subject_request_id").get_to(subjectWithAddress.SubjectRequestID);
    j.at("address").get_to(subjectWithAddress.Address);
}

void from_json(const nlohmann::json& j, WeekDays& weekDays)
{
    weekDays = WeekDays::emptyWeek();
    if(!j.is_array())
        throw std::invalid_argument("Json array expected");

    for(auto&& wd : j)
    {
        const auto value = wd.get<int>();
        if(value < 0 || value > 5)
            throw std::invalid_argument("Week day number must be in range [0, 5]");

        weekDays.insert(static_cast<WeekDay>(value % DAYS_IN_SCHEDULE_WEEK));
    }
}

void from_json(const nlohmann::json& j, SubjectRequest& subjectRequest)
{
    subjectRequest = SubjectRequest(
        j.at("id").get<std::size_t>(),
        j.at("professor").get<std::size_t>(),
        j.at("complexity").get<std::size_t>(),
        j.at("days").get<WeekDays>(),
        ParseIDsSet(j.at("groups")),
        j.at("classrooms"));
}

void from_json(const nlohmann::json& j, std::vector<ClassroomAddress>& classrooms)
{
    classrooms.clear();
    if(!j.is_array())
        throw std::invalid_argument("Json array expected");

    for(std::size_t building = 0; building < j.size(); ++building)
    {
        auto&& jsonClassrooms = j.at(building);
        if(!jsonClassrooms.is_array())
            throw std::invalid_argument("Json array expected");

        for(auto&& cr : jsonClassrooms)
        {
            auto classroom = cr.get<std::int64_t>();
            if(classroom < 0)
                throw std::invalid_argument("Classroom ID must be positive number");

            classrooms.emplace_back(building, classroom);
        }
    }

    std::sort(classrooms.begin(), classrooms.end());
    classrooms.erase(std::unique(classrooms.begin(), classrooms.end()), classrooms.end());
}

void from_json(const nlohmann::json& j, ScheduleData& scheduleData)
{
    std::vector<SubjectRequest> requests;
    j.at("subject_requests").get_to(requests);
    if(requests.empty())
        throw std::invalid_argument("'subject_requests' array is empty");

    // 'locked_lessons' field is optional
    std::vector<SubjectWithAddress> locked;
    auto lockedLessonsIt = j.find("locked_lessons");
    if(lockedLessonsIt != j.end())
        lockedLessonsIt->get_to(locked);

    scheduleData = ScheduleData(std::move(requests), std::move(locked));
}

void to_json(nlohmann::json& j, const ScheduleItem& scheduleItem)
{
    j = nlohmann::json::object({{"address", scheduleItem.Address},
                                   {"subject_request_id", scheduleItem.SubjectRequestID},
                                   {"classroom", scheduleItem.Classroom}});
}

void to_json(nlohmann::json& j, const ScheduleResult& scheduleResult)
{
    j = scheduleResult.items();
}

void to_json(nlohmann::json& j, const OverlappedClassroom& overlappedClassroom)
{
    j = nlohmann::json{ { "address", overlappedClassroom.Address },
                        { "classroom", overlappedClassroom.Classroom },
                        { "subject_ids", overlappedClassroom.SubjectRequestsIDs } };
}

void to_json(nlohmann::json& j, const OverlappedProfessor& overlappedProfessor)
{
    j = nlohmann::json{ { "address", overlappedProfessor.Address },
                        { "professor", overlappedProfessor.Professor },
                        { "subject_ids", overlappedProfessor.SubjectRequestsIDs } };
}

void to_json(nlohmann::json& j, const OverlappedGroups& overlappedGroups)
{
    j = nlohmann::json{ { "address", overlappedGroups.Address },
                        { "groups", overlappedGroups.Groups },
                        { "subject_ids", overlappedGroups.SubjectRequestsIDs } };
}

void to_json(nlohmann::json& j, const ViolatedWeekdayRequest& violatedWeekdayRequest)
{
    j = nlohmann::json{{"address", violatedWeekdayRequest.Address},
                       {"subject_id", violatedWeekdayRequest.SubjectRequestID}};
}

void to_json(nlohmann::json& j, const CheckScheduleResult& checkScheduleResult)
{
    j = nlohmann::json{{"overlapped_classrooms", checkScheduleResult.OverlappedClassroomsList},
                       {"overlapped_professors", checkScheduleResult.OverlappedProfessorsList},
                       {"overlapped_groups", checkScheduleResult.OverlappedGroupsList},
                       {"violated_weekday_requests", checkScheduleResult.ViolatedWeekdayRequestsList}};
}

void from_json(const nlohmann::json& j, ScheduleItem& scheduleItem)
{
    j.at("address").get_to(scheduleItem.Address);
    j.at("classroom").get_to(scheduleItem.Classroom);
    j.at("subject_request_id").get_to(scheduleItem.SubjectRequestID);
}

void from_json(const nlohmann::json& j, ScheduleResult& scheduleResult)
{
    std::vector<ScheduleItem> scheduleItems;
    j.get_to(scheduleItems);
    scheduleResult = ScheduleResult(std::move(scheduleItems));
}
