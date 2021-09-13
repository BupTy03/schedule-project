#include "ScheduleServer.h"

#include "GAScheduleGenerator.h"
#include "ScheduleRequestHandler.h"

#include <Poco/Net/HTTPServer.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>


static const std::string OPTIONS_FILENAME = "options.json";


ScheduleServer::ScheduleServer()
    : generator_(std::make_unique<GAScheduleGenerator>())
{
    auto options = generator_->DefaultOptions();
    try
    {
        options = LoadOptions(OPTIONS_FILENAME, options);
        generator_->SetOptions(options);
    }
    catch(std::exception& e)
    {
        spdlog::get("server")->error("Error while loading options: {}", e.what());
    }
}

int ScheduleServer::main(const std::vector<std::string>&)
{
    using namespace Poco::Net;

    HTTPServer s(new ScheduleRequestHandlerFactory(generator_->Clone()),
                 ServerSocket(9304),
                 new HTTPServerParams);
    s.start();

    int ch = 0;
    while((ch = std::getchar()) != 'q');

    s.stop();
    return Application::EXIT_OK;
}


nlohmann::json OptionsToJson(const ScheduleGenOptions& options)
{
    nlohmann::json result;
    for(auto&& [key, value] : options)
    {
        if(std::holds_alternative<int>(value))
            result.emplace(key, std::get<int>(value));
        else if(std::holds_alternative<bool>(value))
            result.emplace(key, std::get<bool>(value));
    }

    return result;
}


void CreateDefaultOptionsFile(const std::string& filename, const ScheduleGenOptions& defaultOptions)
{
    spdlog::get("server")->info("Creating '{}' file with default options", filename);
    std::ofstream optionsFile(OPTIONS_FILENAME, std::ios::out);
    if(!optionsFile)
        throw std::runtime_error("Unable to create '" + filename + "' file");

    optionsFile << OptionsToJson(defaultOptions).dump(4);
}

ScheduleGenOptions LoadOptions(const std::string& filename,
                               const ScheduleGenOptions& defaultOptions)
{
    std::fstream optionsFile(filename, std::ios::in);
    if(!optionsFile)
    {
        spdlog::get("server")->warn("'{}' file is not found!", filename);
        CreateDefaultOptionsFile(filename, defaultOptions);
        return defaultOptions;
    }

    nlohmann::json jsonOptions;
    optionsFile >> jsonOptions;
    if(!jsonOptions.is_object())
        throw std::invalid_argument("Json object expected");

    ScheduleGenOptions options;
    for(auto&& [key, value] : jsonOptions.items())
    {
        if(value.is_number())
            options[key] = value.get<int>();
        else if(value.is_boolean())
            options[key] = value.get<bool>();
        else
            throw std::invalid_argument("Unexpected type of '" + key + "' option");
    }

    return options;
}
