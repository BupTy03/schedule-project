#include <iostream>
#include <random>


struct IndexesGenerator
{
    using result_type = std::size_t;

    result_type operator()() { return index_++; }

    static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
    static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

private:
    result_type index_ = 0;
};


int main()
{
    IndexesGenerator gen;
    std::uniform_int_distribution<std::size_t> distr(0, 5);
    for(std::size_t i = 0; i < 100; ++i)
        std::cout << distr(gen) << ' ';

    std::cout << std::endl;
    return 0;
}
