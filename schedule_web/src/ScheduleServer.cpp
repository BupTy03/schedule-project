#include "ScheduleServer.h"
#include "ScheduleRequestHandler.h"
#include "ScheduleDataSerialization.h"

#include <Poco/Net/HTTPServer.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>


static const std::string OPTIONS_FILENAME = "options.json";
static constexpr std::uint16_t SERVER_DEFAULT_PORT = 9304;


ScheduleServer::ScheduleServer(std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger))
{
    assert(logger_ != nullptr);
    logger_->info("Starting server...");

    try
    {
        generator_.SetParams(LoadOptions(OPTIONS_FILENAME, *logger_));
    }
    catch(std::exception& e)
    {
        logger_->error("Error while loading options: {}", e.what());
    }
}

int ScheduleServer::main(const std::vector<std::string>&)
{
    using namespace Poco::Net;

    HTTPServer s(new ScheduleRequestHandlerFactory(generator_, logger_),
                 ServerSocket(SERVER_DEFAULT_PORT),
                 new HTTPServerParams);
    s.start();

    int ch = 0;
    while((ch = std::getchar()) != 'q');

    s.stop();
    return Application::EXIT_OK;
}


void CreateDefaultOptionsFile(const std::string& filename, spdlog::logger& logger)
{
    logger.info("Creating '{}' file with default options", filename);
    std::ofstream optionsFile(OPTIONS_FILENAME, std::ios::out);
    if(!optionsFile)
        throw std::runtime_error("Unable to create '" + filename + "' file");

    nlohmann::json j = ScheduleGA::DefaultParams();
    optionsFile << j.dump(4);
}

ScheduleGAParams LoadOptions(const std::string& filename, spdlog::logger& logger)
{
    std::fstream optionsFile(filename, std::ios::in);
    if(!optionsFile)
    {
        logger.warn("'{}' file is not found!", filename);
        CreateDefaultOptionsFile(filename, logger);
        return ScheduleGA::DefaultParams();
    }

    nlohmann::json jsonOptions;
    optionsFile >> jsonOptions;
    return jsonOptions;
}
