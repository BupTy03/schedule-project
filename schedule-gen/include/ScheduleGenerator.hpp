#pragma once
#include "ScheduleData.hpp"
#include "ScheduleResult.hpp"


/**
 * Generates result schedule from schedule data.
 */
class ScheduleGenerator
{
public:
    virtual ~ScheduleGenerator() = default;
    virtual ScheduleResult Genetate(const ScheduleData&) = 0;
};