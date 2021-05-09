#pragma once
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <stdexcept>
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

struct LessonAddress
{
    explicit LessonAddress(std::size_t Group, std::size_t Day, std::size_t Lesson)
            : Group(Group), Day(Day), Lesson(Lesson)
    {
    }

    friend bool operator<(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group < rhs.Group || (lhs.Group == rhs.Group && lhs.Day < rhs.Day) ||
               (lhs.Group == rhs.Group && lhs.Day == rhs.Day && lhs.Lesson < rhs.Lesson);
    }

    friend bool operator==(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group == rhs.Group && lhs.Day == rhs.Day && lhs.Lesson == rhs.Lesson;
    }

    friend bool operator!=(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return !(lhs == rhs);
    }

    std::size_t Group;
    std::size_t Day;
    std::size_t Lesson;
};

struct LessonsMatrixItemAddress
{
    explicit LessonsMatrixItemAddress(std::size_t day,
                                      std::size_t group,
                                      std::size_t professor,
                                      std::size_t lesson,
                                      std::size_t classroom,
                                      std::size_t subject)
            : Day(day)
            , Group(group)
            , Professor(professor)
            , Lesson(lesson)
            , Classroom(classroom)
            , Subject(subject)
    {
    }

    friend bool operator==(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day == rhs.Day &&
               lhs.Group == rhs.Group &&
               lhs.Professor == rhs.Professor &&
               lhs.Lesson == rhs.Lesson &&
               lhs.Classroom == rhs.Classroom &&
               lhs.Subject == rhs.Subject;
    }

    friend bool operator!=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day < rhs.Day ||
               (lhs.Day == rhs.Day && lhs.Group < rhs.Group) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor < rhs.Professor) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson < rhs.Lesson) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom < rhs.Classroom) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom == rhs.Classroom && lhs.Subject < rhs.Subject);
    }

    friend bool operator>(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs < rhs);
    }


    std::size_t Day;
    std::size_t Group;
    std::size_t Professor;
    std::size_t Lesson;
    std::size_t Classroom;
    std::size_t Subject;
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

    [[nodiscard]] const std::vector<T>& elems() const { return elems_; }

    bool contains(const T& value) const
    {
        auto it = lower_bound(value);
        return (it != elems_.end() && *it == value);
    }
    auto insert(const T& value)
    {
        auto it = lower_bound(value);
        if(it != elems_.end() && *it == value)
            return it;

        it = elems_.emplace(it, value);
        return it;
    }

    bool erase(const T& value)
    {
        auto it = lower_bound(value);
        if(it == elems_.end() || *it != value)
            return false;

        elems_.erase(it);
        return true;
    }

    [[nodiscard]] bool empty() const { return elems_.empty(); }
    [[nodiscard]] std::size_t size() const { return elems_.size(); }
    auto begin() const { return elems_.begin(); }
    auto end() const { return elems_.end(); }

    auto lower_bound(const T& value) const
    {
        return std::lower_bound(elems_.begin(), elems_.end(), value);
    }

private:
    std::vector<T> elems_;
};

struct FirstLess
{
    template<typename T1, typename T2>
    bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs.first < rhs.first;
    }

    template<typename T1, typename T2>
    bool operator()(const std::pair<T1, T2>& lhs, const T1& rhs)
    {
        return lhs.first < rhs;
    }

    template<typename T1, typename T2>
    bool operator()(const T1& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs < rhs.first;
    }
};

template<typename K, typename T>
class SortedMap
{
public:
    T& operator[](const K& key)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
        if(it == elems_.end() || FirstLess()(key, *it))
            it = elems_.emplace(it, key, T{});

        return it->second;
    }

    const T& at(const K& key) const
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
        if(it == elems_.end() || FirstLess()(key, *it))
            throw std::out_of_range("key is out of range");

        return it->second;
    }

    void clear() { elems_.clear(); }
    auto begin() const { return elems_.begin(); }
    auto end() const { return elems_.end(); }

private:
    std::vector<std::pair<K, T>> elems_;
};

template<class SortedRange1, class SortedRange2>
bool set_intersects(const SortedRange1& r1, const SortedRange2& r2)
{
    for(const auto& e : r1)
    {
        auto it = std::lower_bound(std::begin(r2), std::end(r2), e);
        if(it != std::end(r2) && !(e < *it))
            return true;
    }

    return false;
}

WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
