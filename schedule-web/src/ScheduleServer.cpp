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

    nlohmann::json jsonOptions;
    try
    {
        optionsFile >> jsonOptions;
    }
    catch(std::exception& e)
    {
        std::cout << "Error while parsing '" << OPTIONS_FILENAME << "' file: " << e.what() << std::endl;
    }

    if(!jsonOptions.is_object())
        return;

    for(auto&&[key, value] : jsonOptions.items())
    {
        if(value.is_number())
            options_[key] = value.get<int>();
        else if(value.is_boolean())
            options_[key] = value.get<bool>();
        else
            std::cout << "Unknown option type in '" << OPTIONS_FILENAME << "' file" << std::endl;
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
