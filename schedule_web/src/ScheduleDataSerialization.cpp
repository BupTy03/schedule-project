#include "ScheduleDataSerialization.h"

#include "ScheduleUtils.h"

#include <set>


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

    std::ranges::sort(result);
    result.erase(std::unique(result.begin(), result.end()), result.end());
    result.shrink_to_fit();
    return result;
}

std::vector<std::size_t> ParseLessonsSet(const nlohmann::json& arr)
{
    if(!arr.is_array())
        throw std::invalid_argument("Json array expected");

    std::vector<std::size_t> result;
    result.reserve(arr.size());
    for(auto&& value : arr)
    {
        const auto v = value.get<std::int64_t>();
        if(value < 0 || value >= MAX_LESSONS_COUNT)
            throw std::out_of_range("Lesson value must be in range [0, "
                                    + std::to_string(MAX_LESSONS_COUNT) + ')');

        result.emplace_back(static_cast<std::size_t>(v));
    }

    std::ranges::sort(result);
    result.erase(std::unique(result.begin(), result.end()), result.end());
    result.shrink_to_fit();
    return result;
}

void from_json(const nlohmann::json& j, SubjectRequest& subjectRequest)
{
    std::vector<ClassroomAddress> classrooms;
    j.at("classrooms").get_to(classrooms);

    std::ranges::sort(classrooms);
    classrooms.erase(std::unique(classrooms.begin(), classrooms.end()), classrooms.end());
    classrooms.shrink_to_fit();

    subjectRequest = SubjectRequest(j.at("id").get<std::size_t>(),
                                    j.at("professor").get<std::size_t>(),
                                    j.at("complexity").get<std::size_t>(),
                                    ParseIDsSet(j.at("groups")),
                                    ParseLessonsSet(j.at("lessons")),
                                    std::move(classrooms));
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

    std::ranges::sort(classrooms);
    classrooms.erase(std::unique(classrooms.begin(), classrooms.end()), classrooms.end());
    classrooms.shrink_to_fit();
}

void from_json(const nlohmann::json& j, ScheduleData& scheduleData)
{
    std::vector<SubjectRequest> requests;
    j.at("subject_requests").get_to(requests);
    if(requests.empty())
        throw std::invalid_argument("'subject_requests' array is empty");

    std::ranges::sort(requests, {}, &SubjectRequest::ID);
    requests.erase(std::unique(requests.begin(), requests.end(), SubjectRequestIDEqual()),
                   requests.end());

    requests.shrink_to_fit();

    std::vector<SubjectsBlock> blocks;
    auto it = j.find("blocks");
    if(it != j.end())
    {
        std::vector<std::vector<std::size_t>> blocksIds;
        it->get_to(blocksIds);

        blocksIds.erase(std::remove_if(blocksIds.begin(),
                                       blocksIds.end(),
                                       [](const std::vector<std::size_t>& b)
                                       { return b.size() < 2; }),
                        blocksIds.end());

        blocks = ToSubjectsBlocks(requests, blocksIds);
    }

    scheduleData = ScheduleData(std::move(requests), std::move(blocks));
}

void from_json(const nlohmann::json& j, ScheduleGAParams& params)
{
    j.at("individuals_count").get_to(params.IndividualsCount);
    j.at("iterations_count").get_to(params.IterationsCount);
    j.at("selection_count").get_to(params.SelectionCount);
    j.at("crossover_count").get_to(params.CrossoverCount);
    j.at("mutation_chance").get_to(params.MutationChance);
}

void to_json(nlohmann::json& j, const ScheduleItem& scheduleItem)
{
    j = {{"address", scheduleItem.Address},
         {"subject_request_id", scheduleItem.SubjectRequestID},
         {"classroom", scheduleItem.Classroom}};
}

void to_json(nlohmann::json& j, const ScheduleResult& scheduleResult)
{
    j = scheduleResult.items();
}

void to_json(nlohmann::json& j, const OverlappedClassroom& overlappedClassroom)
{
    j = {{"address", overlappedClassroom.Address},
         {"classroom", overlappedClassroom.Classroom},
         {"subject_ids", overlappedClassroom.SubjectRequestsIDs}};
}

void to_json(nlohmann::json& j, const OverlappedProfessor& overlappedProfessor)
{
    j = {{"address", overlappedProfessor.Address},
         {"professor", overlappedProfessor.Professor},
         {"subject_ids", overlappedProfessor.SubjectRequestsIDs}};
}

void to_json(nlohmann::json& j, const OverlappedGroups& overlappedGroups)
{
    j = {{"address", overlappedGroups.Address},
         {"groups", overlappedGroups.Groups},
         {"subject_ids", overlappedGroups.SubjectRequestsIDs}};
}

void to_json(nlohmann::json& j, const ViolatedLessonRequest& violatedLessonsRequest)
{
    j = {{"address", violatedLessonsRequest.Address},
         {"subject_id", violatedLessonsRequest.SubjectRequestID}};
}

