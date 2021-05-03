#pragma once
#include "ScheduleGenerator.hpp"


class GAScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;
};
