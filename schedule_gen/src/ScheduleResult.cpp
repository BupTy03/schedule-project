#include "ScheduleResult.h"
#include "ScheduleData.h"
#include <iostream>
#include <algorithm>
#include <functional>


ScheduleResult::ScheduleResult(std::vector<ScheduleItem> items)
    : items_(std::move(items))
{
    std::ranges::sort(items_, {}, &ScheduleItem::Address);
}


std::ranges::subrange<std::vector<ScheduleItem>::const_iterator> ScheduleResult::at(std::size_t lessonAddress) const
{
    return std::ranges::equal_range(items_, lessonAddress, {}, &ScheduleItem::Address);
}

std::vector<ScheduleItem>::iterator ScheduleResult::insert(const ScheduleItem& item)
{
    auto it = std::ranges::lower_bound(items_.begin(), items_.end(), item.Address, {}, &ScheduleItem::Address);
    return items_.insert(it, item);
}
