#pragma once
#include "ScheduleGA.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Poco/Util/ServerApplication.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>


class ScheduleServer : public Poco::Util::ServerApplication
{
public:
    explicit ScheduleServer(std::shared_ptr<spdlog::logger> logger);

protected:
    int main(const std::vector<std::string>&) override;

private:
    std::shared_ptr<spdlog::logger> logger_;
    ScheduleGA generator_;
};

void CreateDefaultOptionsFile(const std::string& filename, spdlog::logger& logger);
ScheduleGAParams LoadOptions(const std::string& filename, spdlog::logger& logger);
