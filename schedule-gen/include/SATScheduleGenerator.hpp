#pragma once
#include "ScheduleGenerator.hpp"


class SATScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Generate(const ScheduleData& data) override;
};
