#pragma once
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <initializer_list>


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

class WeekDays;
class WeekDaysIterator
{
    friend class WeekDays;
    static constexpr std::uint8_t BEGIN_MASK = 0b00000001;
    static constexpr std::uint8_t END_MASK = 0b01000000;
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = bool;
    using reference = bool&;
    using pointer = bool*;

    constexpr WeekDaysIterator() = default;

    constexpr bool operator*() const { assert(mask_ <= 0b00111111); return data_ & mask_; }

    constexpr WeekDaysIterator& operator++() { mask_ <<= 1; return *this; }
    constexpr WeekDaysIterator operator++(int) { auto result = *this; ++(*this); return result; }

    constexpr bool operator==(WeekDaysIterator other) const
    {
        assert(data_ == other.data_);
        return mask_ == other.mask_;
    }
    constexpr bool operator!=(WeekDaysIterator other) const { return (*this == other); }

private:
    explicit WeekDaysIterator(std::uint8_t data, std::uint8_t mask) : mask_(mask), data_(data) {}

private:
    std::uint8_t mask_ = 0;
    std::uint8_t data_ = 0;
};

/**
 * Набор дней недели.
 * Инвариант класса: всегда содержит хотя бы один день недели.
 * Если удалить все дни недели при помощи метода Remove,
 * набор дней недели автоматически станет полным.
 */
class WeekDays
{
    static constexpr std::uint8_t FULL_WEEK = 0b00111111;
public:
    using iterator = WeekDaysIterator;

    WeekDays() = default;
    WeekDays(std::initializer_list<WeekDay> lst);

    iterator begin() const;
    iterator end() const;
    std::size_t size() const;

    void Add(WeekDay d);
    void Remove(WeekDay d);
    bool Contains(WeekDay d) const;

private:
    bool Empty() const;

private:
    mutable std::uint8_t days_ = FULL_WEEK;
};

template<typename T>
class SortedSet
{
public:
    SortedSet() = default;
    SortedSet(std::initializer_list<T> lst) : SortedSet(std::begin(lst), std::end(lst)) {}

    template<class Container, typename = typename std::enable_if_t<!std::is_same_v<std::decay_t<Container>, SortedSet>>>
    SortedSet(const Container& cont) : SortedSet(std::begin(cont), std::end(cont)) {}

    template<class Iter>
    explicit SortedSet(Iter first, Iter last)
        : elems_(first, last)
    {
        std::sort(elems_.begin(), elems_.end());
        elems_.erase(std::unique(elems_.begin(), elems_.end()), elems_.end());
    }

    bool Contains(const T& value) const
    {
        auto it = lower_bound(value);
        return (it != elems_.end() && *it == value);
    }
    bool Add(const T& value)
    {
        auto it = lower_bound(value);
        if(it != elems_.end() && *it == value)
            return false;

        elems_.emplace(it, value);
        return true;
    }

    bool Remove(const T& value)
    {
        auto it = lower_bound(value);
        if(it == elems_.end() || *it != value)
            return false;

        elems_.erase(it);
        return true;
    }

    [[nodiscard]] bool empty() const { return elems_.empty(); }
    [[nodiscard]] std::size_t size() const { return elems_.size(); }
    typename std::vector<T>::const_iterator begin() const { return elems_.begin(); }
    typename std::vector<T>::const_iterator end() const { return elems_.end(); }

    typename std::vector<T>::const_iterator lower_bound(const T& value) const
    {
        return std::lower_bound(elems_.begin(), elems_.end(), value);
    }

private:
    std::vector<T> elems_;
};

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
