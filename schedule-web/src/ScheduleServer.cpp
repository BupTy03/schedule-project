#include "ScheduleServer.h"
#include "ScheduleRequestHandler.h"

#include <Poco/Net/HTTPServer.h>


int ScheduleServer::main(const std::vector<std::string>&)
{
    using namespace Poco::Net;

    HTTPServer s(new ScheduleRequestHandlerFactory, ServerSocket(8000), new HTTPServerParams);
    s.start();

    int ch = 0;
    while((ch = std::getchar()) != 'q');

    s.stop();
    return Application::EXIT_OK;
}
