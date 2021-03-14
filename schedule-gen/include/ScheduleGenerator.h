#pragma once
#include "ScheduleData.h"
#include "ScheduleResult.h"


/**
 * Generates result schedule from schedule data.
 */
class ScheduleGenerator
{
public:
    virtual ~ScheduleGenerator() = default;
    virtual ScheduleResult Genetate(const ScheduleData&) = 0;
};
