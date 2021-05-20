#pragma once
#include <Poco/Util/ServerApplication.h>

#include <iostream>
#include <string>
#include <mutex>
#include <vector>


class ScheduleServer : public Poco::Util::ServerApplication
{
protected:
    int main(const std::vector<std::string>&) override;
};