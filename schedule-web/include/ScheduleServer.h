#pragma once
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"

#include <iostream>
#include <string>
#include <mutex>
#include <vector>


class KeyPassphraseHandler : public Poco::Net::PrivateKeyPassphraseHandler
{
public:
    explicit KeyPassphraseHandler(bool onServerSide)
        : PrivateKeyPassphraseHandler(onServerSide)
    {
    }

    virtual void onPrivateKeyRequested(const void* pSender, std::string& privateKey)
    {
        privateKey = "secret";
    }
};

class ScheduleServer : public Poco::Util::ServerApplication
{
public:
    ScheduleServer()
    {
        Poco::Net::initializeSSL();
        Poco::Net::HTTPStreamFactory::registerFactory();
        Poco::Net::HTTPSStreamFactory::registerFactory();

        using namespace Poco::Net;
        using namespace Poco::Util;

        Context::Ptr pDefaultServerContext = SSLManager::instance().defaultServerContext();
        Context::Ptr pServerContext = new Context(Context::SERVER_USE,
                                                  "any.pfx",
                                                  Context::VERIFY_RELAXED,
                                                  Context::Options::OPT_LOAD_CERT_FROM_FILE);

        Poco::SharedPtr<InvalidCertificateHandler> ptrHandler = new AcceptCertificateHandler(true);
        SSLManager::instance().initializeServer(new KeyPassphraseHandler(true), ptrHandler, pServerContext);
    }
    ~ScheduleServer()
    {
        Poco::Net::uninitializeSSL();
    }

protected:
    int main(const std::vector<std::string>&) override;
};
