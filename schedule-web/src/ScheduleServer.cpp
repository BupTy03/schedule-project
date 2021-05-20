#include "ScheduleServer.h"
#include "ScheduleRequestHandler.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SecureServerSocket.h"


int ScheduleServer::main(const std::vector<std::string>&)
{
    using namespace Poco::Net;

    // set-up a server socket
    SecureServerSocket svs(8000);
    HTTPServer s(new ScheduleRequestHandlerFactory, svs, new HTTPServerParams);
    s.start();
    waitForTerminationRequest();
    s.stop();
    return Application::EXIT_OK;
}
