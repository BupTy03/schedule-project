#include "ScheduleUtils.hpp"


std::size_t CalculatePadding(std::size_t baseAddress, std::size_t alignment)
{
    if(baseAddress == 0 || alignment == 0)
        return 0;

    if(alignment > baseAddress)
        return alignment - baseAddress;

    return baseAddress % alignment;
}
