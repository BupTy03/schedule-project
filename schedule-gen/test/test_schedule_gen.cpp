#include <catch2/catch.hpp>

#include "ScheduleData.h"


TEST_CASE("Test", "[test]")
{
    ScheduleData data;
    REQUIRE(data.Name() == "ScheduleData");
}


