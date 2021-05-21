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


constexpr std::size_t SCHEDULE_DAYS_COUNT = 12;
constexpr std::size_t MAX_LESSONS_PER_DAY = 7;
constexpr std::size_t DAYS_IN_SCHEDULE_WEEK = 6;
constexpr std::size_t DAYS_IN_SCHEDULE = DAYS_IN_SCHEDULE_WEEK * 2;
constexpr std::size_t MAX_LESSONS_COUNT = MAX_LESSONS_PER_DAY * DAYS_IN_SCHEDULE_WEEK * 2;

enum class WeekDay : std::uint8_t
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
};

[[nodiscard]] std::size_t LessonToScheduleDay(std::size_t lesson);
[[nodiscard]] WeekDay ScheduleDayNumberToWeekDay(std::size_t dayNum);
[[nodiscard]] std::vector<std::size_t> MakeIndexesRange(std::size_t n);
[[nodiscard]] constexpr bool IsLateScheduleLessonInSaturday(std::size_t l)
{
    static_assert(MAX_LESSONS_COUNT == 84, "re-fill lateSaturdayLessonsTable");
    assert(l < MAX_LESSONS_COUNT);

    constexpr bool lateSaturdayLessonsTable[MAX_LESSONS_COUNT] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        false,
        false,
        false,

        false,
        false,
        false,
        false,
        true,
        true,
        true,
    };

    return lateSaturdayLessonsTable[l];
}

struct ClassroomAddress
{
    ClassroomAddress() = default;
    explicit ClassroomAddress(std::size_t building, std::size_t classroom)
        : Building(building)
        , Classroom(classroom)
    {}

    static ClassroomAddress NoClassroom();

    friend bool operator==(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return lhs.Building == rhs.Building && lhs.Classroom == rhs.Classroom;
    }
    friend bool operator!=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs == rhs); }

    friend bool operator<(const ClassroomAddress& lhs, const ClassroomAddress& rhs)
    {
        return (lhs.Building < rhs.Building) || (lhs.Building == rhs.Building && lhs.Classroom < rhs.Classroom);
    }
    friend bool operator>(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return rhs < lhs; }
    friend bool operator<=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(rhs < lhs); }
    friend bool operator>=(const ClassroomAddress& lhs, const ClassroomAddress& rhs) { return !(lhs < rhs); }

    std::size_t Building;
    std::size_t Classroom;
};

std::vector<ClassroomAddress> GenerateClassrooms(std::size_t n);


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
 */
class WeekDays
{
    static constexpr std::uint8_t FULL_WEEK = 0b00111111;
public:
    using iterator = WeekDaysIterator;

    WeekDays();
    WeekDays(std::initializer_list<WeekDay> lst);
    static WeekDays fullWeek();
    static WeekDays emptyWeek();

    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;

    void insert(WeekDay d);
    void erase(WeekDay d);
    [[nodiscard]] bool contains(WeekDay d) const;

    friend bool operator==(WeekDays lhs, WeekDays rhs) { return lhs.days_ == rhs.days_; }
    friend bool operator!=(WeekDays lhs, WeekDays rhs) { return !(lhs == rhs); }

private:
    explicit WeekDays(std::uint8_t days);

private:
    std::uint8_t days_;
};

struct LessonAddress
{
    LessonAddress() = default;
    explicit LessonAddress(std::size_t Group, std::size_t Lesson)
            : Group(Group)
            , Lesson(Lesson)
    { }

    [[nodiscard]] friend bool operator<(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return (lhs.Group < rhs.Group) || (lhs.Group == rhs.Group && lhs.Lesson < rhs.Lesson);
    }

    [[nodiscard]] friend bool operator==(const LessonAddress& lhs, const LessonAddress& rhs)
    {
        return lhs.Group == rhs.Group && lhs.Lesson == rhs.Lesson;
    }

    [[nodiscard]] friend bool operator!=(const LessonAddress& lhs, const LessonAddress& rhs) { return !(lhs == rhs); }

    std::size_t Group = 0;
    std::size_t Lesson = 0;
};

struct LessonsMatrixItemAddress
{
    explicit LessonsMatrixItemAddress(std::size_t group,
                                      std::size_t professor,
                                      std::size_t lesson,
                                      std::size_t classroom,
                                      std::size_t subject);

