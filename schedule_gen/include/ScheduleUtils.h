#pragma once
#include <algorithm>
#include <cassert>
#include <vector>


template<class InputIt1, class InputIt2>
bool set_intersects(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
{
    assert(std::is_sorted(first1, last1));
    assert(std::is_sorted(first2, last2));

    while(first1 != last1 && first2 != last2)
    {
        if(*first1 < *first2)
        {
            ++first1;
        }
        else
        {
            if(!(*first2 < *first1))
                return true;

            ++first2;
        }
    }
    return false;
}

template<class SortedRange1, class SortedRange2>
bool set_intersects(const SortedRange1& r1, const SortedRange2& r2)
{
    return set_intersects(std::begin(r1), std::end(r1), std::begin(r2), std::end(r2));
}

template<class Iter, class Value> bool contains(Iter first, Iter last, const Value& value)
{
    return std::find(first, last, value) != last;
}

template<class Container, class Value> bool contains(const Container& container, const Value& value)
{
    return contains(std::begin(container), std::end(container), value);
}


class BitVector
{
    static constexpr std::uint64_t MASK_TEMPLATE = 1;
public:
    BitVector() = default;
    explicit BitVector(std::size_t n)
        : chunks_(n / 64 + 1)
    {}

    bool get_bit(std::size_t index) const
    {
        const std::size_t chunkIndex = index / 64;
        const std::uint64_t bitIndex = index % 64;
        return chunks_.at(chunkIndex) & (MASK_TEMPLATE << bitIndex);
    }

    void set_bit(std::size_t index, bool value)
    {
        const std::size_t chunkIndex = index / 64;
        const std::uint64_t bitIndex = index % 64;
        if(value)
            chunks_.at(chunkIndex) |= (MASK_TEMPLATE << bitIndex);
        else
            chunks_.at(chunkIndex) &= ~(MASK_TEMPLATE << bitIndex);
    }

private:
    std::vector<std::uint64_t> chunks_;
};


class BitIntersectionsMatrix
{
public:
    BitIntersectionsMatrix() = default;
    explicit BitIntersectionsMatrix(std::size_t n)
        : data_((n * n) / 2 - 2)
    {}

    bool get_bit(std::size_t i, std::size_t j) const
    {
        if(i == j)
            return true;

        return data_.get_bit(to_bit_index(i, j));
    }

    void set_bit(std::size_t i, std::size_t j, bool value)
    {
        if(i == j)
            return;

        data_.set_bit(to_bit_index(i, j), value);
    }

private:
    static constexpr std::size_t to_bit_index(std::size_t i, std::size_t j) noexcept
    {
        return i > j ? i * (i - 1) / 2 + j : to_bit_index(j, i);
    }

private:
    BitVector data_;
};


template<class Iter>
Iter remove_duplicates(Iter first, Iter last)
{
    auto result = first;
    for(; first != last; ++first)
    {
        bool found = false;
        for(auto s = first; s != last; ++s)
        {
            if(first != s && *first == *s)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            *result = std::move(*first);
            ++result;
        }
    }

    return result;
}

template<class Iter, class Pred>
Iter remove_duplicates(Iter first, Iter last, Pred pred)
{
    auto result = first;
    for(; first != last; ++first)
    {
        bool found = false;
        for(auto s = first; s != last; ++s)
        {
            if(first != s && pred(*first, *s))
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            *result = std::move(*first);
            ++result;
        }
    }

    return result;
}

template<typename InputIter1, typename InputIter2>
bool unsorted_set_intersects(InputIter1 first1, InputIter1 last1,
                             InputIter2 first2, InputIter2 last2)
{
    for(auto i = first1; i != last1; ++i)
    {
        for(auto j = first2; j != last2; ++j)
        {
            if(*i == *j)
                return true;
        }
    }

    return false;
}

template<class Range1, class Range2>
bool unsorted_set_intersects(const Range1& r1, const Range2& r2)
{
    return unsorted_set_intersects(std::begin(r1), std::end(r1),
                                   std::begin(r2), std::end(r2));
}
