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

template<class Container, class Value> bool contains(const Container& container, const Value& value)
{
    return std::ranges::find(container, value) != std::end(container);
}

template<class T, class Arg> bool insert_unique_ordered(std::vector<T>& vec, Arg&& value)
{
    auto it = std::lower_bound(std::begin(vec), std::end(vec), value);
    if(it != std::end(vec) && *it == value)
        return false;

    vec.emplace(it, std::forward<Arg>(value));
    return true;
}

template<class T, class Arg, class Comp>
bool insert_unique_ordered(std::vector<T>& vec, Arg&& value, Comp&& comp)
{
    auto it = std::lower_bound(std::begin(vec), std::end(vec), value, comp);
    if(it != std::end(vec) && !comp(value, *it))
        return false;

    vec.emplace(it, std::forward<Arg>(value));
    return true;
}

class BitVector
{
    static constexpr std::uint64_t MASK_TEMPLATE = 1;

public:
    BitVector() = default;
    explicit BitVector(std::size_t n)
        : chunks_(n / 64 + 1)
    {
    }

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
    {
    }

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
