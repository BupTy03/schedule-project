#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <algorithm>


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

template<typename T>
class SortedSet
{
public:
    SortedSet() = default;

    template<class Iter>
    explicit SortedSet(Iter first, Iter last)
        : elems_(first, last)
    {
        std::sort(elems_.begin(), elems_.end());
        elems_.erase(std::unique(elems_.begin(), elems_.end()), elems_.end());
    }

    bool Contains(const T& value) const
    {
        auto it = find(value);
        return (it != elems_.end() && *it == value);
    }
    bool Add(const T& value)
    {
        auto it = find(value);
        if(it != elems_.end() && *it == value)
            return false;

        elems_.emplace(it, value);
        return true;
    }
    bool Remove(const T& value)
    {
        auto it = find(value);
        if(it == elems_.end() || *it != value)
            return false;

        elems_.erase(it);
        return true;
    }

    bool empty() const { return elems_.empty(); }
    std::size_t size() const { return elems_.size(); }
    typename std::vector<T>::const_iterator begin() const
    {
        return elems_.begin();
    }
    typename std::vector<T>::const_iterator end() const
    {
        return elems_.end();
    }

private:
    typename std::vector<T>::const_iterator find(const T& value) const
    {
        return std::lower_bound(elems_.begin(), elems_.end(), value);
    }

private:
    std::vector<T> elems_;
};

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
