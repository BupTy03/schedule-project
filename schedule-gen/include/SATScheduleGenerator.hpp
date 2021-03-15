#pragma once
#include "ScheduleGenerator.hpp"


class SATScheduleGenerator : public ScheduleGenerator
{
public:
    ScheduleResult Genetate(const ScheduleData& data) override;
};
