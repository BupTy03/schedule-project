#include "ScheduleDataGenerator.h"
#include "ScheduleDataSerialization.h"
#include "ScheduleServer.h"

#include <nlohmann/json.hpp>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <random>


std::shared_ptr<spdlog::logger> make_server_logger();

int main(int argc, char** argv)
{
    // ScheduleServer app{make_server_logger()};
    // return app.run(argc, argv);

    std::random_device randomDevice;
    ScheduleDataGenerator generator{randomDevice,
                                    ScheduleDataGeneratorParameters{.MinGroupsCount = 2,
                                                                    .MaxGroupsCount = 10,
                                                                    .MinLessonsCount = 1,
                                                                    .MaxLessonsCount = 84,
                                                                    .MinClassroomsCount = 1,
                                                                    .MaxClassroomsCount = 7,
                                                                    .MaxBuildingID = 3}};

    nlohmann::json jData = generator.GenerateData(200, 20);

    std::cout << jData.dump(4) << std::endl;

    std::cout << "\n\nBlocks:\n";
    std::cout << (jData.contains("blocks") ? jData["blocks"].dump(4) : "blocks are empty")
              << std::endl;
    return 0;
}


std::shared_ptr<spdlog::logger> make_server_logger()
{
    // Create a file rotating logger with 5mb size max and 3 rotated files
    constexpr auto max_size = 1048576 * 5;
    constexpr auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("server", "logs/log.txt", max_size, max_files);
    spdlog::flush_every(std::chrono::seconds(3));
    return logger;
}
