#pragma once
#include "ScheduleCommon.h"
#include "ScheduleUtils.h"

#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <vector>


class ScheduleData;

struct ScheduleItem
{
    friend bool operator==(const ScheduleItem& lhs, const ScheduleItem& rhs)
    {
        return lhs.Address == rhs.Address && lhs.SubjectRequestID == rhs.SubjectRequestID
               && lhs.Classroom == rhs.Classroom;
    }
    friend bool operator!=(const ScheduleItem& lhs, const ScheduleItem& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Address = 0;
    std::size_t SubjectRequestID = 0;
    std::size_t Classroom = 0;
};

class ScheduleResult
{
public:
    explicit ScheduleResult(std::vector<ScheduleItem> items = {});

    const std::vector<ScheduleItem>& items() const { return items_; }
    std::vector<ScheduleItem>::iterator insert(const ScheduleItem& item);
    std::ranges::subrange<std::vector<ScheduleItem>::const_iterator>
        at(std::size_t lessonAddress) const;

    auto begin() const { return items_.begin(); }
    auto end() const { return items_.end(); }

private:
    std::vector<ScheduleItem> items_;
};
