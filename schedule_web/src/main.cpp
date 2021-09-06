#include "ScheduleServer.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"


int main(int argc, char** argv)
{
    // Create a file rotating logger with 5mb size max and 3 rotated files
    constexpr auto max_size = 1048576 * 5;
    constexpr auto max_files = 3;
    auto logger = spdlog::rotating_logger_mt("server", "logs/log.txt", max_size, max_files);
    spdlog::flush_every(std::chrono::seconds(3));
    logger->info("Starting server...");

    ScheduleServer app;
    return app.run(argc, argv);
}
