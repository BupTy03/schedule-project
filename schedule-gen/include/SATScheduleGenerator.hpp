#pragma once
#include "ScheduleGenerator.hpp"


class SATScheduleGenerator : public ScheduleGenerator
{
public:
    [[nodiscard]] ScheduleResult Generate(const ScheduleData& data) override;
};
