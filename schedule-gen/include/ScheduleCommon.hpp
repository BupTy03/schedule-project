#pragma once
#include <cstdint>
#include <cstddef>
#include <array>


extern const std::size_t SCHEDULE_DAYS_COUNT;

enum class WeekDay : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};


/**
 * Набор дней недели.
 * Инвариант класса: всегда содержит хотя бы один день недели.
 * Если удалить все дни недели при помощи метода Remove,
 * набор дней недели автоматически станет полным.
 */
class WeekDays
{
public:
    typename std::array<bool, 6>::const_iterator begin() const;
    typename std::array<bool, 6>::const_iterator end() const;
    std::size_t size() const;

    void Add(WeekDay d);
    void Remove(WeekDay d);
    bool Contains(WeekDay d) const;

private:
    bool Empty() const;

private:
    mutable std::array<bool, 6> days_ = {true, true, true, true, true, true};
};

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
