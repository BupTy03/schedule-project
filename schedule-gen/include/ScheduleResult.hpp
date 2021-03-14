#pragma once

#include "ScheduleCommon.hpp"
#include <cstdint>
#include <vector>
#include <optional>
#include <stdexcept>


struct ScheduleItem
{
    std::size_t Subject = 0;
    std::size_t Professor = 0;
    std::size_t ClassRoom = 0;
};

class ScheduleResult
{
public:
    using Lesson = std::optional<ScheduleItem>;
    using Day = std::vector<Lesson>;
    using Group = std::vector<Day>;

    explicit ScheduleResult(std::vector<Group> groups)
        : groups_(std::move(groups))
    {
        if (groups.empty())
            throw std::invalid_argument("Groups are empty");

        if (groups.front().size() != SCHEDULE_DAYS_COUNT)
            throw std::invalid_argument("Invalid days count");
    }

    Lesson At(std::size_t group,
              std::size_t day,
              std::size_t lesson) const
    {
        return groups_.at(group).at(day).at(lesson);
    }

private:
    std::vector<Group> groups_;
};
