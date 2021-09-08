#pragma once
#include <cassert>
#include <algorithm>


template<class InputIt1, class InputIt2>
bool set_intersects(InputIt1 first1, InputIt1 last1,
                    InputIt2 first2, InputIt2 last2)
{
    assert(std::is_sorted(first1, last1));
    assert(std::is_sorted(first2, last2));

    while (first1 != last1 && first2 != last2)
    {
        if (*first1 < *first2)
        {
            ++first1;
        }
        else
        {
            if (!(*first2 < *first1))
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

template<class Iter, class Value>
bool contains(Iter first, Iter last, const Value& value)
{
    return std::find(first, last, value) != last;
}

template<class Container, class Value>
bool contains(const Container& container, const Value& value)
{
    return contains(std::begin(container), std::end(container), value);
}
