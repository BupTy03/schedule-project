#include "ScheduleServer.h"
#include "ScheduleRequestHandler.h"

#include "nlohmann/json.hpp"

#include <Poco/Net/HTTPServer.h>

#include <string>
#include <fstream>


static const std::string OPTIONS_FILENAME = "options.json";

ScheduleServer::ScheduleServer()
{
    std::ifstream optionsFile(OPTIONS_FILENAME, std::ios::in);
    if(!optionsFile)
    {
        std::cout << "Warning: Unable to find '" << OPTIONS_FILENAME << "' file!" << std::endl;
        return;
    }

    optionsFile.seekg(0, std::ios::end);
    const auto fileSize = optionsFile.tellg();
    optionsFile.seekg(0, std::ios::beg);

    std::vector<char> fileBytes(fileSize);
    optionsFile.read(fileBytes.data(), fileBytes.size());

    nlohmann::json jsonFile;
    try
    {
        jsonFile = nlohmann::json::parse(fileBytes);
    }
    catch(std::exception& e)
    {
        std::cout << "Error while parsing '" << OPTIONS_FILENAME << "' file: " << e.what() << std::endl;
    }

    if(!jsonFile.is_object())
        return;

    for(auto&&[key, value] : jsonFile.items())
    {
        if(value.is_number_unsigned())
            options_[key] = value.get<std::uint32_t>();
        else if(value.is_boolean())
            options_[key] = value.get<bool>();
    }
}

int ScheduleServer::main(const std::vector<std::string>&)
{
    using namespace Poco::Net;

    HTTPServer s(new ScheduleRequestHandlerFactory(&options_), ServerSocket(9304), new HTTPServerParams);
    s.start();

    int ch = 0;
    while((ch = std::getchar()) != 'q');

    s.stop();
    return Application::EXIT_OK;
}
