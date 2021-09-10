#pragma once
#include "ScheduleUtils.h"
#include "ScheduleCommon.h"
#include <map>
#include <vector>
#include <cstdint>
#include <optional>
#include <stdexcept>


class ScheduleData;

struct ScheduleItem
{
    friend bool operator==(const ScheduleItem& lhs, const ScheduleItem& rhs)
    {
        return lhs.Address == rhs.Address &&
            lhs.SubjectRequestID == rhs.SubjectRequestID &&
            lhs.Classroom == rhs.Classroom;
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
    ScheduleResult() = default;
    explicit ScheduleResult(std::vector<ScheduleItem> items);

    const std::vector<ScheduleItem>& items() const { return items_; }
    std::vector<ScheduleItem>::iterator insert(const ScheduleItem& item);
    std::ranges::subrange<std::vector<ScheduleItem>::const_iterator> at(std::size_t lessonAddress) const;

private:
    std::vector<ScheduleItem> items_;
};
