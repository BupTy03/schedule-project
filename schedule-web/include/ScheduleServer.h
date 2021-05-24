#pragma once
#include "ScheduleGenerator.hpp"
#include <Poco/Util/ServerApplication.h>

#include <iostream>
#include <string>
#include <mutex>
#include <vector>


class ScheduleServer : public Poco::Util::ServerApplication
{
public:
    ScheduleServer();

protected:
    int main(const std::vector<std::string>&) override;

private:
    std::map<std::string, ScheduleGenOption> options_;
};