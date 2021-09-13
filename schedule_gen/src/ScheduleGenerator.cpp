#include "ScheduleGenerator.h"


std::ostream& operator<<(std::ostream& os, const ScheduleGenOptions& options)
{
    for(auto&& [key, option] : options)
    {
        os << '[' << key << "] = ";
        std::visit([&](auto&& value) { os << value; }, option);
        os << '\n';
    }

    return os;
}


ScheduleGenOptions ScheduleGenerator::DefaultOptions() const { return {}; }
