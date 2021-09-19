#include "ScheduleCommon.h"
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include "ScheduleUtils.h"

#include <array>
#include <catch2/catch.hpp>


TEST_CASE("Search by subject id performs correctly", "[schedule_data]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData sut{{SubjectRequest{0, 1, 1, {0}, {}, {}},
                            SubjectRequest{1, 2, 1, {1}, {}, {}},
                            SubjectRequest{2, 3, 2, {2}, {}, {}},
                            SubjectRequest{3, 4, 3, {3}, {}, {}},
                            SubjectRequest{4, 5, 4, {4}, {}, {}}}};

    SECTION("Searching subject request by id")
    {
        for(auto&& request : sut.SubjectRequests())
            REQUIRE(sut.SubjectRequestAtID(request.ID()) == request);

        REQUIRE_THROWS(sut.SubjectRequestAtID(5));
        REQUIRE_THROWS(sut.SubjectRequestAtID(101));
    }
    SECTION("Searching index in requests list by subject id works")
    {
        for(std::size_t i = 0; i < sut.SubjectRequests().size(); ++i)
        {
            auto&& request = sut.SubjectRequests().at(i);
            REQUIRE(sut.IndexOfSubjectRequestWithID(request.ID()) == i);
        }

        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(5));
        REQUIRE_THROWS(sut.IndexOfSubjectRequestWithID(100));
    }
}

TEST_CASE("Sorting lessons by order in day")
{
    SECTION("Empty lessons")
    {
        REQUIRE(LessonsSortedByOrderInDay({}) == std::vector<std::size_t>{});
    }

    SECTION("One lesson")
    {
        REQUIRE(LessonsSortedByOrderInDay({1}) == std::vector<std::size_t>{1});
    }

    SECTION("One day lessons")
    {
        REQUIRE(LessonsSortedByOrderInDay({0, 1, 2, 3, 4, 5, 6})
                == std::vector<std::size_t>{0, 1, 2, 3, 4, 5, 6});
    }

    SECTION("Sorting lessons")
    {
        REQUIRE(LessonsSortedByOrderInDay({0, 1, 3, 5, 6, 7, 8, 15, 17, 21})
                == std::vector<std::size_t>{0, 7, 21, 1, 8, 15, 3, 17, 5, 6});
    }
}
