#include "ScheduleCommon.h"
#include "ScheduleData.h"

#include <random>
#include <vector>


struct ScheduleDataGeneratorParameters
{
    std::size_t MinGroupsCount = 0;
    std::size_t MaxGroupsCount = 0;
    std::size_t MinLessonsCount = 0;
    std::size_t MaxLessonsCount = 0;
    std::size_t MinClassroomsCount = 0;
    std::size_t MaxClassroomsCount = 0;
    std::size_t MaxBuildingID = 0;
};

class ScheduleDataGenerator
{
public:
    explicit ScheduleDataGenerator(std::random_device& randDevice,
                                   const ScheduleDataGeneratorParameters& parameters);

    std::size_t GenerateRandomValue(std::size_t minID, std::size_t maxID);
    std::vector<std::size_t>
        GenerateRandomArray(std::size_t n, std::size_t minID, std::size_t maxID);
    std::vector<std::size_t> GenerateRandomGroups();
    std::vector<std::size_t> GenerateRandomLessons();
    std::vector<ClassroomAddress> GenerateRandomClassrooms();

    std::vector<SubjectRequest> GenerateSubjectRequests(std::size_t n);
    ScheduleData GenerateData(std::size_t requestsCount, std::size_t maxBlocksCount);

private:
    std::mt19937 randGen_;
    ScheduleDataGeneratorParameters parameters_;
};
