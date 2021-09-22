#include "ScheduleDataGenerator.h"


constexpr std::size_t MIN_BLOCK_SIZE = 2;
constexpr std::size_t MAX_BLOCK_SIZE = 4;


ScheduleDataGenerator::ScheduleDataGenerator(std::random_device& randDevice,
                                             const ScheduleDataGeneratorParameters& parameters)
    : randGen_(randDevice())
    , parameters_(parameters)
{
    if(parameters_.MinGroupsCount < 1)
        throw std::invalid_argument("Invalid MinGroupsCount: at least 1 group expected");

    if(parameters_.MaxGroupsCount < 1)
        throw std::invalid_argument("Invalid MaxGroupsCount: at least 1 group expected");

    if(parameters_.MinGroupsCount > parameters_.MaxGroupsCount)
        throw std::invalid_argument("Invalid min-max groups count range");


    if(parameters_.MaxLessonsCount < 1 || parameters_.MaxLessonsCount > MAX_LESSONS_COUNT)
        throw std::invalid_argument("Invalid MaxLessonsCount");

    if(parameters_.MinLessonsCount < 1 || parameters_.MinLessonsCount > MAX_LESSONS_COUNT)
        throw std::invalid_argument("Invalid MinLessonsCount");

    if(parameters_.MinLessonsCount > parameters_.MaxLessonsCount)
        throw std::invalid_argument("Invalid min-max lessons count range");


    if(parameters_.MinClassroomsCount < 1)
        throw std::invalid_argument("Invalid MinClassroomsCount: at least 1 group expected");

    if(parameters_.MaxClassroomsCount < 1)
        throw std::invalid_argument("Invalid MaxClassroomsCount: at least 1 group expected");

    if(parameters_.MinClassroomsCount > parameters_.MaxClassroomsCount)
        throw std::invalid_argument("Invalid min-max classrooms count range");


    if(parameters_.MaxBuildingID < 1)
        throw std::invalid_argument("Invalid MaxBuildingID: at least 1 expected");
}

std::size_t ScheduleDataGenerator::GenerateRandomValue(std::size_t minID, std::size_t maxID)
{
    std::uniform_int_distribution<std::size_t> dist(minID, maxID);
    return dist(randGen_);
}

std::vector<std::size_t>
    ScheduleDataGenerator::GenerateRandomArray(std::size_t n, std::size_t minID, std::size_t maxID)
{
    std::vector<std::size_t> result;
    result.reserve(n);

    for(std::size_t i = 0; i < n; ++i)
        while(!insert_unique_ordered(result, GenerateRandomValue(minID, maxID)))
            ;

    return result;
}

std::vector<std::size_t> ScheduleDataGenerator::GenerateRandomGroups()
{
    return GenerateRandomArray(
        GenerateRandomValue(parameters_.MinGroupsCount, parameters_.MaxGroupsCount),
        0,
        std::numeric_limits<std::size_t>::max());
}

std::vector<std::size_t> ScheduleDataGenerator::GenerateRandomLessons()
{
    return GenerateRandomArray(
        GenerateRandomValue(parameters_.MinLessonsCount, parameters_.MaxLessonsCount),
        0,
        MAX_LESSONS_COUNT - 1);
}

std::vector<ClassroomAddress> ScheduleDataGenerator::GenerateRandomClassrooms()
{
    const auto count =
        GenerateRandomValue(parameters_.MinClassroomsCount, parameters_.MaxClassroomsCount);
    std::vector<ClassroomAddress> result;
    result.reserve(count);

    for(std::size_t i = 0; i < count; ++i)
    {
        while(!insert_unique_ordered(
            result,
            ClassroomAddress{.Building = GenerateRandomValue(0, parameters_.MaxBuildingID),
                             .Classroom =
                                 GenerateRandomValue(0, std::numeric_limits<std::size_t>::max())}))
            ;
    }

    return result;
}

std::vector<SubjectRequest> ScheduleDataGenerator::GenerateSubjectRequests(std::size_t n)
{
    std::vector<SubjectRequest> requests;
    requests.reserve(n);
    for(std::size_t i = 0; i < n; ++i)
    {
        // [id, professor, complexity, groups, lessons, classrooms]
        requests.emplace_back(
            SubjectRequest{i,
                           GenerateRandomValue(0, std::numeric_limits<std::size_t>::max()),
                           GenerateRandomValue(MIN_COMPLEXITY, MAX_COMPLEXITY),
                           GenerateRandomGroups(),
                           GenerateRandomLessons(),
                           GenerateRandomClassrooms()});
    }
    return requests;
}

ScheduleData ScheduleDataGenerator::GenerateData(std::size_t requestsCount,
                                                 std::size_t maxBlocksCount)
{
    std::vector<SubjectRequest> requests = GenerateSubjectRequests(requestsCount);
    assert(!std::empty(requests));

    std::vector<SubjectsBlock> blocks;
    blocks.reserve(maxBlocksCount);

    std::unordered_set<std::size_t> requestsInBlocks;
    for(std::size_t i = 1; i < requests.size(); ++i)
    {
        for(std::size_t j = 0; j < i; ++j)
        {
            if(requestsInBlocks.count(i) > 0 || requestsInBlocks.count(j) > 0)
                continue;

            std::vector<std::size_t> blockFirstLessons = SelectBlockFirstLessons(requests, {i, j});
            if(blockFirstLessons.empty())
                continue;

            requestsInBlocks.emplace(i);
            requestsInBlocks.emplace(j);
            blocks.emplace_back(std::vector<std::size_t>{i, j}, std::move(blockFirstLessons));
            if(blocks.size() >= maxBlocksCount)
                return ScheduleData{std::move(requests), std::move(blocks)};
        }
    }

    return ScheduleData{std::move(requests), std::move(blocks)};
}
