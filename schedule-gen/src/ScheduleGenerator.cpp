#include "ScheduleGenerator.hpp"


std::ostream& operator<<(std::ostream& os, const std::map<std::string, ScheduleGenOption>& options)
{
    for(auto&&[key, option] : options)
    {
        os << '[' << key << "] = ";
        std::visit([&](auto&& value){ os << value; }, option);
        os << '\n';
    }

    return os;
}
