#pragma once
#include <array>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iterator>
#include <iostream>
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

    [[nodiscard]] constexpr bool operator*() const { assert(mask_ <= 0b00111111); return data_ & mask_; }

    constexpr WeekDaysIterator& operator++() { mask_ <<= 1; return *this; }
    constexpr WeekDaysIterator operator++(int) { auto result = *this; ++(*this); return result; }

    [[nodiscard]] constexpr bool operator==(WeekDaysIterator other) const
    {
        assert(data_ == other.data_);
        return mask_ == other.mask_;
    }
    [[nodiscard]] constexpr bool operator!=(WeekDaysIterator other) const { return (*this == other); }

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

    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;
    [[nodiscard]] std::size_t size() const;

    void Add(WeekDay d);
    void Remove(WeekDay d);
    [[nodiscard]] bool Contains(WeekDay d) const;

private:
    [[nodiscard]] bool Empty() const;

private:
    mutable std::uint8_t days_ = FULL_WEEK;
};

struct LessonAddress
{
    explicit LessonAddress(std::size_t Group, std::size_t Day, std::size_t Lesson)
            : Group(Group), Day(Day), Lesson(Lesson) { }

    [[nodiscard]] friend bool operator<(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group < rhs.Group || (lhs.Group == rhs.Group && lhs.Day < rhs.Day) ||
               (lhs.Group == rhs.Group && lhs.Day == rhs.Day && lhs.Lesson < rhs.Lesson);
    }

    [[nodiscard]] friend bool operator==(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group == rhs.Group && lhs.Day == rhs.Day && lhs.Lesson == rhs.Lesson;
    }

    [[nodiscard]] friend bool operator!=(const LessonAddress& lhs, const LessonAddress& rhs) { return !(lhs == rhs); }

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
                                      std::size_t subject);

    [[nodiscard]] friend bool operator==(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day == rhs.Day &&
               lhs.Group == rhs.Group &&
               lhs.Professor == rhs.Professor &&
               lhs.Lesson == rhs.Lesson &&
               lhs.Classroom == rhs.Classroom &&
               lhs.Subject == rhs.Subject;
    }

    [[nodiscard]] friend bool operator!=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool operator<(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Day < rhs.Day ||
               (lhs.Day == rhs.Day && lhs.Group < rhs.Group) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor < rhs.Professor) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson < rhs.Lesson) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom < rhs.Classroom) ||
               (lhs.Day == rhs.Day && lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom == rhs.Classroom && lhs.Subject < rhs.Subject);
    }

    [[nodiscard]] friend bool operator>(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend bool operator<=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return !(lhs > rhs);
    }
    [[nodiscard]] friend bool operator>=(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
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

    [[nodiscard]] bool contains(const T& value) const
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
    [[nodiscard]] auto begin() const { return elems_.begin(); }
    [[nodiscard]] auto end() const { return elems_.end(); }

    [[nodiscard]] auto lower_bound(const T& value) const
    {
        return std::lower_bound(elems_.begin(), elems_.end(), value);
    }

private:
    std::vector<T> elems_;
};

struct FirstLess
{
    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs.first < rhs.first;
    }

    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const std::pair<T1, T2>& lhs, const T1& rhs)
    {
        return lhs.first < rhs;
    }

    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const T1& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs < rhs.first;
    }
};

template<typename K, typename T, typename A = std::allocator<std::pair<K, T>>>
class SortedMap
{
public:
    SortedMap() = default;
    explicit SortedMap(const A& a) : elems_(a) { }

    [[nodiscard]] const std::vector<std::pair<K, T>, A>& elems() const { return elems_; }
    [[nodiscard]] std::vector<std::pair<K, T>, A>& elems() { return elems_; }

    [[nodiscard]] T& operator[](const K& key)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
        if(it == elems_.end() || FirstLess()(key, *it))
            it = elems_.emplace(it, key, T{});

        return it->second;
    }

    void clear() { elems_.clear(); }
    [[nodiscard]] auto begin() const { return elems_.begin(); }
    [[nodiscard]] auto end() const { return elems_.end(); }

private:
    std::vector<std::pair<K, T>, A> elems_;
};

template<class SortedRange1, class SortedRange2>
[[nodiscard]] bool set_intersects(const SortedRange1& r1, const SortedRange2& r2)
{
    for(const auto& e : r1)
    {
        auto it = std::lower_bound(std::begin(r2), std::end(r2), e);
        if(it != std::end(r2) && !(e < *it))
            return true;
    }

    return false;
}

[[nodiscard]] std::size_t CalculatePadding(std::size_t baseAddress, std::size_t alignment);


struct LinearAllocatorBufferSpan
{
    explicit LinearAllocatorBufferSpan(std::uint8_t* ptr, std::size_t total);

    std::uint8_t* ptr;
    std::size_t offset;
    std::size_t total;
};


template<typename T>
class LinearAllocator
{
    template<typename U>
    friend class LinearAllocator;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    template< class U > struct rebind { using other = LinearAllocator<U>; };

    LinearAllocator() = default;
#ifdef _DEBUG
    ~LinearAllocator() { assert(externalAllocationsCounter_ == 0); }
#endif

    explicit LinearAllocator(LinearAllocatorBufferSpan* buffer) : buffer_(buffer) { assert(buffer != nullptr); }

    template<typename U>
    LinearAllocator(const LinearAllocator<U>& other) : buffer_(other.buffer_) { }

    template<typename U>
    LinearAllocator& operator=(const LinearAllocator<U>& other) { buffer_ = other.buffer_; }

    [[nodiscard]] T* allocate(std::size_t count)
    {
        constexpr auto alignment = __alignof(T);
        const std::size_t size = count * sizeof(T);

        std::size_t padding = 0;
        const std::size_t currentAddress = reinterpret_cast<std::size_t>(buffer_->ptr) + buffer_->offset;
        if (alignment != 0 && buffer_->offset % alignment != 0)
        {
            // Alignment is required. Find the next aligned memory address and update offset
            padding = CalculatePadding(currentAddress, alignment);
        }

        if (buffer_->offset + padding + size > buffer_->total)
        {
#ifdef _DEBUG
            externalAllocationsCounter_++;
#endif
            return overflowAllocator_.allocate(count);
        }

        buffer_->offset += padding + size;
        return reinterpret_cast<T*>(currentAddress + padding);
    }
    void deallocate(T* p, std::size_t count)
    {
        auto ptr = reinterpret_cast<std::uint8_t*>(p);
        if(ptr >= buffer_->ptr && ptr <= (buffer_->ptr + buffer_->total))
            return;

#ifdef _DEBUG
        externalAllocationsCounter_--;
#endif
        overflowAllocator_.deallocate(p, count);
    }

    [[nodiscard]] friend bool operator==(const LinearAllocator& lhs, const LinearAllocator& rhs) { return lhs.buffer_ == rhs.buffer_; }
    [[nodiscard]] friend bool operator!=(const LinearAllocator& lhs, const LinearAllocator& rhs) { return !(lhs == rhs); }

private:

#ifdef _DEBUG
    std::int64_t externalAllocationsCounter_ = 0;
#endif

    LinearAllocatorBufferSpan* buffer_ = nullptr;
    std::allocator<T> overflowAllocator_;
};

[[nodiscard]] WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
