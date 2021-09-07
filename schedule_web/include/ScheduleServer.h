#pragma once
#include "ScheduleGenerator.h"
#include <Poco/Util/ServerApplication.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>


class ScheduleServer : public Poco::Util::ServerApplication
{
public:
    ScheduleServer();

protected:
    int main(const std::vector<std::string>&) override;

private:
    std::unique_ptr<ScheduleGenerator> generator_;
};

nlohmann::json OptionsToJson(const ScheduleGenOptions& options);
void CreateDefaultOptionsFile(const std::string& filename,
                              const ScheduleGenOptions& defaultOptions);
ScheduleGenOptions LoadOptions(const std::string& filename,
                               const ScheduleGenOptions& defaultOptions);
