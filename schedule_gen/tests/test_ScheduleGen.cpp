#include "ScheduleUtils.h"
#include "ScheduleCommon.h"
#include "ScheduleData.h"
#include "ScheduleResult.h"
#include <catch2/catch.hpp>
#include <array>


TEST_CASE("SubjectRequest constructs correctly", "[subject_request]")
{
    SECTION("Sorting and removing duplicates from groups list while constructing")
    {
        // set_intersects algorithm REQUIRES sorted groups

        // [id, professor, complexity, groups, lessons, classrooms]
        const SubjectRequest sut{0, 1, 1, {3, 1, 2, 2, 5, 0, 10, 5, 1, 3, 3}, {}, {}};
        REQUIRE(sut.Groups() == std::vector<std::size_t>{0, 1, 2, 3, 5, 10});
    }
}

TEST_CASE("Search by subject id performs correctly", "[schedule_data]")
{
    // [id, professor, complexity, groups, lessons, classrooms]
    const ScheduleData sut{{
        SubjectRequest{0, 1, 1, {0}, {}, {}},
        SubjectRequest{1, 2, 1, {1}, {}, {}},
        SubjectRequest{2, 3, 2, {2}, {}, {}},
        SubjectRequest{3, 4, 3, {3}, {}, {}},
        SubjectRequest{4, 5, 4, {4}, {}, {}}
    }};

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