    [[nodiscard]] friend bool operator==(const LessonsMatrixItemAddress& lhs, const LessonsMatrixItemAddress& rhs)
    {
        return lhs.Group == rhs.Group &&
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
        return
               (lhs.Group < rhs.Group) ||
               (lhs.Group == rhs.Group && lhs.Professor < rhs.Professor) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson < rhs.Lesson) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom < rhs.Classroom) ||
               (lhs.Group == rhs.Group && lhs.Professor == rhs.Professor && lhs.Lesson == rhs.Lesson && lhs.Classroom == rhs.Classroom && lhs.Subject < rhs.Subject);
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

    friend bool operator==(const SortedSet& lhs, const SortedSet& rhs) { return lhs.elems_ == rhs.elems_; }
    friend bool operator!=(const SortedSet& lhs, const SortedSet& rhs) { return !(lhs == rhs); }

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

struct FirstEqual
{
    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs.first == rhs.first;
    }

    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const std::pair<T1, T2>& lhs, const T1& rhs)
    {
        return lhs.first == rhs;
    }

    template<typename T1, typename T2>
    [[nodiscard]] bool operator()(const T1& lhs, const std::pair<T1, T2>& rhs)
    {
        return lhs == rhs.first;
    }
};

template<typename K, typename T, typename A = std::allocator<std::pair<K, T>>>
class SortedMap
{
public:
    SortedMap() = default;
    explicit SortedMap(const A& a) : elems_(a) { }
    SortedMap(std::initializer_list<std::pair<K, T>> initLst) : SortedMap(initLst.begin(), initLst.end()) { }

    template<class Container, typename = typename std::enable_if_t<!std::is_same_v<std::decay_t<Container>, SortedMap>>>
    explicit SortedMap(const Container& cont) : SortedMap(std::begin(cont), std::end(cont)) {}

    template<class Iter>
    explicit SortedMap(Iter first, Iter last)
        : elems_(first, last)
    {
        std::sort(elems_.begin(), elems_.end(), FirstLess());
        elems_.erase(std::unique(elems_.begin(), elems_.end(), FirstEqual()), elems_.end());
    }

    [[nodiscard]] const std::vector<std::pair<K, T>, A>& elems() const { return elems_; }

    [[nodiscard]] T& operator[](const K& key)
    {
        auto it = std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
        if(it == elems_.end() || FirstLess()(key, *it))
            it = elems_.emplace(it, key, T{});

        return it->second;
    }

    auto lower_bound(const K& key)
    {
        return std::lower_bound(elems_.begin(), elems_.end(), key, FirstLess());
    }

    auto emplace_hint(
        typename std::vector<std::pair<K, T>, A>::iterator hint,
        const K& key,
        const T& value)
    {
        FirstLess comp;
        if((hint == begin() || (hint != end() && comp(*std::prev(hint), key))) && (hint == end() || !comp(*hint, key)))
            hint = lower_bound(key);

        return elems_.emplace(hint, key, value);
    }

    [[nodiscard]] auto begin() const { return elems_.begin(); }
    [[nodiscard]] auto end() const { return elems_.end(); }

private:
    std::vector<std::pair<K, T>, A> elems_;
};

template<class SortedRange1, class SortedRange2>
[[nodiscard]] bool set_intersects(const SortedRange1& r1, const SortedRange2& r2)
{
    assert(std::is_sorted(std::begin(r1), std::end(r1)));
    assert(std::is_sorted(std::begin(r2), std::end(r2)));

    if(std::empty(r1) || std::empty(r2))
        return true;

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
    explicit LinearAllocatorBufferSpan(std::uint8_t* ptr, std::size_t total)
        : pBegin(ptr)
        , pEnd(ptr)
        , pCapacityEnd(ptr + total)
        , peak(0)
    {
        assert(ptr != nullptr);
        assert(total > 0);
    }

    std::uint8_t* pBegin;
    std::uint8_t* pEnd;
    std::uint8_t* pCapacityEnd;
    std::size_t peak;
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
        const auto currentAddress = reinterpret_cast<std::size_t>(buffer_->pEnd);
        const std::size_t padding = CalculatePadding(currentAddress, __alignof(T));
        const std::size_t countBytes = count * sizeof(T);
        const std::size_t sumAlloc = padding + countBytes;
        buffer_->peak += sumAlloc;
        if (buffer_->pEnd + sumAlloc > buffer_->pCapacityEnd)
        {
#ifdef _DEBUG
            externalAllocationsCounter_++;
#endif
            return overflowAllocator_.allocate(count);
        }

        buffer_->pEnd += sumAlloc;
        return reinterpret_cast<T*>(currentAddress + padding);
    }

    void deallocate(T* p, std::size_t count)
    {
        auto ptr = reinterpret_cast<std::uint8_t*>(p);
        if(ptr >= buffer_->pBegin && ptr < buffer_->pCapacityEnd)
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
