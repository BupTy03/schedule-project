#pragma once
#include "ScheduleData.h"
#include "ScheduleIndividual.h"

#include <chrono>
#include <vector>


struct ScheduleGAParams
{
    int IndividualsCount = 0;
    int SelectionCount = 0;
    int CrossoverCount = 0;
    int MutationChance = 0;
};

class ScheduleGA
{
public:
    static ScheduleGAParams DefaultParams();

    void SetParams(const ScheduleGAParams& params);
    const ScheduleGAParams& Params() const { return params_; }

    ScheduleIndividual operator()(const ScheduleData& scheduleData) const;

private:
    ScheduleGAParams params_ = ScheduleGA::DefaultParams();
};

std::ostream& operator<<(std::ostream& os, const ScheduleGAParams& params);
ScheduleResult Generate(const ScheduleGA& generator, const ScheduleData& data);
