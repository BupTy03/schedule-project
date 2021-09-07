#pragma once
#include "ScheduleGA.h"
#include "ScheduleGenerator.h"
#include <memory>


class GAScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;

    void SetOptions(const ScheduleGenOptions& options) override;
    ScheduleGenOptions DefaultOptions() const override;

    std::unique_ptr<ScheduleGenerator> Clone() const override;

public:
    ScheduleGAParams params_ = ScheduleGA::DefaultParams();
};
