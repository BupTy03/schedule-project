#include <catch2/catch.hpp>


TEST_CASE("SimpleTest", "[tests]")
{
    int a = 0;
    int b = 0;
    REQUIRE(a == b);
}