void to_json(nlohmann::json& j, const CheckScheduleResult& checkScheduleResult)
{
    j = {{"overlapped_classrooms", checkScheduleResult.OverlappedClassroomsList},
         {"overlapped_professors", checkScheduleResult.OverlappedProfessorsList},
         {"overlapped_groups", checkScheduleResult.OverlappedGroupsList},
         {"violated_lessons", checkScheduleResult.ViolatedLessons},
         {"out_of_block_requests", checkScheduleResult.OutOfBlockRequests}};
}

void to_json(nlohmann::json& j, const ScheduleGAParams& params)
{
    j = {{"individuals_count", params.IndividualsCount},
         {"iterations_count", params.IterationsCount},
         {"selection_count", params.SelectionCount},
         {"crossover_count", params.CrossoverCount},
         {"mutation_chance", params.MutationChance}};
}

void from_json(const nlohmann::json& j, ScheduleItem& scheduleItem)
{
    j.at("address").get_to(scheduleItem.Address);
    j.at("classroom").get_to(scheduleItem.Classroom);
    j.at("subject_request_id").get_to(scheduleItem.SubjectRequestID);
}

void to_json(nlohmann::json& j, const std::vector<ClassroomAddress>& classrooms)
{
    for(auto&& classroom : classrooms)
        j.push_back({classroom.Building, classroom.Classroom});
}

void to_json(nlohmann::json& j, const SubjectRequest& subjectRequest)
{
    j = {{"id", subjectRequest.ID()},
         {"professor", subjectRequest.Professor()},
         {"complexity", subjectRequest.Complexity()},
         {"groups", subjectRequest.Groups()},
         {"lessons", subjectRequest.Lessons()},
         {"classrooms", subjectRequest.Classrooms()}};
}

void to_json(nlohmann::json& j, const ScheduleData& scheduleData)
{
    j.emplace("subject_requests", scheduleData.SubjectRequests());

    nlohmann::json jBlocks;
    for(auto&& block : scheduleData.Blocks())
    {
        nlohmann::json jBlock;
        std::transform(std::begin(block.Requests()),
                       std::end(block.Requests()),
                       std::back_inserter(jBlock),
                       [&](std::size_t r) { return scheduleData.SubjectRequests().at(r).ID(); });

        jBlocks.emplace_back(std::move(jBlock));
    }

    if(!std::empty(jBlocks))
        j.emplace("blocks", std::move(jBlocks));
}

void from_json(const nlohmann::json& j, ScheduleResult& scheduleResult)
{
    std::vector<ScheduleItem> scheduleItems;
    j.get_to(scheduleItems);
    scheduleResult = ScheduleResult(std::move(scheduleItems));
}


constexpr bool IsLateScheduleLessonInSaturday(std::size_t l)
{
    constexpr std::array lateSaturdayLessonsTable = {
        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, true,  true,  true,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, false, false, false,

        false, false, false, false, true,  true,  true};

    static_assert(lateSaturdayLessonsTable.size() == MAX_LESSONS_COUNT,
                  "re-fill lateSaturdayLessonsTable");
    return lateSaturdayLessonsTable[l];
}


constexpr bool SuitableForEveningClasses(std::size_t l)
{
    constexpr std::array eveningLessonsTable = {false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                true,  true,  true,  true,  true,  true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                false, false, false, false, false, true, true,

                                                true,  true,  true,  true,  true,  true, true};

    static_assert(eveningLessonsTable.size() == MAX_LESSONS_COUNT, "re-fill eveningLessonsTable");
    return eveningLessonsTable[l];
}


nlohmann::json JsonConvertFromOldFormat(const nlohmann::json& j)
{
    nlohmann::json result;

    auto subjectRequests = j.at("subject_requests");
    const auto lockedLessons = j.at("locked_lessons");
    std::size_t subjectRequestIndex = 0;
    for(auto&& subjectRequest : subjectRequests)
    {
        const std::set<std::size_t> weekDays = subjectRequest.at("days");
        std::set<std::size_t> lessons;
        for(std::size_t lesson = 0; lesson < MAX_LESSONS_COUNT; ++lesson)
        {
            std::size_t weekDay = (lesson / MAX_LESSONS_PER_DAY) % 6;
            if(weekDays.count(weekDay) == 0)
                continue;

            if(IsLateScheduleLessonInSaturday(lesson))
                continue;

            lessons.emplace(lesson);
        }

        subjectRequest.erase("days");
        for(const auto& lockedLesson : lockedLessons)
        {
            if(lockedLesson.at("subject_request_id") == subjectRequest.at("id"))
            {
                for(std::size_t l : std::set<std::size_t>{lessons})
                {
                    if(l % MAX_LESSONS_PER_DAY != lockedLesson.at("address"))
                        lessons.erase(l);
                }
            }
        }

        subjectRequest.emplace("lessons", lessons);
        result.emplace_back(subjectRequest);
    }

    return result;
}
